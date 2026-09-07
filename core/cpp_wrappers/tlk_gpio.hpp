#ifndef GPIO_DRIVER_HPP
#define GPIO_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_gpio.h"
#ifdef __cplusplus
}
#endif

class GPIODriver
{
  public:
    // constexpr + member-initializer list so global instances are constant-initialized
    // (baked into .data at compile time) instead of requiring a runtime call through
    // .init_array, which this SDK's startup code never walks.
    explicit constexpr GPIODriver(tlk_gpio_port port) : port_(port) {}

    operator tlk_gpio_port() const
    {
        return port_;
    }

    void configure(tlk_gpio_pin pin, tlk_gpio_mode mode)
    {
        if (!isValidPin(pin))
        {
            return;
        }
        tlk_gpio_configure(port_, pin, mode);
    }

    bool read(tlk_gpio_pin pin) const
    {
        if (!isValidPin(pin))
        {
            return false;
        }
        return tlk_gpio_pin_read(port_, pin);
    }

    void write(tlk_gpio_pin pin, bool state)
    {
        if (!isValidPin(pin))
        {
            return;
        }
        tlk_gpio_pin_write(port_, pin, state);
    }

    void toggle(tlk_gpio_pin pin)
    {
        if (!isValidPin(pin))
        {
            return;
        }
        tlk_gpio_pin_toggle(port_, pin);
    }

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC)
    void irqConfigure(tlk_gpio_pin pin, tlk_gpio_intr_trigger trigger)
    {
        if (!isValidPin(pin))
        {
            return;
        }
        tlk_gpio_irq_configure(port_, pin, trigger);
    }

    void irqAddCallback(tlk_gpio_irq_callback* callback)
    {
        tlk_gpio_irq_add_callback(port_, callback);
    }
#endif

    void setPort(tlk_gpio_port port)
    {
        port_ = port;
    }

  private:
    tlk_gpio_port port_;

    bool isValidPin(unsigned char pin) const
    {
        return pin <= TLK_GPIO_PIN_7;
    }
};

#endif // GPIO_DRIVER_HPP
