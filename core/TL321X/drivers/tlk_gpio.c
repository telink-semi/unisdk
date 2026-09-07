#include "core/include/tlk_gpio.h"
#include "common/include/tlk_init.h"
#include "core/include/tlk_analog.h"
#include "properties/tlk_plic.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_gpio.h"

struct tlk_gpio_port_handle
{
#if TLK_IS_ENABLED(CONFIG_TLK_PLIC)
    struct
    {
        struct tlk_linked_list callbacks;
        volatile uint8_t       previous_in_state;
        volatile uint8_t       both_edge_en;
        volatile uint8_t       falling_edge_en;
        volatile uint8_t       rising_edge_en;
    } irq;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PM_DEVICE)
    struct
    {
        uint16_t pupd;
        uint8_t  function;
        uint8_t  input_enable;
        uint8_t  irq_enable;
        uint8_t  output;
        uint8_t  output_enable;
        uint8_t  polarity;
    } retention;
#endif
};

#define TLK_DECLARE_GPIO_PORT_HANDLE(num) static struct tlk_gpio_port_handle tlk_gpio##num;

#define TLK_DECLARE_GPIO_PORT_HANDLE_IF_ENABLED(num)                                               \
    TLK_IF_ENABLED(CONFIG_TLK_GPIO_PORT_##num##_ENABLED, (TLK_DECLARE_GPIO_PORT_HANDLE(num)))

#define TLK_PAST_GPIO_PORT_HANDLE_OR_NULL(num)                                                     \
    TLK_IF_ENABLED_ELSE(CONFIG_TLK_GPIO_PORT_##num##_ENABLED, (&tlk_gpio##num, ), (NULL, ))

TLK_FOR_CALL(UNISDK_GPIO_PORT_COUNT, TLK_DECLARE_GPIO_PORT_HANDLE_IF_ENABLED)

struct tlk_gpio_port_handle* tlk_gpio[UNISDK_GPIO_PORT_COUNT] = {
    TLK_FOR_CALL(UNISDK_GPIO_PORT_COUNT, TLK_PAST_GPIO_PORT_HANDLE_OR_NULL)};

struct
{
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PM_DEVICE)
    struct
    {
        uint8_t irq_ctrl;
        uint8_t irq_src_mask;
    } retention;
#endif
} tlk_gpio_handle;

/* Helper functions */

static void tlk_gpio_enable(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    TLK_BM_SET(tlk_gpio_reg.port_config[port].function, pin);
}

static void tlk_gpio_set_high_level(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    tlk_gpio_reg.port_config[port].output_set = pin;
    TLK_IF_ENABLED(CONFIG_TLK_GPIO_PM_DEVICE, (TLK_BM_SET(tlk_gpio[port]->retention.output, pin);))
}

static void tlk_gpio_set_low_level(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    tlk_gpio_reg.port_config[port].output_clear = pin;
    TLK_IF_ENABLED(CONFIG_TLK_GPIO_PM_DEVICE, (TLK_BM_CLR(tlk_gpio[port]->retention.output, pin);))
}

static void tlk_gpio_output_en(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    TLK_BM_CLR(tlk_gpio_reg.port_config[port].output_en, pin);
}

static void tlk_gpio_output_dis(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    TLK_BM_SET(tlk_gpio_reg.port_config[port].output_en, pin);
}

static void tlk_gpio_input_en(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_B_ENABLED)
    if ((port == TLK_GPIO_PORT_B) && (pin >= TLK_GPIO_PIN_4) && (pin <= TLK_GPIO_PIN_7))
    {
        TLK_ANALOG_MODIFY_RAW8(TLK_AREG_PB_IE, (value |= pin;));
    }
    else
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_C_ENABLED)
        if (port == TLK_GPIO_PORT_C)
    {
        TLK_ANALOG_MODIFY_RAW8(TLK_AREG_PC_IE, (value |= pin;));
    }
    else
#endif
    {
        TLK_BM_SET(tlk_gpio_reg.port_config[port].input_en, pin);
    }
}

static void tlk_gpio_input_dis(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_B_ENABLED)
    if ((port == TLK_GPIO_PORT_B) && (pin >= TLK_GPIO_PIN_4) && (pin <= TLK_GPIO_PIN_7))
    {
        TLK_ANALOG_MODIFY_RAW8(TLK_AREG_PB_IE, (value &= ~pin;));
    }
    else
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_C_ENABLED)
        if (port == TLK_GPIO_PORT_C)
    {
        TLK_ANALOG_MODIFY_RAW8(TLK_AREG_PC_IE, (value &= ~pin;));
    }
    else
#endif
    {
        TLK_BM_CLR(tlk_gpio_reg.port_config[port].input_en, pin);
    }
}

static void tlk_gpio_set_mode(enum tlk_gpio_port port, enum tlk_gpio_pin pin,
                              enum tlk_gpio_mode mode)
{
    uint8_t pin_num   = TLK_BIT_LOW_BIT(pin);
    uint8_t shift_num = pin_num * 2; // 2 bits per each pin in the register
    uint8_t areg      = TLK_AREG_PX_PUPD(port);

    TLK_ANALOG_MODIFY_RAW16(areg, (value &= ~(0b11 << shift_num); value |= (mode << shift_num);));
}

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC)

#include "core/include/tlk_plic.h"

static void tlk_gpio_irq_en(enum tlk_gpio_port port, enum tlk_gpio_pin pin, int32_t irq)
{
    TLK_BM_SET(tlk_gpio_reg.port_config[port].irq_en[irq], pin);
}

static void tlk_gpio_irq_clr(uint8_t status)
{
    tlk_gpio_reg.irq_status = status;
}

static void tlk_gpio_irq_configure_trigger(enum tlk_gpio_port port, enum tlk_gpio_pin pin,
                                           enum tlk_gpio_intr_trigger trigger, uint8_t irq_num)
{
    switch (trigger)
    {
    case TLK_GPIO_INTR_BOTH_EDGE:
        if (tlk_gpio_pin_read(port, pin))
        {
            TLK_BM_SET(tlk_gpio_reg.port_config[port].polarity, pin);
        }
        else
        {
            TLK_BM_CLR(tlk_gpio_reg.port_config[port].polarity, pin);
        }
        TLK_BM_CLR(tlk_gpio_reg.irq_level_mode, TLK_BIT(irq_num));
        break;

    case TLK_GPIO_INTR_RISING_EDGE:
        TLK_BM_CLR(tlk_gpio_reg.port_config[port].polarity, pin);
        TLK_BM_CLR(tlk_gpio_reg.irq_level_mode, TLK_BIT(irq_num));
        break;

    case TLK_GPIO_INTR_FALLING_EDGE:
        TLK_BM_SET(tlk_gpio_reg.port_config[port].polarity, pin);
        TLK_BM_CLR(tlk_gpio_reg.irq_level_mode, TLK_BIT(irq_num));
        break;

    case TLK_GPIO_INTR_HIGH_LEVEL:
        TLK_BM_CLR(tlk_gpio_reg.port_config[port].polarity, pin);
        TLK_BM_SET(tlk_gpio_reg.irq_level_mode, TLK_BIT(irq_num));
        break;

    case TLK_GPIO_INTR_LOW_LEVEL:
        TLK_BM_SET(tlk_gpio_reg.port_config[port].polarity, pin);
        TLK_BM_SET(tlk_gpio_reg.irq_level_mode, TLK_BIT(irq_num));
        break;
    }
}

#endif

/* GPIO driver API */

void tlk_gpio_disable(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    TLK_BM_CLR(tlk_gpio_reg.port_config[port].function, pin);
}

void tlk_gpio_configure(enum tlk_gpio_port port, enum tlk_gpio_pin pin, enum tlk_gpio_mode mode)
{
    tlk_gpio_enable(port, pin);
    if (mode == TLK_GPIO_OUTPUT)
    {
        tlk_gpio_input_dis(port, pin);
        tlk_gpio_output_en(port, pin);
    }
    else
    {
        tlk_gpio_output_dis(port, pin);
        tlk_gpio_input_en(port, pin);
        tlk_gpio_set_mode(port, pin, mode);
    }
}

void tlk_gpio_pin_write(enum tlk_gpio_port port, enum tlk_gpio_pin pin, bool value)
{
    if (value)
    {
        tlk_gpio_set_high_level(port, pin);
    }
    else
    {
        tlk_gpio_set_low_level(port, pin);
    }
}

void tlk_gpio_pin_toggle(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    tlk_gpio_reg.port_config[port].output_toggle = pin;
    TLK_IF_ENABLED(CONFIG_TLK_GPIO_PM_DEVICE, (TLK_BM_FLIP(tlk_gpio[port]->retention.output, pin);))
}

bool tlk_gpio_pin_read(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    return TLK_BM_IS_SET(tlk_gpio_reg.port_config[port].input, pin);
}

void tlk_gpio_set_mux(enum tlk_gpio_port port, enum tlk_gpio_pin pin, enum tlk_gpio_mux mux)
{
    tlk_gpio_reg.pin_func[port][TLK_BIT_LOW_BIT(pin)] = mux;
}

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC)

static enum tlk_gpio_pin tlk_gpio_port_read(enum tlk_gpio_port port)
{
    return tlk_gpio_reg.port_config[port].input;
}

void tlk_gpio_irq_configure(enum tlk_gpio_port port, enum tlk_gpio_pin pin,
                            enum tlk_gpio_intr_trigger trigger)
{
    int32_t irq_num = 0;

    if (irq_num == 0)
    {
        tlk_gpio_reg.irq_control_bit.gpio_irq_en =
            1; // Only GPIO_IRQ0 needs to enable FLD_GPIO_CORE_INTERRUPT_EN.
    }

    tlk_gpio_irq_en(port, pin, irq_num);
    tlk_gpio_irq_clr(TLK_BIT(irq_num));

    switch (trigger)
    {
    case TLK_GPIO_INTR_BOTH_EDGE:
        TLK_BM_SET(tlk_gpio[port]->irq.both_edge_en, pin);
        TLK_BM_CLR(tlk_gpio[port]->irq.falling_edge_en, pin);
        TLK_BM_CLR(tlk_gpio[port]->irq.rising_edge_en, pin);
        break;
    case TLK_GPIO_INTR_FALLING_EDGE:
        TLK_BM_CLR(tlk_gpio[port]->irq.both_edge_en, pin);
        TLK_BM_SET(tlk_gpio[port]->irq.falling_edge_en, pin);
        TLK_BM_CLR(tlk_gpio[port]->irq.rising_edge_en, pin);
        break;
    case TLK_GPIO_INTR_RISING_EDGE:
        TLK_BM_CLR(tlk_gpio[port]->irq.both_edge_en, pin);
        TLK_BM_CLR(tlk_gpio[port]->irq.falling_edge_en, pin);
        TLK_BM_SET(tlk_gpio[port]->irq.rising_edge_en, pin);
        break;
    default:
        break;
    }

    if (tlk_gpio_pin_read(port, pin))
    {
        TLK_BM_SET(tlk_gpio[port]->irq.previous_in_state, TLK_BIT(pin));
    }
    else
    {
        TLK_BM_CLR(tlk_gpio[port]->irq.previous_in_state, TLK_BIT(pin));
    }

    if (irq_num == 0)
    {
        trigger = TLK_GPIO_INTR_BOTH_EDGE;
    }

    tlk_gpio_irq_configure_trigger(port, pin, trigger, irq_num);

    TLK_BM_SET(tlk_gpio_reg.irq_mask, TLK_BIT(irq_num));

    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_GPIO_SRC0);
}

void tlk_gpio_irq_add_callback(enum tlk_gpio_port port, struct tlk_gpio_irq_callback* callback)
{
    tlk_list_prepend(&tlk_gpio[port]->irq.callbacks, &callback->node);
}

_tlk_attribute_ram_code_sec_ void tlk_gpio_irq_handle(void)
{
    for (int32_t port = 0; port < UNISDK_GPIO_PORT_COUNT; port++)
    {
        if (tlk_gpio[port] == NULL)
        {
            continue;
        }

        uint8_t current_state = tlk_gpio_port_read(port);
        uint8_t changed_pins  = current_state ^ tlk_gpio[port]->irq.previous_in_state;

        if (!changed_pins)
        {
            continue;
        }

        tlk_gpio[port]->irq.previous_in_state = current_state;

        uint8_t changed_to_0 = changed_pins & (~current_state);
        uint8_t changed_to_1 = changed_pins & current_state;

        /* flip the actual polarity */
        TLK_BM_FLIP(tlk_gpio_reg.port_config[port].polarity, changed_pins);

        uint8_t both_edge_triggered    = changed_pins & tlk_gpio[port]->irq.both_edge_en;
        uint8_t falling_edge_triggered = changed_to_0 & tlk_gpio[port]->irq.falling_edge_en;
        uint8_t rising_edge_triggered  = changed_to_1 & tlk_gpio[port]->irq.rising_edge_en;

        uint8_t irq_triggered =
            both_edge_triggered | falling_edge_triggered | rising_edge_triggered;

        if (!irq_triggered)
        {
            continue;
        }

        struct tlk_gpio_irq_callback* cb;
        TLK_LIST_FOR_EACH(&tlk_gpio[port]->irq.callbacks, cb, struct tlk_gpio_irq_callback, node)
        {
            if (cb->pin & irq_triggered)
            {
                cb->handler(port, irq_triggered);
            }
        }
    }

    tlk_gpio_irq_clr(TLK_BIT(0));
}

TLK_PLIC_ISR_REGISTER(tlk_gpio_irq_handle, UNISDK_PLIC_IRQ_NUM_GPIO_SRC0)

#endif

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PREVENT_SLEEP)

static bool tlk_gpio_prevent_sleep(void)
{
    for (int32_t i = 0; i < UNISDK_GPIO_PORT_COUNT; i++)
    {
        if (tlk_gpio_reg.port_config[i].function & ~tlk_gpio_reg.port_config[i].output_en)
        {
            return true;
        }
    }

    return false;
}

TLK_REGISTER_PREVENT_SLEEP(tlk_gpio_prevent_sleep)

#endif

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PM_DEVICE)

static void tlk_gpio_save_context(void)
{
    for (int32_t port = 0; port < UNISDK_GPIO_PORT_COUNT; port++)
    {
        tlk_gpio[port]->retention.pupd = tlk_analog_read_reg16(TLK_AREG_PX_PUPD(port));

        tlk_gpio[port]->retention.function = tlk_gpio_reg.port_config[port].function;

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_B_ENABLED)
        if (port == TLK_GPIO_PORT_B)
        {
            tlk_gpio[port]->retention.input_enable =
                (0x0f & tlk_gpio_reg.port_config[port].input_en) |
                (0xf0 & tlk_analog_read_reg8(TLK_AREG_PB_IE));
        }
        else
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_C_ENABLED)
            if (port == TLK_GPIO_PORT_C)
        {
            tlk_gpio[port]->retention.input_enable = tlk_analog_read_reg8(TLK_AREG_PC_IE);
        }
        else
#endif
        {
            tlk_gpio[port]->retention.input_enable = tlk_gpio_reg.port_config[port].input_en;
        }

        tlk_gpio[port]->retention.irq_enable    = tlk_gpio_reg.port_config[port].irq_en[0];
        tlk_gpio[port]->retention.output_enable = tlk_gpio_reg.port_config[port].output_en;
        tlk_gpio[port]->retention.polarity      = tlk_gpio_reg.port_config[port].polarity;
    }

    tlk_gpio_handle.retention.irq_ctrl     = tlk_gpio_reg.irq_control;
    tlk_gpio_handle.retention.irq_src_mask = tlk_gpio_reg.irq_mask;
}

static void tlk_gpio_restore_context(void)
{
    for (int32_t port = 0; port < UNISDK_GPIO_PORT_COUNT; port++)
    {
        tlk_analog_write_reg16(TLK_AREG_PX_PUPD(port), tlk_gpio[port]->retention.pupd);

        tlk_gpio_reg.port_config[port].function = tlk_gpio[port]->retention.function;

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_B_ENABLED)
        if (port == TLK_GPIO_PORT_B)
        {
            tlk_gpio_reg.port_config[port].input_en = 0x0f & tlk_gpio[port]->retention.input_enable;
            tlk_analog_write_reg8(TLK_AREG_PB_IE, 0xf0 & tlk_gpio[port]->retention.input_enable);
        }
        else
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_C_ENABLED)
            if (port == TLK_GPIO_PORT_C)
        {
            tlk_analog_write_reg8(TLK_AREG_PC_IE, tlk_gpio[port]->retention.input_enable);
        }
        else
#endif
        {
            tlk_gpio_reg.port_config[port].input_en = tlk_gpio[port]->retention.input_enable;
        }

        tlk_gpio_reg.port_config[port].irq_en[0]    = tlk_gpio[port]->retention.irq_enable;
        tlk_gpio_reg.port_config[port].output_set   = tlk_gpio[port]->retention.output;
        tlk_gpio_reg.port_config[port].output_clear = ~tlk_gpio[port]->retention.output;
        tlk_gpio_reg.port_config[port].output_en    = tlk_gpio[port]->retention.output_enable;
        tlk_gpio_reg.port_config[port].polarity     = tlk_gpio[port]->retention.polarity;
    }

    tlk_gpio_reg.irq_control = tlk_gpio_handle.retention.irq_ctrl;
    tlk_gpio_reg.irq_mask    = tlk_gpio_handle.retention.irq_src_mask;

    tlk_gpio_irq_clr(TLK_BIT(0));
    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_GPIO_SRC0);
}

TLK_REGISTER_BEFORE_SLEEP(tlk_gpio_save_context, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_AFTER_SLEEP(tlk_gpio_restore_context, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#endif
