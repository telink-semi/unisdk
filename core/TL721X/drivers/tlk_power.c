#include "core/include/tlk_power.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_sys.h"
#include "tlk_otp_internal.h"
#include "registers/tlk_analog_afe3v_reg.h"
#include "registers/tlk_analog_afe1v_reg.h"

#define TLK_POWER_DIG_MODULES_CLOCK     TLK_BIT(7)
#define TLK_POWER_INTERNAL_CAP_VALUE    0x4c

extern uint8_t tlk_g_chip_version;

typedef enum
{
    TLK_POWER_TRIM_VDDF_TO_1P64V = 0,
    TLK_POWER_TRIM_VDDF_TO_1P66V,
    TLK_POWER_TRIM_VDDF_TO_1P68V,
    TLK_POWER_TRIM_VDDF_TO_1P70V,
    TLK_POWER_TRIM_VDDF_TO_1P72V,
    TLK_POWER_TRIM_VDDF_TO_1P74V,
    TLK_POWER_TRIM_VDDF_TO_1P76V,
    TLK_POWER_TRIM_VDDF_TO_1P78V,
    TLK_POWER_TRIM_VDDF_TO_1P80V,
    TLK_POWER_TRIM_VDDF_TO_1P83V,
    TLK_POWER_TRIM_VDDF_TO_1P86V,
    TLK_POWER_TRIM_VDDF_TO_1P89V,
    TLK_POWER_TRIM_VDDF_TO_1P92V,
    TLK_POWER_TRIM_VDDF_TO_1P95V,
    TLK_POWER_TRIM_VDDF_TO_1P98V,
    TLK_POWER_TRIM_VDDF_TO_2P01V,
} tlk_power_trim_vddf_e;


typedef enum
{
    TLK_POWER_TRIM_VDD0P94_TO_0P820V = 0,
    TLK_POWER_TRIM_VDD0P94_TO_0P835V,
    TLK_POWER_TRIM_VDD0P94_TO_0P850V,
    TLK_POWER_TRIM_VDD0P94_TO_0P865V,
    TLK_POWER_TRIM_VDD0P94_TO_0P880V,
    TLK_POWER_TRIM_VDD0P94_TO_08950V,
    TLK_POWER_TRIM_VDD0P94_TO_0P910V,
    TLK_POWER_TRIM_VDD0P94_TO_0P925V,
    TLK_POWER_TRIM_VDD0P94_TO_0P940V,
    TLK_POWER_TRIM_VDD0P94_TO_0P963V,
    TLK_POWER_TRIM_VDD0P94_TO_0P986V,
    TLK_POWER_TRIM_VDD0P94_TO_1P009V,
    TLK_POWER_TRIM_VDD0P94_TO_1P032V,
    TLK_POWER_TRIM_VDD0P94_TO_1P055V,
    TLK_POWER_TRIM_VDD0P94_TO_1P078V,
    TLK_POWER_TRIM_VDD0P94_TO_1P101V,
} tlk_power_trim_vdd0p94_e;

typedef enum
{
    TLK_POWER_CAL_0P94V_TO_0P95V,
    TLK_POWER_CAL_0P94V_TO_1P05V,
} tlk_power_cal_0p94v_e;


typedef enum
{
    TLK_POWER_TRIM_VDDO1P8_TO_1P664V = 0,
    TLK_POWER_TRIM_VDDO1P8_TO_1P703V,
    TLK_POWER_TRIM_VDDO1P8_TO_1P744V,
    TLK_POWER_TRIM_VDDO1P8_TO_1P787V,
    TLK_POWER_TRIM_VDDO1P8_TO_1P832V,
    TLK_POWER_TRIM_VDDO1P8_TO_1P879V,
    TLK_POWER_TRIM_VDDO1P8_TO_1P929V,
    TLK_POWER_TRIM_VDDO1P8_TO_1P981V,
} tlk_power_trim_vddo1p8_e;

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

    tlk_clock_cclk_delay((unsigned long long)(1 * tlk_sys_clk.cclk)); /* 1us delay */

    tlk_clock_24m_rc_release();
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_set_mode(enum tlk_power_mode power_mode)
{
    uint8_t dcdc_status = 0;
    bool clk_24m_rc_was_used = false;
    enum tlk_sys_reboot_reason reboot_reason = tlk_sys_get_reboot_reason();

    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION) {
        /* 
         * In A1 version, this bit need to set 0 before turning on DCDC mode. Otherwise, the DCDC mode cannot be used.
         * In A2 version, the default value of this bit is 1, but added a reverser internally so not need to set. */
        if (tlk_g_chip_version == TLK_CHIP_VERSION_A1) {
            /*
             *                      poweron_dft:    0x83 -> 0x82.
             *      bit                     note
             * ---------------------------------------------------------------------------
             * <0>:dcdc_cal_twohigh_en,     default:1,->0 disable calibrate the logic bug when two EA output is high
             * This will reduce power consumption and has little impact to operate early or late.
             */
            uint8_t calibration_val = tlk_analog_read_reg8(areg_aon_0x02);
            TLK_BM_CLR(calibration_val, FLD_DCDC_CAL_TWOHIGH_EN);
            tlk_analog_write_reg8(areg_aon_0x02, calibration_val);
        }

        dcdc_status = tlk_analog_read_reg8(areg_aon_0x0a);
    }

    if ((dcdc_status & 0x03) != power_mode) {
        /* The power-on process of a DCDC requires a 24M rc clock. */
        if (TLK_POWER_LDO_LDO_MODE != power_mode) {
            clk_24m_rc_was_used = tlk_clock_24m_rc_acquire();

            /* DCDC voltage increase one level from 0.94/1.8 to 0.963/1.83. */
            tlk_analog_write_reg8(areg_aon_0x02, tlk_analog_read_reg8(areg_aon_0x02) | FLD_DCDC_VCOMP_VOS_PN); /* calibrate the comparator offset voltage enable */
            tlk_analog_write_reg8(areg_aon_0x01, 0x84);                                                    /* adjust offset to optimize efficiency */
        }

        dcdc_status = (dcdc_status & 0xfc) | power_mode;
        tlk_analog_write_reg8(areg_aon_0x0a, dcdc_status);

        if (TLK_POWER_LDO_LDO_MODE != power_mode) {
            if (!clk_24m_rc_was_used) {
                tlk_clock_cclk_delay((unsigned long long)(25 * tlk_sys_clk.cclk)); /* 25us delay */
            }
            tlk_clock_24m_rc_release();
        }
    }
}

static _tlk_always_inline void tlk_power_update_internal_cap(unsigned char value)
{
    uint8_t capacitor_control = tlk_analog_read_reg8(0x8a);
    capacitor_control = TLK_BM_IS_SET(capacitor_control, TLK_BIT_RNG(6,7));
    value = TLK_BM_IS_SET(value, TLK_BIT_RNG(0,5));
    TLK_BM_SET(capacitor_control, value);
    tlk_analog_write_reg8(0x8a, capacitor_control);
}

static _tlk_always_inline void tlk_power_turn_off_internal_cap(void)
{
    tlk_analog_write_reg8(0x8a, tlk_analog_read_reg8(0x8a) | TLK_BIT(7));
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_trim_vdd0p94v(tlk_power_cal_0p94v_e value, tlk_power_cal_vdd0p94_t tlk_power_vdd0p94_cal_info)
{
    /* 0-3 bits - 1.8V LDO output
    *  4-7 bits - 0.94V LDO output */
    uint8_t ldo_outputs_cal_values = 0;

    /* 0-3 bits - 1.8V DCDC output to flash
    *  4-7 bits - 0.94V DCDC output to flash */
    uint8_t dcdc_outputs_cal_values = 0;

    if (value == TLK_POWER_CAL_0P94V_TO_0P95V) {
        ldo_outputs_cal_values = (tlk_power_vdd0p94_cal_info.ldo_0p95v << 4)   | TLK_POWER_TRIM_VDDF_TO_1P80V;
        dcdc_outputs_cal_values = (tlk_power_vdd0p94_cal_info.dcdc_0p95v << 4) | TLK_POWER_TRIM_VDDF_TO_1P80V;

    } else if (value == TLK_POWER_CAL_0P94V_TO_1P05V) {
        ldo_outputs_cal_values = (tlk_power_vdd0p94_cal_info.ldo_1p05v << 4)   | TLK_POWER_TRIM_VDDF_TO_1P80V;
        dcdc_outputs_cal_values = (tlk_power_vdd0p94_cal_info.dcdc_1p05v << 4) | TLK_POWER_TRIM_VDDF_TO_1P80V;
    }

    tlk_analog_write_reg8(0x09, ldo_outputs_cal_values);
    tlk_analog_write_reg8(0x0c, dcdc_outputs_cal_values);
}

static _tlk_always_inline void tlk_power_trim_ddo1p8v(tlk_power_trim_vddo1p8_e trim_vddo1p8)
{
    /*
    * <2:0>:areg_aon_0x08,    calibrate LDO_1p8_HP/LDO_1p8_LP LDO output.
    */
    uint8_t ldo_1p8_cal_value = TLK_BM_IS_SET(tlk_analog_read_reg8(areg_aon_0x08), TLK_BIT_RNG(3,7));
    TLK_BM_SET(ldo_1p8_cal_value, trim_vddo1p8);
    tlk_analog_write_reg8(areg_aon_0x08, ldo_1p8_cal_value);
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_init( enum tlk_power_gpio_voltage power_level, enum tlk_power_cap cap )
{
    (void) power_level;

    if (cap != TLK_POWER_INTERNAL_CAP_XTAL24M)
    {
        tlk_power_turn_off_internal_cap();
    } else {
        /* Updating the crystal internal capacitance value to adjust RF frequency offset.*/
        tlk_power_update_internal_cap(TLK_POWER_INTERNAL_CAP_VALUE);
    }
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_configure( enum tlk_power_vbat vbat_v, enum tlk_power_mode power_mode )
{
    enum tlk_sys_reboot_reason reboot_reason = tlk_sys_get_reboot_reason();
    tlk_power_cal_vdd0p94_t tlk_power_vdd0p94_cal_info = {TLK_POWER_TRIM_VDD0P94_TO_0P963V, TLK_POWER_TRIM_VDD0P94_TO_0P963V, TLK_POWER_TRIM_VDD0P94_TO_1P078V, TLK_POWER_TRIM_VDD0P94_TO_1P078V};
    uint8_t tlk_g_pm_cal_vddo1p8_info    = TLK_POWER_TRIM_VDDO1P8_TO_1P832V;

    /*
     * If back from deep ret sleep, there is no need to read or set calibration value cause they are all maintained.
     * If back from deep sleep, the calibration value need to read but not need to set cause they are all maintained.
     * For other case(power on or reboot), it's necessary to both read and set calibration value for both LDO and DCDC mode.
     */
    if(reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION) {
        /* Read calibration values from OTP */
        tlk_otp_get_vdd0p94_vddo1p8_calib_value(&tlk_power_vdd0p94_cal_info, &tlk_g_pm_cal_vddo1p8_info);
    }

    tlk_power_set_mode(power_mode);

    uint8_t is_boot_or_reboot = TLK_BM_IS_SET(tlk_analog_read_reg8(TLK_PM_ANA_REG_WD_CLR_BUF0), TLK_POWERON_FLAG);
    if (is_boot_or_reboot) {
        tlk_power_trim_vdd0p94v(TLK_POWER_CAL_0P94V_TO_0P95V, tlk_power_vdd0p94_cal_info);
        tlk_power_trim_ddo1p8v(tlk_g_pm_cal_vddo1p8_info);
    }
    /*
     * After waking up, it is not safe to power supply both the native LDO and the normal LDO together. */
    uint8_t dcdc_ldo_native_enable = tlk_analog_read_reg8(areg_aon_0x0b);
    TLK_BM_CLR(dcdc_ldo_native_enable, FLD_MSCN_PULLUP_RES_ENB);
    TLK_BM_SET(dcdc_ldo_native_enable, (FLD_PD_NVT_0P94 | FLD_PD_NVT_1P8));
    tlk_analog_write_reg8(areg_aon_0x0b, dcdc_ldo_native_enable);

    uint8_t ldo_outputs = tlk_analog_read_reg8(areg_aon_0x06);
    TLK_BM_SET(ldo_outputs, (FLD_PD_SPD_LDO | FLD_PD_DIG_RET_LDO | FLD_PD_VBAT_SW));
    TLK_BM_CLR(ldo_outputs, vbat_v);
    tlk_analog_write_reg8(areg_aon_0x06, ldo_outputs);

    if (tlk_g_chip_version == TLK_CHIP_VERSION_A1) {
         /*
         * This will reduce power consumption and has little impact to operate early or late.
         */
        tlk_analog_write_reg8(areg_0x8f, tlk_analog_read_reg8(areg_0x8f) | FLD_AUDIO_VMID_PD); // POWER

        if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION) {
            /*
            * A1 version add 0x13<3> as bb power switch to fix the ISSUE(TER-32).
            * vdd_bb was powered by dig_ldo/spd_ldo instead dcore_ldo, so here need to power it up first.
            */
            tlk_analog_write_reg8(areg_aon_0x13, tlk_analog_read_reg8(areg_aon_0x13) & ~TLK_BIT(3)); //POWER
        }
    }
}
