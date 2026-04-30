#include "core/include/tlk_power.h"
#include "core/include/tlk_sys.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"

#define TLK_POWER_DIG_MODULES_CLOCK TLK_BIT(7)

uint8_t power_status = 0;

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_dig_modules_set_state(enum tlk_power_dig_module modules, bool enable)
{
    uint8_t modules_power_status = tlk_analog_read_reg8(0x7d);
    modules_power_status |=  TLK_POWER_DIG_MODULES_CLOCK;
    if (enable) {
        TLK_BM_CLR(modules_power_status, modules);
    } else {
        TLK_BM_SET(modules_power_status, modules);
    }
    tlk_analog_write_reg8(0x7d, modules_power_status);

    tlk_clock_cclk_delay((unsigned long long)(5 * tlk_sys_clk.cclk));;
}

_tlk_always_inline void tlk_power_turn_off_internal_cap(void)
{
    tlk_analog_write_reg8(0x8a, tlk_analog_read_reg8(0x8a) | TLK_BIT(7));
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_init( enum tlk_power_gpio_voltage power_level, enum tlk_power_cap cap )
{
    enum tlk_sys_reboot_reason reboot_reason = tlk_sys_get_reboot_reason();
    //The value must be set to 1 for the VBUS and 0 or 1 for the VBAT.
    //In order to simplify usage, there is no distinction between VBAT and VBUS power supplies at the user level.
    
    uint8_t ldo_trim_val = tlk_analog_read_reg8(0x19);
    
    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION) {
        if (power_level == TLK_POWER_GPIO_VOLTAGE_1V8) {
            ldo_trim_val &= TLK_BIT(7);                             /* Clear LDO calibrations */
            TLK_BM_SET(ldo_trim_val, TLK_BIT(3));                   /* Set 3.3V LDO output to 1.8V*/
        } else {
            ldo_trim_val &= (TLK_BIT(7) | TLK_BIT(3));              /* Clear LDO calibrations */
        }

        TLK_BM_SET(ldo_trim_val, TLK_BIT_RNG(1,2));                 /* Set LCLDO to 3.5V output */
        TLK_BM_SET(ldo_trim_val, TLK_BIT_RNG(5,6));                 /* Set VBAT LDO output to 3.5V*/
        tlk_analog_write_reg8(0x19, ldo_trim_val);

        uint8_t vbat_aldo_cal_val = tlk_analog_read_reg8(0x18);
        TLK_BM_CLR(vbat_aldo_cal_val, TLK_BIT_RNG(4,6));            /* Clear VBAT AOLDO trim value */
        TLK_BM_SET(vbat_aldo_cal_val, TLK_BIT_RNG(5,6));            /* Set VBAT LDO to 3.5V */
        tlk_analog_write_reg8(0x18, vbat_aldo_cal_val);
    }

    /* If the crystal oscillator uses an external capacitor, the internal capacitor must be turned off at the very beginning */
    if (cap == TLK_POWER_EXTERNAL_CAP_XTAL24M) {
        tlk_power_turn_off_internal_cap();
    }
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_power_configure(enum tlk_power_vbat vbat_v, enum tlk_power_mode power_mode)
{  
    enum tlk_sys_reboot_reason reboot_reason = tlk_sys_get_reboot_reason();
    
    if (reboot_reason != TLK_SYS_REBOOT_REASON_DEEP_RETENTION) {
        power_status = tlk_analog_read_reg8(0x0a); //poweron_dft:   0x90.
    }
    if (TLK_BM_IS_SET(power_status, 0x03) != power_mode) {
        TLK_BM_CLR(power_status, TLK_BIT_RNG(0, 1));
        TLK_BM_SET(power_status, power_mode);
        tlk_analog_write_reg8(0x0a, power_status);
    }

    uint8_t native_dcdc_conf = tlk_analog_read_reg8(0x0b);
    TLK_BM_CLR(native_dcdc_conf, TLK_BIT(6)); /* Clear configurations */
    TLK_BM_SET(native_dcdc_conf, (TLK_BIT(0) | TLK_BIT(1))); /* Power down 1.2V and 2.0V native DCDC */
    tlk_analog_write_reg8(0x0b, native_dcdc_conf);

    uint8_t low_energy_ldo_conf = tlk_analog_read_reg8(0x02);
    TLK_BM_CLR(low_energy_ldo_conf, TLK_BIT(3)); /* Disable flash LDO_flash's bypass mode enable signal */
    TLK_BM_CLR(low_energy_ldo_conf, TLK_BIT_RNG(0,1)); /* Disable flash LDO_flash's bypass mode enable signal */
    TLK_BM_SET(low_energy_ldo_conf, TLK_BIT(2)); /* Set retention LDO output to 0.799V */
    TLK_BM_SET(low_energy_ldo_conf, TLK_BIT_RNG(4,6)); /* Set suspend LDO output to 1.15V */
    tlk_analog_write_reg8(0x02, low_energy_ldo_conf);
    //The supply voltage of the SRAM needs to be more than 1.2V, so the output voltage of the SRAM LDO is set to 1.2V.
    //When passing through the SRAM LDO, the voltage will drop, so the input voltage of the SRAM LDO needs to be above 1.2V+100mV,
    //so the voltage of 1.2V needs to be configured above 1.3V.(add by weihua.zhang, confirmed by wenfeng.lou 20230607)

    uint8_t ldo_outputs_switch = tlk_analog_read_reg8(0x06);
    uint8_t clr_mask = TLK_BIT(0) | vbat_v | TLK_BIT(4) | TLK_BIT(5); /* Disable  bbpll LDO, bypass switch, digital core LDO, SRAM LDO */
    /* TLK_BIT(7) here is confusing because it will power up vbat that should be disabled ath the line above */
    uint8_t set_mask = (TLK_BIT(3) | TLK_BIT(6) | TLK_BIT(7)) & ~clr_mask; /* Disable SPD LDO, retention LDO */

    TLK_BM_CLR(ldo_outputs_switch, clr_mask);
    TLK_BM_SET(ldo_outputs_switch, set_mask);
    tlk_analog_write_reg8(0x06, ldo_outputs_switch);

    uint8_t digital_ldo_switch = tlk_analog_read_reg8(0x09);
    TLK_BM_SET(digital_ldo_switch, TLK_BIT_RNG(6,7)); /* Power down digital LDO to dcore and sram */
    tlk_analog_write_reg8(0x09, digital_ldo_switch);

    uint8_t digital_ldo_cal_value = tlk_analog_read_reg8(0x00);
    TLK_BM_SET(digital_ldo_cal_value, TLK_BIT(7)); /* Set digital LDO to 1V output */
    tlk_analog_write_reg8(0x00, digital_ldo_cal_value);

    tlk_power_dig_modules_set_state(TLK_POWER_BASEBAND | TLK_POWER_USB, true);

    uint8_t analog_ldo_cal_value = tlk_analog_read_reg8(0x01);
    TLK_BM_SET(analog_ldo_cal_value, TLK_BIT(0)); /* Set 001 code for 1V output */
    TLK_BM_SET(analog_ldo_cal_value, TLK_BIT(6)); /* Set 100 code for analog LDO trim to 1.2V */
    tlk_analog_write_reg8(0x01, analog_ldo_cal_value);
}
