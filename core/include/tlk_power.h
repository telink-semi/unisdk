#ifndef TLK_INCLUDE_DRIVERS_POWER_H_
#define TLK_INCLUDE_DRIVERS_POWER_H_

enum tlk_power_dig_module
{
    TLK_POWER_BASEBAND = TLK_BIT(0), // power down baseband
    TLK_POWER_USB      = TLK_BIT(1), // power down usb
    TLK_POWER_AUDIO    = TLK_BIT(2), // power down audio
};

enum tlk_power_mode
{
    TLK_POWER_LDO_LDO_MODE   = 0x00,
    TLK_POWER_DCDC_LDO_MODE  = 0x01,
    TLK_POWER_DCDC_DCDC_MODE = 0x03,
};

enum tlk_power_cap
{
    TLK_POWER_INTERNAL_CAP_XTAL24M =
        0, /**<    Use the chip's internal crystal capacitors,
    <p>  hardware boards can not have 24M crystal matching capacitors */
    TLK_POWER_EXTERNAL_CAP_XTAL24M =
        1, /**<    Use an external crystal capacitor,
   <p>  the hardware board needs to have a matching capacitor for the 24M crystal,
   <p>  the program will turn off the chip's internal capacitor */
};

enum tlk_power_vbat
{
    TLK_POWER_VBAT_MAX_VALUE_GREATER_THAN_3V6 =
        0, /*  VBAT must be greater than 2.2V. VBAT may be greater than 3.6V. */
    TLK_POWER_VBAT_MAX_VALUE_LESS_THAN_3V6 =
        TLK_BIT(3), /*  VBAT must be below 3.6V. VBAT may be below 2.2V. */
};

enum tlk_power_gpio_voltage
{
    TLK_POWER_GPIO_VOLTAGE_3V3 = 0x00, /**< the GPIO voltage is set to 3.3V. */
    TLK_POWER_GPIO_VOLTAGE_1V8 = 0x01, /**< the GPIO voltage is set to 1.8V. */
};

/**
 * @brief Set the power state of digital modules.
 *
 * @param modules Bitmask of digital modules to control.
 * @param enable  true to power on, false to power down.
 *
 * @return None.
 */
_tlk_attribute_ram_code_sec_noinline_ void
tlk_power_set_dig_modules_state(enum tlk_power_dig_module modules, bool enable);

/**
 * @brief Initialize the power management system.
 *
 * @param power_level GPIO voltage level (3.3V or 1.8V).
 * @param cap         Crystal capacitor configuration (internal or external).
 *
 * @return None.
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_power_init(enum tlk_power_gpio_voltage power_level,
                                                          enum tlk_power_cap          cap);

/**
 * @brief Configure the power mode and VBAT range.
 *
 * @param vbat_v    VBAT voltage range configuration.
 * @param power_mode Power mode (LDO-only, DCDC+LDO, or DCDC-only).
 *
 * @return None.
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_power_configure(enum tlk_power_vbat vbat_v,
                                                               enum tlk_power_mode power_mode);

#endif
