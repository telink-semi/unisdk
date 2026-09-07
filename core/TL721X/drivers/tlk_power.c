#include "core/include/tlk_power.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_sys.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_soc.h"

#define TLK_POWER_INTERNAL_CAP_VALUE 0x4c

typedef struct
{
    unsigned char dcdc_0p95v;
    unsigned char ldo_0p95v;
    unsigned char dcdc_1p05v;
    unsigned char ldo_1p05v;
} tlk_power_cal_vdd0p94_t;

extern uint8_t tlk_g_chip_version;
extern unsigned char
tlk_otp_get_vdd0p94_vddo1p8_calib_value(tlk_power_cal_vdd0p94_t* tlk_power_vdd0p94_cal_info,
                                        uint8_t*                 tlk_g_pm_cal_vddo1p8_info);

union tlk_areg_power5 tlk_power_status = {.raw = 0};

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

    tlk_clock_cclk_delay((unsigned long long) (1 * tlk_sys_clk.cclk)); /* 1us delay */

    tlk_clock_24m_rc_release();
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_set_mode(enum tlk_power_mode power_mode)
{
    bool                       clk_24m_rc_was_used = false;
    enum tlk_sys_reboot_reason reboot_reason       = tlk_sys_get_reboot_reason();

    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION)
    {
        // In A1 version, this bit need to set 0 before turning on DCDC mode. Otherwise, the DCDC
        // mode cannot be used. In A2 version, the default value of this bit is 1, but added a
        // reverser internally so not need to set.
        if (tlk_g_chip_version == TLK_CHIP_VERSION_A1)
        {
            // Disable calibrate the logic bug when two EA output is high
            // This will reduce power consumption and has little impact to operate early or late.
            TLK_ANALOG_MODIFY(TLK_AREG_POWER1, (value.bit.dcdc_cal_two_high_en = 0;));
        }

        tlk_power_status = TLK_ANALOG_READ(TLK_AREG_POWER5);
    }

    if (tlk_power_status.bit.pd_dcdc_ldo_sw != power_mode)
    {
        /* The power-on process of a DCDC requires a 24M rc clock. */
        if (TLK_POWER_LDO_LDO_MODE != power_mode)
        {
            clk_24m_rc_was_used = tlk_clock_24m_rc_acquire();

            /* DCDC voltage increase one level from 0.94/1.8 to 0.963/1.83. */
            TLK_ANALOG_MODIFY(TLK_AREG_POWER1, (value.bit.dcdc_vcomp_cal_en = 1;)); /* calibrate the comparator offset voltage enable */

            union tlk_areg_power0 power0 = {
                .bit = {.dcdc_btr = 0b100, .dcdc_vcomp_in_cal_offset = 0b10000}};
            TLK_ANALOG_WRITE(TLK_AREG_POWER0, power0); /* adjust offset to optimize efficiency */
        }

        tlk_power_status.bit.pd_dcdc_ldo_sw = power_mode;
        TLK_ANALOG_WRITE(TLK_AREG_POWER5, tlk_power_status);

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

_tlk_attribute_ram_code_sec_noinline_ void
tlk_power_trim_vdd0p94v(tlk_power_cal_0p94v_e   value,
                        tlk_power_cal_vdd0p94_t tlk_power_vdd0p94_cal_info)
{
    union tlk_areg_power4 power4 = {.bit.tr_dcdc_ldo_1p8 = TLK_POWER_TRIM_VDDF_TO_1P80V};
    union tlk_areg_power7 power7 = {.bit.dcdc_trim_flash_out = TLK_POWER_TRIM_VDDF_TO_1P80V};

    if (value == TLK_POWER_CAL_0P94V_TO_0P95V)
    {
        power4.bit.tr_dcdc_ldo_0p94  = tlk_power_vdd0p94_cal_info.ldo_0p95v;
        power7.bit.dcdc_trim_soc_out = tlk_power_vdd0p94_cal_info.dcdc_0p95v;
    }
    else if (value == TLK_POWER_CAL_0P94V_TO_1P05V)
    {
        power4.bit.tr_dcdc_ldo_0p94  = tlk_power_vdd0p94_cal_info.ldo_1p05v;
        power7.bit.dcdc_trim_soc_out = tlk_power_vdd0p94_cal_info.dcdc_1p05v;
    }

    TLK_ANALOG_WRITE(TLK_AREG_POWER4, power4);
    TLK_ANALOG_WRITE(TLK_AREG_POWER7, power7);
}

_tlk_attribute_ram_code_sec_ static void
tlk_power_trim_ddo1p8v(tlk_power_trim_vddo1p8_e trim_vddo1p8)
{
    TLK_ANALOG_MODIFY(TLK_AREG_POWER3, (value.bit.vbat_ldo1p8_trim_3v = trim_vddo1p8;));
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

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_configure(enum tlk_power_vbat vbat_v,
                                                               enum tlk_power_mode power_mode)
{
    enum tlk_sys_reboot_reason reboot_reason              = tlk_sys_get_reboot_reason();
    tlk_power_cal_vdd0p94_t    tlk_power_vdd0p94_cal_info = {TLK_POWER_TRIM_VDD0P94_TO_0P963V,
                                                             TLK_POWER_TRIM_VDD0P94_TO_0P963V,
                                                             TLK_POWER_TRIM_VDD0P94_TO_1P078V,
                                                             TLK_POWER_TRIM_VDD0P94_TO_1P078V};
    uint8_t                    tlk_g_pm_cal_vddo1p8_info  = TLK_POWER_TRIM_VDDO1P8_TO_1P832V;

    /*
     * If back from deep ret sleep, there is no need to read or set calibration value cause they are
     * all maintained. If back from deep sleep, the calibration value need to read but not need to
     * set cause they are all maintained. For other case(power on or reboot), it's necessary to both
     * read and set calibration value for both LDO and DCDC mode.
     */
    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION)
    {
        /* Read calibration values from OTP */
        tlk_otp_get_vdd0p94_vddo1p8_calib_value(&tlk_power_vdd0p94_cal_info,
                                                &tlk_g_pm_cal_vddo1p8_info);
    }

    tlk_power_set_mode(power_mode);

    if (TLK_ANALOG_READ(TLK_AREG_WD_BUFFER_CLR0).bit.poweron_flag)
    {
        tlk_power_trim_vdd0p94v(TLK_POWER_CAL_0P94V_TO_0P95V, tlk_power_vdd0p94_cal_info);
        tlk_power_trim_ddo1p8v(tlk_g_pm_cal_vddo1p8_info);
    }

    // After waking up, it is not safe to power supply both the native LDO and the normal LDO
    // together.
    TLK_ANALOG_MODIFY(TLK_AREG_POWER6, (
        value.bit.mscn_pullup_res_en_b = 0;
        value.bit.pd_nvt_0p94 = 1;
        value.bit.pd_nvt_1p8 = 1;
    ));

    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN1, (
        value.bit.pd_vbat_switch = ~(vbat_v);
        value.bit.pd_spd_ldo = 1;
        value.bit.pd_ret_ldo = 1;
    ));

    if (tlk_g_chip_version == TLK_CHIP_VERSION_A1)
    {
        // This will reduce power consumption and has little impact to operate early or late.
        TLK_ANALOG_MODIFY(TLK_AREG_PGA_CTRL2, (value.bit.audio_vmid_pd = 1;));

        if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION)
        {
            /*
             * A1 version add 0x13<3> as bb power switch to fix the ISSUE(TER-32).
             * vdd_bb was powered by dig_ldo/spd_ldo instead dcore_ldo, so here need to power it up
             * first.
             */
            TLK_ANALOG_MODIFY(TLK_AREG_POWER13, (value.bit.a1_bb_power_sw = 0;));
        }
    }
}
