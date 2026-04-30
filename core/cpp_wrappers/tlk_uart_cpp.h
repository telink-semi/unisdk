#ifndef UART_DRIVER_HPP
#define UART_DRIVER_HPP

#ifdef __cplusplus
    extern "C" {
#endif
#include "core/include/tlk_uart.h"

#ifdef __cplusplus
    }
#endif

class UARTDriver
{
public:
    // Constructor
    UARTDriver(enum tlk_uart_module uart_num);

    void configure(unsigned int baudrate, enum tlk_uart_parity parity, enum tlk_uart_stop_bit stop_bit,
                   enum tlk_uart_flow_control flow_control, enum tlk_uart_flow_polarity flow_polarity);

    void configure_pinmux(struct tlk_gpio_port_pin tx_port_pin, struct tlk_gpio_port_pin rx_port_pin,
                               struct tlk_gpio_port_pin rts_port_pin, struct tlk_gpio_port_pin cts_port_pin);

    enum tlk_uart_status send_bytes(const unsigned char *addr, unsigned int size, unsigned int timeout_us);
    enum tlk_uart_status receive_bytes(unsigned char *addr, unsigned int size, tlk_uart_rx_handler_t rx_handler, unsigned int timeout_us);
    
private:
    enum tlk_uart_module device;
};

UARTDriver::UARTDriver(enum tlk_uart_module uart_num)
{
    this->device = uart_num;
}

void UARTDriver::configure(unsigned int baudrate, enum tlk_uart_parity parity, enum tlk_uart_stop_bit stop_bit,
                                       enum tlk_uart_flow_control flow_control, enum tlk_uart_flow_polarity flow_polarity)
{
    tlk_uart_configure(this->device, baudrate, parity, stop_bit, flow_control, flow_polarity);
}

void UARTDriver::configure_pinmux(struct tlk_gpio_port_pin tx_port_pin, struct tlk_gpio_port_pin rx_port_pin,
                                       struct tlk_gpio_port_pin rts_port_pin, struct tlk_gpio_port_pin cts_port_pin)
{
    tlk_uart_configure_pinmux(this->device, tx_port_pin, rx_port_pin, rts_port_pin, cts_port_pin);
}

enum tlk_uart_status UARTDriver::send_bytes(const unsigned char *addr, unsigned int size, unsigned int timeout_us)
{
    return tlk_uart_send_bytes(this->device, addr, size, timeout_us);
}

enum tlk_uart_status UARTDriver::receive_bytes(unsigned char *addr, unsigned int size, tlk_uart_rx_handler_t rx_handler, unsigned int timeout_us)
{
    return tlk_uart_receive_bytes(this->device, addr, size, rx_handler, timeout_us);
}

#endif // UART_DRIVER_HPP