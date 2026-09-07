#include "core/include/tlk_power.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_sys.h"
#include "registers/tlk_analog.h"

#define TLK_POWER_INTERNAL_CAP_VALUE 0x20

union tlk_areg_power4 tlk_power_status = {.raw = 0};

_tlk_attribute_ram_code_sec_noinline_ void
tlk_power_set_dig_modules_state(enum tlk_power_dig_module modules, bool enable)
{
    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN2, (
        if (enable) { TLK_BM_CLR(value.raw, modules);
        } else { TLK_BM_SET(value.raw, modules);
        }

        value.bit.pg_clk_en = 1;
    ));

    tlk_clock_cclk_delay((unsigned long long) (5 * tlk_sys_clk.cclk));
    ;
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_init(enum tlk_power_gpio_voltage power_level,
                                                          enum tlk_power_cap          cap)
{
    enum tlk_sys_reboot_reason reboot_reason = tlk_sys_get_reboot_reason();
    // The value must be set to 1 for the VBUS and 0 or 1 for the VBAT.
    // In order to simplify usage, there is no distinction between VBAT and VBUS power supplies at
    // the user level.

    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION)
    {
        TLK_ANALOG_MODIFY(TLK_AREG_POWER8, (
            value.bit.chg_ldo_sw_3p3v_1p8v = (power_level == TLK_POWER_GPIO_VOLTAGE_1V8);
            value.bit.pd_vbatlao_1p2_aon = 0;   /* Clear LDO calibrations */
            value.bit.trim_vbat_lcldo = 0b110;  /* Set LCLDO to 3.5V output */
            value.bit.trim_vbat_ldo = 0b110;    /* Set VBAT LDO output to 3.5V*/
        ));

        TLK_ANALOG_MODIFY(TLK_AREG_POWER7, (value.bit.trim_vbat_aoldo = 0b110;)); /* Set VBAT LDO to 3.5V */
    }

    /* If the crystal oscillator uses an external capacitor, the internal capacitor must be turned
     * off at the very beginning */
    tlk_clock_24m_xtal_configure_cap((struct tlk_clock_24m_xtal_cap){
        .use_external_cap   = cap == TLK_POWER_EXTERNAL_CAP_XTAL24M,
        .internal_cap_value = TLK_POWER_INTERNAL_CAP_VALUE,
    });
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_configure(enum tlk_power_vbat vbat_v,
                                                               enum tlk_power_mode power_mode)
{
    enum tlk_sys_reboot_reason reboot_reason = tlk_sys_get_reboot_reason();

    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION)
    {
        tlk_power_status = TLK_ANALOG_READ(TLK_AREG_POWER4);
    }
    if (tlk_power_status.bit.pd_dcdc_ldo_sw != power_mode)
    {
        tlk_power_status.bit.pd_dcdc_ldo_sw = power_mode;
        TLK_ANALOG_WRITE(TLK_AREG_POWER4, tlk_power_status);
    }

    TLK_ANALOG_MODIFY(TLK_AREG_POWER5, (
        value.bit.mscn_pullup_res_en_b = 0;
        value.bit.pd_nvt_1p2 = 1;
        value.bit.pd_nvt_2p0 = 1;
    ));

    TLK_ANALOG_MODIFY(TLK_AREG_POWER1, (
        value.bit.ldo_flash_bypass_en = 0;
        value.bit.ldo_ret_trim = 0b100; /* Set retention LDO output to 0.799V */
        value.bit.ldo_spd_trim = 0b111; /* Set suspend LDO output to 1.15V */
    ));

    // The supply voltage of the SRAM needs to be more than 1.2V, so the output voltage of the SRAM
    // LDO is set to 1.2V. When passing through the SRAM LDO, the voltage will drop, so the input
    // voltage of the SRAM LDO needs to be above 1.2V+100mV, so the voltage of 1.2V needs to be
    // configured above 1.3V.(add by weihua.zhang, confirmed by wenfeng.lou 20230607)

    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN1, (
        value.bit.pd_vbus_switch = !(vbat_v);
        value.bit.pd_dcore_ldo = 0;
        value.bit.pd_sram_ldo = 0;
        value.bit.pd_spd_ldo = 1;
        value.bit.pd_ret_ldo = 1;
    ));

    union tlk_areg_power3 power3 = {.bit = {
                                        .tr_dcdc_ldo_1p2 = 0b11, /* 1.293V ( default ) */
                                        .tr_ldo_2p0      = 0b01, /* 1.989V ( default ) */
                                        .pd_sw_dcore     = 1,
                                        .pd_sw_sram      = 1,
                                    }};
    TLK_ANALOG_WRITE(TLK_AREG_POWER3, power3);

    union tlk_areg_bg_ctrl0 bg_ctrl = {.bit = {
                                           .bandgap_trim_3v = 0b100, /* Default : 1.26V */
                                           .pd_bg_ts        = 1,
                                           .ldo_main_trim   = 0b100, /* 1.0V */
                                       }};
    TLK_ANALOG_WRITE(TLK_AREG_BG_CTRL0, bg_ctrl);

    tlk_power_set_dig_modules_state(TLK_POWER_BASEBAND | TLK_POWER_USB, true);

    union tlk_areg_power0 power0 = {
        .bit = {
            .bbpll_ldo_trim = 0b001, /* 1V */
            .ana_ldo_trim   = 0b100, /* According to the previous comment, it is 1.2V, but according
                                        to the doc, it's 0.999V */
        }};
    TLK_ANALOG_WRITE(TLK_AREG_POWER0, power0);
}
