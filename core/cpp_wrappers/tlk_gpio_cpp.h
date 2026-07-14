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
    // Constructor
    GPIODriver(enum tlk_gpio_port port);

    // Implicit conversion to enum tlk_gpio_port
    operator enum tlk_gpio_port() const;

    // Configure a specific pin
    void configure(enum tlk_gpio_pin pinNumber, enum tlk_gpio_mode mode);

    // Read the state of a specific pin
    bool read(enum tlk_gpio_pin pinNumber) const;

    // Write a value to a specific pin
    void write(enum tlk_gpio_pin pinNumber, bool state);

    // Toggle a specific pin
    void toggle(enum tlk_gpio_pin pinNumber);

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC)
    void irq_configure(enum tlk_gpio_pin pin, enum tlk_gpio_intr_trigger trigger);
    void irq_add_callback(struct tlk_gpio_irq_callback* callback);
#endif

    // Set port number
    void setPort(enum tlk_gpio_port port);

  private:
    enum tlk_gpio_port portNum;

    // Helper method to validate pin number
    bool isValidPin(unsigned char pinNumber) const;
};

GPIODriver::GPIODriver(enum tlk_gpio_port port)
{
    portNum = port;
}

GPIODriver::operator enum tlk_gpio_port() const { return portNum; }

void GPIODriver::configure(enum tlk_gpio_pin pinNumber, enum tlk_gpio_mode mode)
{
    if (!isValidPin(pinNumber))
    {
        return; // Invalid pin number
    }
    tlk_gpio_configure(this->portNum, pinNumber, mode);
}

bool GPIODriver::read(enum tlk_gpio_pin pinNumber) const
{
    if (!isValidPin(pinNumber))
    {
        return false; // Invalid pin number
    }

    return tlk_gpio_pin_read(this->portNum, pinNumber);
}

void GPIODriver::write(enum tlk_gpio_pin pinNumber, bool state)
{
    if (!isValidPin(pinNumber))
    {
        return; // Invalid pin number
    }
    tlk_gpio_pin_write(this->portNum, pinNumber, state);
}

void GPIODriver::toggle(enum tlk_gpio_pin pinNumber)
{
    if (!isValidPin(pinNumber))
    {
        return; // Invalid pin number
    }
    tlk_gpio_pin_toggle(this->portNum, pinNumber);
}

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC)
void GPIODriver::irq_configure(enum tlk_gpio_pin pinNumber, enum tlk_gpio_intr_trigger trigger)
{
    if (!isValidPin(pinNumber))
    {
        return; // Invalid pin number
    }

    tlk_gpio_irq_configure(this->portNum, pinNumber, trigger);
}

void GPIODriver::irq_add_callback(struct tlk_gpio_irq_callback* callback)
{
    tlk_gpio_irq_add_callback(this->portNum, callback);
}
#endif

void GPIODriver::setPort(enum tlk_gpio_port port)
{
    portNum = port;
}

bool GPIODriver::isValidPin(unsigned char pinNumber) const
{
    return pinNumber <= TLK_GPIO_PIN_7;
}

#endif // GPIO_DRIVER_HPP
