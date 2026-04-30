#ifndef TLK_INCLUDE_DRIVERS_GPIO_H_
#define TLK_INCLUDE_DRIVERS_GPIO_H_

#include "common/include/tlk_linked_list.h"
#include "properties/tlk_gpio.h"

#define __TLK_DECLARE_GPIO_PORT(num, name) TLK_GPIO_PORT_##name = num,
#define _TLK_DECLARE_GPIO_PORT(num, name) __TLK_DECLARE_GPIO_PORT(num, name)
#define TLK_DECLARE_GPIO_PORT(num) _TLK_DECLARE_GPIO_PORT(num, UNISDK_GPIO_PORT_NAMES_##num)

#define TLK_DECLARE_GPIO_PORT_IF_ENABLED(num) IF_ENABLED(CONFIG_TLK_GPIO_PORT_##num##_ENABLED, (TLK_DECLARE_GPIO_PORT(num)))

enum tlk_gpio_port
{
    FOR_CALL(UNISDK_GPIO_PORT_COUNT, TLK_DECLARE_GPIO_PORT_IF_ENABLED)
};

enum tlk_gpio_pin
{
    TLK_GPIO_PIN_NONE = 0,
    TLK_GPIO_PIN_0 = TLK_BIT(0),
    TLK_GPIO_PIN_1 = TLK_BIT(1),
    TLK_GPIO_PIN_2 = TLK_BIT(2),
    TLK_GPIO_PIN_3 = TLK_BIT(3),
    TLK_GPIO_PIN_4 = TLK_BIT(4),
    TLK_GPIO_PIN_5 = TLK_BIT(5),
    TLK_GPIO_PIN_6 = TLK_BIT(6),
    TLK_GPIO_PIN_7 = TLK_BIT(7),
};

struct tlk_gpio_port_pin {
    enum tlk_gpio_port port;
    enum tlk_gpio_pin pin;
};

#define __TLK_DECLARE_GPIO_PULL_UP(name, value)     TLK_GPIO_INPUT_PULL_UP_##name = value,
#define _TLK_DECLARE_GPIO_PULL_UP(name, value)      __TLK_DECLARE_GPIO_PULL_UP(name, value)
#define TLK_DECLARE_GPIO_PULL_UP(number)            _TLK_DECLARE_GPIO_PULL_UP(UNISDK_GPIO_PULL_UP_OPTIONS_##number##_NAME, UNISDK_GPIO_PULL_UP_OPTIONS_##number##_VALUE)

#define __TLK_DECLARE_GPIO_PULL_DOWN(name, value)   TLK_GPIO_INPUT_PULL_DOWN_##name = value,
#define _TLK_DECLARE_GPIO_PULL_DOWN(name, value)    __TLK_DECLARE_GPIO_PULL_DOWN(name, value)
#define TLK_DECLARE_GPIO_PULL_DOWN(number)          _TLK_DECLARE_GPIO_PULL_DOWN(UNISDK_GPIO_PULL_DOWN_OPTIONS_##number##_NAME, UNISDK_GPIO_PULL_DOWN_OPTIONS_##number##_VALUE)

#define __TLK_DEFAULT_PULL_UP(name)     TLK_GPIO_INPUT_PULL_UP_##name
#define _TLK_DEFAULT_PULL_UP(name)      __TLK_DEFAULT_PULL_UP(name)
#define TLK_DEFAULT_PULL_UP             _TLK_DEFAULT_PULL_UP(UNISDK_GPIO_PULL_UP_DEFAULT)

#define __TLK_DEFAULT_PULL_DOWN(name)   TLK_GPIO_INPUT_PULL_DOWN_##name
#define _TLK_DEFAULT_PULL_DOWN(name)    __TLK_DEFAULT_PULL_DOWN(name)
#define TLK_DEFAULT_PULL_DOWN           _TLK_DEFAULT_PULL_DOWN(UNISDK_GPIO_PULL_DOWN_DEFAULT)

enum tlk_gpio_mode {
    TLK_GPIO_INPUT_NO_PULL = 0,

    FOR_CALL(UNISDK_GPIO_PULL_UP_OPTIONS_COUNT, TLK_DECLARE_GPIO_PULL_UP)
    FOR_CALL(UNISDK_GPIO_PULL_DOWN_OPTIONS_COUNT, TLK_DECLARE_GPIO_PULL_DOWN)

    TLK_GPIO_INPUT_PULL_UP = TLK_DEFAULT_PULL_UP,
    TLK_GPIO_INPUT_PULL_DOWN = TLK_DEFAULT_PULL_DOWN,

    TLK_GPIO_OUTPUT = -1,
};

#define __DECLARE_GPIO_MUX_FUNCTION(name, reg_value) \
    TLK_GPIO_MUX_##name = reg_value,

#define _DECLARE_GPIO_MUX_FUNCTION(name, reg_value) \
    __DECLARE_GPIO_MUX_FUNCTION(name, reg_value)

#define DECLARE_GPIO_MUX_FUNCTION(num) \
    _DECLARE_GPIO_MUX_FUNCTION(UNISDK_GPIO_FUNCTION_##num##_NAME, UNISDK_GPIO_FUNCTION_##num##_REG_VALUE)

enum tlk_gpio_mux {
    FOR_CALL(UNISDK_GPIO_FUNCTION_COUNT, DECLARE_GPIO_MUX_FUNCTION)
};

#if IS_ENABLED(CONFIG_TLK_PLIC)

enum tlk_gpio_intr_trigger
{
    TLK_GPIO_INTR_RISING_EDGE = 0,
    TLK_GPIO_INTR_FALLING_EDGE,
    TLK_GPIO_INTR_HIGH_LEVEL,
    TLK_GPIO_INTR_LOW_LEVEL,
    TLK_GPIO_INTR_BOTH_EDGE,
};

typedef void(*tlk_gpio_irq_callback_handler)(enum tlk_gpio_port, enum tlk_gpio_pin);

struct tlk_gpio_irq_callback 
{
    struct tlk_list_node node;
    tlk_gpio_irq_callback_handler handler;
    enum tlk_gpio_pin pin;
};

#endif



/**
 * @brief Disable a GPIO pin.
 *
 * Turns off the pin's functionality and resets its configuration. 
 * 
 * @param port  GPIO port.
 * @param pin   GPIO pin.
 * 
 * @return None.
 */
void tlk_gpio_disable(enum tlk_gpio_port port, enum tlk_gpio_pin pin);

/**
 * @brief Configure a GPIO pin.
 *
 * Sets the GPIO mode for the specified port and pin.
 * The pin can be configured for:
 * - Input (with optional pull-up or pull-down).
 * - Output.
 *
 * @param port  GPIO port.
 * @param pin   GPIO pin.
 * @param mode  GPIO mode (output/input pull-up/pull-down).
 * 
 * @return None.
 */
void tlk_gpio_configure(enum tlk_gpio_port port, enum tlk_gpio_pin pin, enum tlk_gpio_mode mode);

/**
 * @brief Write a logical value to a GPIO pin.
 *
 * Sets the pin output to high (`true`) or low (`false`).
 *
 * @param port  GPIO port.
 * @param pin   GPIO pin.
 * @param value Logic level (true = high, false = low).
 * 
 * @return None.
 */
void tlk_gpio_pin_write(enum tlk_gpio_port port, enum tlk_gpio_pin pin, bool value);

/**
 * @brief Toggle the state of a GPIO output pin.
 *
 * Inverts the current output value.
 *
 * @param port  GPIO port.
 * @param pin   GPIO pin.
 * 
 * @return None.
 */
void tlk_gpio_pin_toggle(enum tlk_gpio_port port, enum tlk_gpio_pin pin);

/**
 * @brief Read the current logical level of a GPIO pin.
 *
 * Reads the pin state regardless of mode.
 *
 * @param port  GPIO port.
 * @param pin   GPIO pin.
 *
 * @return `true` if high, `false` if low.
 */
bool tlk_gpio_pin_read(enum tlk_gpio_port port, enum tlk_gpio_pin pin);

/**
 * @brief Set the pin multiplexing mode.
 *
 * Configures the pin's alternate function (MUX).
 *
 * @param port  GPIO port.
 * @param pin   GPIO pin.
 * @param mux   Multiplexer index for the certain function.
 * 
 * @return None.
 */
void tlk_gpio_set_mux(enum tlk_gpio_port port, enum tlk_gpio_pin pin, enum tlk_gpio_mux mux);

#if IS_ENABLED(CONFIG_TLK_PLIC)
/**
 * @brief Configure GPIO interrupt trigger.
 *
 * Sets the interrupt trigger type for the given pin.
 *
 * @param port    GPIO port.
 * @param pin     GPIO pin.
 * @param trigger Trigger mode.
 * 
 * @return None.
 */
void tlk_gpio_irq_configure(enum tlk_gpio_port port, enum tlk_gpio_pin pin, enum tlk_gpio_intr_trigger trigger);

/**
 * @brief Register a callback for a GPIO interrupt.
 *
 * Adds the callback to the internal callback list for the specified port.
 *
 * @param port     GPIO port.
 * @param callback Pointer to callback structure.
 * 
 * @return None.
 */
void tlk_gpio_irq_add_callback(enum tlk_gpio_port port, struct tlk_gpio_irq_callback *callback);

#endif

#endif