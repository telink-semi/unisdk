#include "core/include/tlk_power.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_sys.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_soc.h"

#define TLK_POWER_INTERNAL_CAP_VALUE 0x20

extern uint8_t tlk_g_chip_version;

union tlk_areg_power5 power_status = {.raw = 0};

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

_tlk_attribute_ram_code_sec_noinline_ static void tlk_power_set_mode(enum tlk_power_mode power_mode)
{
    bool                       clk_24m_rc_was_used = false;
    enum tlk_sys_reboot_reason reboot_reason       = tlk_sys_get_reboot_reason();

    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION)
    {
        power_status = TLK_ANALOG_READ(TLK_AREG_POWER5);
    }

    if (power_status.bit.pd_dcdc_ldo_sw != power_mode)
    {
        if (TLK_POWER_LDO_LDO_MODE != power_mode)
        {
            /* Set trim code 0110 to get 1.2V DCDC output */
            TLK_ANALOG_MODIFY(TLK_AREG_POWER7, (value.bit.dcdc_trim_soc_out = 0b0110;));

            /*
             * In A1 version, undering DCDC mode, some chips with low vdd_core voltage (around
             * 0.95V, still within the theoretical voltage range for digital operation) will cause a
             * crash when open RF module frequently by write analog 0x7d. The specific reason is not
             * yet clear, but raising the digital ldo voltage can temporarily avoid this
             * problem.(ISSUE:BUT-41) In A2 version, this issue has been fixed(originating from the
             * IR Drop issue).(confirmed at 20250319)
             */
            if (tlk_g_chip_version == TLK_CHIP_VERSION_A1)
            {
                /* Set 000 to close all branches */
                TLK_ANALOG_MODIFY(TLK_AREG_POWER10, (value.bit.dig_ldo_fb_trim_3v = 0b000;));

                /* Trim digital LDO to 1.05V */
                TLK_ANALOG_MODIFY(TLK_AREG_POWER9, (value.bit.dig_ldo_trim = 0b1100;));
            }
            clk_24m_rc_was_used = tlk_clock_24m_rc_acquire();
        }

        power_status.bit.pd_dcdc_ldo_sw = power_mode;
        power_status.bit.tr_dcdc_oc     = 0b110; /* Set 1.4V comparator reference */
        TLK_ANALOG_WRITE(TLK_AREG_POWER5, power_status);

        if (TLK_POWER_LDO_LDO_MODE != power_mode)
        {
            if (!clk_24m_rc_was_used)
            {
                tlk_clock_cclk_delay((unsigned long long) (25 * tlk_sys_clk.cclk)); /* 25us delay */
            }

            tlk_clock_24m_rc_release();
        }
    }
}

_tlk_attribute_ram_code_sec_ static void
tlk_power_set_dig_ldo_voltage(tlk_power_dig_ldo_trim_e dig_ldo_trim)
{
    TLK_ANALOG_MODIFY(TLK_AREG_POWER9, (value.bit.dig_ldo_trim = dig_ldo_trim;));
}

_tlk_attribute_ram_code_sec_ static void tlk_power_set_1p25v(tlk_power_trim_1p25v_e trim_1p25vldo)
{
    TLK_ANALOG_MODIFY(TLK_AREG_POWER4, (value.bit.tr_dcdc_ldo_0p94 = trim_1p25vldo;));
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_configure(enum tlk_power_vbat vbat_v,
                                                               enum tlk_power_mode power_mode)
{
    tlk_power_set_mode(power_mode);

    /*
     * After waking up, it is not safe to power supply both the native LDO and the normal LDO
     * together. Therefore, this code will be processed in advance here to reduce the shared power
     * supply time.(add by jilong.liu, 20240221)
     */
    TLK_ANALOG_MODIFY(TLK_AREG_POWER6, (
        value.bit.mscn_pullup_res_en_b = 0;
        value.bit.pd_nvt_0p94 = 1;
        value.bit.pd_nvt_1p8 = 1;
    ));

    /*
     * For tl321x, the retention ldo should turn on before use cause it need more time to be stable.
     * The follow chip version will not change this situation.
     */
    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN1, (
        value.bit.pd_vbat_switch = !(vbat_v);
        value.bit.pd_spd_ldo = 1;
        value.bit.pd_ret_ldo = 0;
    ));

    if (tlk_g_chip_version == TLK_CHIP_VERSION_A0)
    {
        // For version A0, the theoretical value is low, and the current enumeration value is set
        // after actual testing.
        tlk_power_set_dig_ldo_voltage(TLK_POWER_DIG_LDO_TRIM_1P010V); // trim VDDDEC to 1V
        tlk_power_set_1p25v(TLK_POWER_TRIM_1P25V_TO_1P25V);           // trim VDD1P25 to 1.25V
    }
}

_tlk_attribute_ram_code_sec_noinline_ void
tlk_power_set_dig_modules_state(enum tlk_power_dig_module modules, bool enable)
{
    /* when doing digital module power switch should make sure the 24m rc is working or power switch
     * won't take effect.*/
    tlk_clock_24m_rc_acquire();

    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN2, (
        if (enable) { TLK_BM_CLR(value.raw, modules);
        } else { TLK_BM_SET(value.raw, modules);
        }

        value.bit.pg_clk_en = 1;
    ));

    /* Wait for power stable, for this chip(tl321x), is a fixed value 5us.*/
    tlk_clock_cclk_delay((unsigned long long) (5 * tlk_sys_clk.cclk));
    ;

    tlk_clock_24m_rc_release();
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_init(enum tlk_power_gpio_voltage power_level,
                                                          enum tlk_power_cap          cap)
{
    (void) power_level;

    tlk_clock_24m_xtal_configure_cap((struct tlk_clock_24m_xtal_cap){
        .use_external_cap   = cap == TLK_POWER_EXTERNAL_CAP_XTAL24M,
        .internal_cap_value = TLK_POWER_INTERNAL_CAP_VALUE,
    });
}
