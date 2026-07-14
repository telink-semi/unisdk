#ifndef TLK_INCLUDE_DRIVERS_UART_H_
#define TLK_INCLUDE_DRIVERS_UART_H_

#include "core/include/tlk_gpio.h"
#include "properties/tlk_uart.h"

#define TLK_UART_DECLARE(num) TLK_UART##num = num,
#define TLK_UART_DECLARE_IF_ENABLED(num)                                                           \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_ENABLED, (TLK_UART_DECLARE(num)))

enum tlk_uart_module
{
    TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_IF_ENABLED)
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

/**
 * @brief Multiplier for RX timeout duration.
 *
 * The total RX timeout = (one UART frame duration) * multiplier.
 * After this silence on the RX line, rxdone_irq is triggered.
 */
enum tlk_uart_rx_timeout_mul
{
    TLK_UART_RX_TIMEOUT_MUL_1 = 0,
    TLK_UART_RX_TIMEOUT_MUL_2,
    TLK_UART_RX_TIMEOUT_MUL_3,
    TLK_UART_RX_TIMEOUT_MUL_4,
};

enum tlk_uart_status
{
    TLK_UART_OK,
    TLK_UART_TIMEOUT,
    TLK_UART_ERROR
};

struct tlk_uart_config
{
    uint32_t                     baudrate;
    enum tlk_uart_parity         parity;
    enum tlk_uart_stop_bit       stop_bit;
    enum tlk_uart_rx_timeout_mul rx_timeout_mul;
    struct tlk_gpio_port_pin     tx_port_pin;
    struct tlk_gpio_port_pin     rx_port_pin;
};

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_FLOW_CONTROL)

enum tlk_uart_flow_control
{
    TLK_UART_FLOW_NONE    = 0,
    TLK_UART_FLOW_RTS     = 1,
    TLK_UART_FLOW_CTS     = 2,
    TLK_UART_FLOW_RTS_CTS = 3,
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
enum tlk_uart_flow_polarity
{
    TLK_UART_ACTIVE_LOW  = 0,
    TLK_UART_ACTIVE_HIGH = 1,
};

struct tlk_uart_flow_control_config
{
    enum tlk_uart_flow_control  flow_control;
    enum tlk_uart_flow_polarity flow_polarity;
    struct tlk_gpio_port_pin    rts_port_pin;
    struct tlk_gpio_port_pin    cts_port_pin;
};

#endif

typedef void (*tlk_uart_tx_handler_t)(void);
typedef void (*tlk_uart_rx_handler_t)(uint32_t rx_count);

/**
 * @brief Configure the specified UART module.
 *
 * Initializes the UART module with the provided configurations.
 *
 * @param uart_num       UART module index.
 * @param config         pointer to the structure that contains configuration parameters
 *
 * @return None.
 */
void tlk_uart_configure(enum tlk_uart_module uart_num, struct tlk_uart_config* config);

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
void tlk_uart_configure_pinmux(enum tlk_uart_module uart_num, struct tlk_uart_config* config);

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
 * @param tx_handler  Callback function which will be called after transmission complete (used in
 * IRQ and DMA modes).
 * @param timeout_us  Timeout in microseconds (used in no IRQ mode).
 *
 * @return TLK_UART_OK on success, TLK_UART_TIMEOUT if timeout occurred, TLK_UART_ERROR on failure.
 */
enum tlk_uart_status tlk_uart_send_bytes(enum tlk_uart_module uart_num, const uint8_t* buff,
                                         uint32_t buff_size, tlk_uart_tx_handler_t tx_handler,
                                         uint32_t timeout_us);

/**
 * @brief Receive data buffer over UART.
 *
 * Function stores the buffer information. The provided handler is called when reception is
 * complete.
 *
 * @param uart_num     UART module index.
 * @param buff         Pointer to the receive buffer.
 * @param buff_size    Number of bytes to receive.
 * @param rx_handler   Callback function which will be called after reception complete.
 *
 * @return TLK_UART_OK.
 */
enum tlk_uart_status tlk_uart_receive_bytes(enum tlk_uart_module uart_num, uint8_t* buff,
                                            uint32_t buff_size, tlk_uart_rx_handler_t rx_handler);

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_FLOW_CONTROL)
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
void tlk_uart_configure_flow_control(enum tlk_uart_module                 uart_num,
                                     struct tlk_uart_flow_control_config* config);

/**
 * @brief Configure UART RTS and CTS pin multiplexing.
 *
 * Configures the GPIO pins used for UART flow control signals RTS (Request To Send) and CTS (Clear
 * To Send).
 *
 * @param uart_num     UART module index.
 * @param rts_port_pin RTS port and pin.
 * @param cts_port_pin CTS port and pin.
 *
 * @return None.
 */
void tlk_uart_configure_flow_control_pinmux(enum tlk_uart_module                 uart_num,
                                            struct tlk_uart_flow_control_config* config);

#endif

#endif
