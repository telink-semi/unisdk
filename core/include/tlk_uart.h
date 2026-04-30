#ifndef TLK_INCLUDE_DRIVERS_UART_H_
#define TLK_INCLUDE_DRIVERS_UART_H_

#include "core/include/tlk_gpio.h"
#include "properties/tlk_uart.h"

#define TLK_DECLARE_UART(num) TLK_UART##num = num,
#define TLK_DECLARE_UART_IF_ENABLED(num) IF_ENABLED(CONFIG_TLK_UART##num##_ENABLED, (TLK_DECLARE_UART(num)))

enum tlk_uart_module
{
    FOR_CALL(UNISDK_UART_COUNT, TLK_DECLARE_UART_IF_ENABLED)
};

enum tlk_uart_parity
{
    TLK_UART_PARITY_NONE,
    TLK_UART_PARITY_EVEN,
    TLK_UART_PARITY_ODD,
};

enum tlk_uart_stop_bit
{
    TLK_UART_STOP_BIT_ONE          = 0,
    TLK_UART_STOP_BIT_ONE_DOT_FIVE = 1,
    TLK_UART_STOP_BIT_TWO          = 2,
};

enum tlk_uart_status
{
    TLK_UART_OK,
    TLK_UART_TIMEOUT,
    TLK_UART_ERROR
};

struct tlk_uart_config 
{
    uint32_t baudrate;
    enum tlk_uart_parity parity;
    enum tlk_uart_stop_bit stop_bit;
    struct tlk_gpio_port_pin tx_port_pin;
    struct tlk_gpio_port_pin rx_port_pin;
};

#if IS_ENABLED(CONFIG_TLK_UART_FLOW_CONTROL)

enum tlk_uart_flow_control {
    TLK_UART_FLOW_NONE        = 0,
    TLK_UART_FLOW_RTS         = 1,
    TLK_UART_FLOW_CTS         = 2,
    TLK_UART_FLOW_RTS_CTS     = 3,
};

/**
 * @brief UART flow control polarity
 * 
 * TLK_UART_ACTIVE_LOW:
 *     UART stops sending when CTS is low, and sets RTS low to stop receiving 
 *
 * TLK_UART_ACTIVE_HIGH:
 *     UART stops sending when CTS is high, and sets RTS high to stop receiving
 */
enum tlk_uart_flow_polarity {
    TLK_UART_ACTIVE_LOW   = 0,
    TLK_UART_ACTIVE_HIGH  = 1,
};

struct tlk_uart_flow_control_config {
    enum tlk_uart_flow_control flow_control;
    enum tlk_uart_flow_polarity flow_polarity;
    struct tlk_gpio_port_pin rts_port_pin;
    struct tlk_gpio_port_pin cts_port_pin;
};

#endif

typedef void (*tlk_uart_tx_handler_t)(void);
typedef void (*tlk_uart_rx_handler_t)(uint32_t rx_count);

/**
 * @brief Configure the specified UART module.
 *
 * Initializes the UART module with the provided baud rate, parity, and stop bit configuration.
 *
 * @param uart_num       UART module index.
 * @param config         pointer to the structure that contains configuration parameters
 *
 * @return None.
 */
void tlk_uart_configure(enum tlk_uart_module uart_num, struct tlk_uart_config *config);

/**
 * @brief Configure UART TX and RX pin multiplexing.
 *
 * Configures the GPIO pins used for UART transmission (TX) and reception (RX).
 *
 * @param uart_num      UART module index.
 * @param tx_port_pin   TX port and pin.
 * @param rx_port_pin   RX port and pin.
 *
 * @return None.
 */
void tlk_uart_configure_pinmux(enum tlk_uart_module uart_num, struct tlk_uart_config *config);

/**
 * @brief Send data buffer over UART.
 * 
 * 1. If blocking mode is enabled, the function performs blocking
 * transmission until the buffer is fully sent or the timeout expires.
 * 
 * 2. If interrupt or dma modes are enabled, the function stores the
 * buffer information. The provided handler is called when transmission is complete.
 *
 * @param uart_num    UART module index.
 * @param buff        Pointer to the transmit buffer.
 * @param buff_size   Number of bytes to send.
 * @param tx_handler  Callback function which will be called after transmission complete (used in IRQ and DMA modes).
 * @param timeout_us  Timeout in microseconds (used in no IRQ mode).
 *
 * @return TLK_UART_OK on success, TLK_UART_TIMEOUT if timeout occurred, TLK_UART_ERROR on failure.
 */
enum tlk_uart_status tlk_uart_send_bytes(enum tlk_uart_module uart_num, const uint8_t *buff, uint32_t buff_size, tlk_uart_tx_handler_t tx_handler, uint32_t timeout_us);

/**
 * @brief Receive data buffer over UART.
 * 
 * 1. If blocking mode is enabled, the function performs blocking
 * reception until the buffer is filled or the timeout ends.
 * 
 * 2. If interrupt or dma modes are enabled, the function stores the
 * buffer information. The provided handler is called when reception is complete.
 *
 * @param uart_num     UART module index.
 * @param buff         Pointer to the receive buffer.
 * @param buff_size    Number of bytes to receive.
 * @param rx_handler   Callback function which will be called after reception complete (used in IRQ and DMA modes).
 * @param timeout_us   Timeout in microseconds (used in no IRQ mode).
 *
 * @return TLK_UART_OK always in IRQ mode; in no IRQ mode returns TLK_UART_OK on success or TLK_UART_TIMEOUT on timeout.
 */
enum tlk_uart_status tlk_uart_receive_bytes(enum tlk_uart_module uart_num, uint8_t *buff, uint32_t buff_size, tlk_uart_rx_handler_t rx_handler, uint32_t timeout_us);

#if IS_ENABLED(CONFIG_TLK_UART_FLOW_CONTROL)
/**
 * @brief Configure the flow control for the specified UART module.
 * 
 * Disabled by default. Should be called after the tlk_uart_configure.
 *
 * @param uart_num      UART module index.
 * @param config        pointer to the structure that contains configuration parameters
 *
 * @return None.
 */
void tlk_uart_flow_configure(enum tlk_uart_module uart_num, struct tlk_uart_flow_control_config *config);

/**
 * @brief Configure UART RTS and CTS pin multiplexing.
 * 
 * Configures the GPIO pins used for UART flow control signals RTS (Request To Send) and CTS (Clear To Send).
 *
 * @param uart_num     UART module index.
 * @param rts_port_pin RTS port and pin.
 * @param cts_port_pin CTS port and pin.
 *
 * @return None.
 */
void tlk_uart_flow_configure_pinmux(enum tlk_uart_module uart_num, struct tlk_uart_flow_control_config *config);

#endif

#endif