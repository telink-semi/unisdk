#include "core/include/tlk_power.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_sys.h"
#include "registers/tlk_analog_afe3v_reg.h"
#include "registers/tlk_analog_afe1v_reg.h"

#define TLK_POWER_DIG_MODULES_CLOCK TLK_BIT(7)
extern uint8_t tlk_g_chip_version;

typedef enum
{
    TLK_POWER_TRIM_1P25V_TO_1P050V = 0x0,
    TLK_POWER_TRIM_1P25V_TO_1P065V,
    TLK_POWER_TRIM_1P25V_TO_1P080V,
    TLK_POWER_TRIM_1P25V_TO_1P095V,
    TLK_POWER_TRIM_1P25V_TO_1P110V,
    TLK_POWER_TRIM_1P25V_TO_1P125V,
    TLK_POWER_TRIM_1P25V_TO_1P140V,
    TLK_POWER_TRIM_1P25V_TO_1P155V,
    TLK_POWER_TRIM_1P25V_TO_1P170V,
    TLK_POWER_TRIM_1P25V_TO_1P19V,
    TLK_POWER_TRIM_1P25V_TO_1P21V,
    TLK_POWER_TRIM_1P25V_TO_1P23V,
    TLK_POWER_TRIM_1P25V_TO_1P25V,
    TLK_POWER_TRIM_1P25V_TO_1P27V,
    TLK_POWER_TRIM_1P25V_TO_1P29V,
    TLK_POWER_TRIM_1P25V_TO_1P31V,
} tlk_power_trim_1p25v_e;

typedef enum
{
    TLK_POWER_DIG_LDO_TRIM_0P735V = 0,
    TLK_POWER_DIG_LDO_TRIM_0P760V,
    TLK_POWER_DIG_LDO_TRIM_0P785V,
    TLK_POWER_DIG_LDO_TRIM_0P810V,
    TLK_POWER_DIG_LDO_TRIM_0P835V,
    TLK_POWER_DIG_LDO_TRIM_0P860V,
    TLK_POWER_DIG_LDO_TRIM_0P885V,
    TLK_POWER_DIG_LDO_TRIM_0P910V,
    TLK_POWER_DIG_LDO_TRIM_0P935V,
    TLK_POWER_DIG_LDO_TRIM_0P960V,
    TLK_POWER_DIG_LDO_TRIM_0P985V,
    TLK_POWER_DIG_LDO_TRIM_1P010V,
    TLK_POWER_DIG_LDO_TRIM_1P035V,
    TLK_POWER_DIG_LDO_TRIM_1P060V,
    TLK_POWER_DIG_LDO_TRIM_1P085V,
    TLK_POWER_DIG_LDO_TRIM_1P100V,
} tlk_power_dig_ldo_trim_e;


static _tlk_attribute_ram_code_sec_noinline_ void tlk_power_set_mode(enum tlk_power_mode power_mode)
{
    uint8_t power_status = 0;
    bool clk_24m_rc_was_used = false;
    enum tlk_sys_reboot_reason reboot_reason = tlk_sys_get_reboot_reason();

    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION) {
        power_status = tlk_analog_read_reg8(areg_aon_0x0a);
    }

    if ((TLK_BM_IS_SET(power_status, 0x03)) != power_mode) {
        if (TLK_POWER_LDO_LDO_MODE != power_mode) {
            uint8_t dcdc_cal_value = tlk_analog_read_reg8(areg_aon_0x0c);
            TLK_BM_CLR(dcdc_cal_value, ~(TLK_BIT_RNG(0,3)));
            TLK_BM_SET(dcdc_cal_value, TLK_BIT_RNG(5,6)); /* Set trim code 0110 to get 1.2V DCDC output*/
            tlk_analog_write_reg8(areg_aon_0x0c, dcdc_cal_value);
            /*
             * In A1 version, undering DCDC mode, some chips with low vdd_core voltage (around 0.95V, still within the theoretical voltage range for digital operation) 
             * will cause a crash when open RF module frequently by write analog 0x7d.
             * The specific reason is not yet clear, but raising the digital ldo voltage can temporarily avoid this problem.(ISSUE:BUT-41)
             * In A2 version, this issue has been fixed(originating from the IR Drop issue).(confirmed at 20250319)
             */
            if (tlk_g_chip_version == TLK_CHIP_VERSION_A1) {
                uint8_t digital_ldo_current_val = tlk_analog_read_reg8(0x10);
                TLK_BM_CLR(digital_ldo_current_val, TLK_BIT_RNG(4,6)); /* Set 000 to close all branches */
                tlk_analog_write_reg8(0x10, digital_ldo_current_val );

                uint8_t digital_ldo_trim_val = tlk_analog_read_reg8(areg_aon_0x0f);
                TLK_BM_CLR(digital_ldo_trim_val, ~TLK_BIT_RNG(0,3));
                TLK_BM_SET(digital_ldo_trim_val, TLK_BIT_RNG(2,3)); /* Trim digital LDO to 1.05V */
                tlk_analog_write_reg8(areg_aon_0x0f, digital_ldo_trim_val); // trim dig ldo to 1.05v
            }
            clk_24m_rc_was_used = tlk_clock_24m_rc_acquire();
        }

        TLK_BM_CLR(power_status, ~(TLK_BIT(2) | TLK_BIT(3) | TLK_BIT(7))); /* Reset calibration parameters */
        TLK_BM_SET(power_status, (power_mode | TLK_BIT_RNG(5,6))); /* Set 110 code for 1.4V comparator reference */
        tlk_analog_write_reg8(areg_aon_0x0a, power_status);

        if (TLK_POWER_LDO_LDO_MODE != power_mode) {
            if (!clk_24m_rc_was_used) {
                tlk_clock_cclk_delay((unsigned long long)(25 * tlk_sys_clk.cclk)); /* 25us delay */
            }

            tlk_clock_24m_rc_release();
        }
    }
}

static _tlk_always_inline void tlk_power_turn_off_internal_cap(void)
{
    tlk_analog_write_reg8(0x8a, tlk_analog_read_reg8(0x8a) | TLK_BIT(7));
}

static _tlk_always_inline void tlk_power_set_dig_ldo_voltage(tlk_power_dig_ldo_trim_e dig_ldo_trim)
{
    uint8_t ldo_trim_value = tlk_analog_read_reg8(areg_aon_0x0f) & TLK_BIT_RNG(0, 3);
    TLK_BM_SET(ldo_trim_value, dig_ldo_trim << 4);
    tlk_analog_write_reg8(areg_aon_0x0f, ldo_trim_value);
}


static _tlk_always_inline void tlk_power_set_1p25v(tlk_power_trim_1p25v_e trim_1p25vldo)
{
    uint8_t ldo_trim_value = tlk_analog_read_reg8(areg_aon_0x09) & (TLK_BIT_RNG(0, 3) << 4);
    TLK_BM_SET(ldo_trim_value, trim_1p25vldo);
    tlk_analog_write_reg8(areg_aon_0x09, ldo_trim_value);
}


_tlk_attribute_ram_code_sec_noinline_ void tlk_power_configure( enum tlk_power_vbat vbat_v, enum tlk_power_mode power_mode )
{
    tlk_power_set_mode(power_mode);

    /*
     * After waking up, it is not safe to power supply both the native LDO and the normal LDO together.
     * Therefore, this code will be processed in advance here to reduce the shared power supply time.(add by jilong.liu, 20240221)
     */
    uint8_t dcdc_ldo_native_enable = tlk_analog_read_reg8(areg_aon_0x0b);
    TLK_BM_CLR(dcdc_ldo_native_enable, FLD_MSCN_PULLUP_RES_ENB);
    TLK_BM_SET(dcdc_ldo_native_enable, (FLD_PD_NVT_0P94 | FLD_PD_NVT_1P8));
    tlk_analog_write_reg8(areg_aon_0x0b, dcdc_ldo_native_enable);

    /*
     * For tl321x, the retention ldo should turn on before use cause it need more time to be stable.
     * The follow chip version will not change this situation.
     */
    uint8_t ldo_outputs = tlk_analog_read_reg8(areg_aon_0x06);
    TLK_BM_SET(ldo_outputs, (FLD_PD_SPD_LDO | FLD_PD_VBAT_SW));
    TLK_BM_CLR(ldo_outputs, (vbat_v | FLD_PD_DIG_RET_LDO));
    tlk_analog_write_reg8(areg_aon_0x06, ldo_outputs);

    if (tlk_g_chip_version == TLK_CHIP_VERSION_A0) {
        //For version A0, the theoretical value is low, and the current enumeration value is set after actual testing.
        tlk_power_set_dig_ldo_voltage(TLK_POWER_DIG_LDO_TRIM_1P010V); //trim VDDDEC to 1V
        tlk_power_set_1p25v(TLK_POWER_TRIM_1P25V_TO_1P25V);           //trim VDD1P25 to 1.25V
    }
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_dig_modules_set_state(enum tlk_power_dig_module modules, bool enable)
{
    /* when doing digital module power switch should make sure the 24m rc is working or power switch won't take effect.*/
    tlk_clock_24m_rc_acquire();

    uint8_t modules_power_status = tlk_analog_read_reg8(areg_aon_0x7d);
    modules_power_status |=  TLK_POWER_DIG_MODULES_CLOCK;
    if (enable) {
        TLK_BM_CLR(modules_power_status, modules);
    } else {
        TLK_BM_SET(modules_power_status, modules);
        
    }
    tlk_analog_write_reg8(areg_aon_0x7d, modules_power_status);

    /* Wait for power stable, for this chip(tl321x), is a fixed value 5us.*/
    tlk_clock_cclk_delay((unsigned long long)(5 * tlk_sys_clk.cclk));;

    tlk_clock_24m_rc_release();
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_init( enum tlk_power_gpio_voltage power_level, enum tlk_power_cap cap )
{
    (void) power_level;

    if (cap != TLK_POWER_INTERNAL_CAP_XTAL24M) {
        tlk_power_turn_off_internal_cap();
    }
}

