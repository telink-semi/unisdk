#ifndef UART_DRIVER_HPP
#define UART_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_uart.h"

#ifdef __cplusplus
}
#endif

class UARTDriver
{
  public:
    // constexpr + member-initializer list so global instances are constant-initialized
    // (baked into .data at compile time) instead of requiring a runtime call through
    // .init_array, which this SDK's startup code never walks.
    explicit constexpr UARTDriver(tlk_uart_module uart_num) : device_(uart_num) {}

    void configure(tlk_uart_config* config)
    {
        tlk_uart_configure(device_, config);
    }

    void configurePinmux(tlk_uart_config* config)
    {
        tlk_uart_configure_pinmux(device_, config);
    }

    tlk_uart_status sendBytes(const uint8_t* addr, uint32_t size, tlk_uart_tx_handler_t tx_handler,
                              uint32_t timeout_us)
    {
        return tlk_uart_send_bytes(device_, addr, size, tx_handler, timeout_us);
    }

    tlk_uart_status receiveBytes(uint8_t* addr, uint32_t size, tlk_uart_rx_handler_t rx_handler)
    {
        return tlk_uart_receive_bytes(device_, addr, size, rx_handler);
    }

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_FLOW_CONTROL)
    void configureFlowControl(tlk_uart_flow_control_config* config)
    {
        tlk_uart_configure_flow_control(device_, config);
    }

    void configureFlowControlPinmux(tlk_uart_flow_control_config* config)
    {
        tlk_uart_configure_flow_control_pinmux(device_, config);
    }
#endif

  private:
    tlk_uart_module device_;
};

#endif // UART_DRIVER_HPP
