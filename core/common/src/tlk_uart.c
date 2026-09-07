#include "core/include/tlk_uart.h"
#include "common/include/tlk_init.h"

/* ==== AUTO CONFIG ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_AUTO_CONFIG)
#define TLK_UART_DECLARE_AUTO_CONFIG(num)                                                          \
    __attribute__((used, noinline)) static void tlk_uart##num##_auto_config(void)                  \
    {                                                                                              \
        struct tlk_uart_config config = {.baudrate       = CONFIG_TLK_UART##num##_BAUDRATE,        \
                                         .parity         = CONFIG_TLK_UART##num##_PARITY,          \
                                         .stop_bit       = CONFIG_TLK_UART##num##_STOP_BIT,        \
                                         .rx_timeout_mul = TLK_UART_RX_TIMEOUT_MUL_2,              \
                                         .tx_port_pin =                                            \
                                             (struct tlk_gpio_port_pin){                           \
                                                 .port = PINMUX_UART##num##_TX_PORT,               \
                                                 .pin  = PINMUX_UART##num##_TX_PIN,                \
                                             },                                                    \
                                         .rx_port_pin = (struct tlk_gpio_port_pin){                \
                                             .port = PINMUX_UART##num##_RX_PORT,                   \
                                             .pin  = PINMUX_UART##num##_RX_PIN,                    \
                                         }};                                                       \
        tlk_uart_configure(TLK_UART##num, &config);                                                \
                                                                                                   \
        TLK_IF_ENABLED(                                                                            \
            CONFIG_TLK_UART##num##_FLOW_CONTROL,                                                   \
            (struct tlk_uart_flow_control_config flow_control_config =                             \
                 {.flow_control  = CONFIG_TLK_UART##num##_FLOW_CONTROL_TYPE,                       \
                  .flow_polarity = CONFIG_TLK_UART##num##_FLOW_POLARITY,                           \
                  .rts_port_pin  = TLK_IF_ENABLED_ELSE(CONFIG_TLK_UART##num##_FLOW_CONTROL_RTS_ON, \
                                                       ((struct tlk_gpio_port_pin){                \
                                                           .port = PINMUX_UART##num##_RTS_PORT,    \
                                                           .pin  = PINMUX_UART##num##_RTS_PIN,     \
                                                       }),                                         \
                                                       ((struct tlk_gpio_port_pin){})),            \
                  .cts_port_pin  = TLK_IF_ENABLED_ELSE(CONFIG_TLK_UART##num##_FLOW_CONTROL_CTS_ON, \
                                                       ((struct tlk_gpio_port_pin){                \
                                                           .port = PINMUX_UART##num##_CTS_PORT,    \
                                                           .pin  = PINMUX_UART##num##_CTS_PIN,     \
                                                       }),                                         \
                                                       ((struct tlk_gpio_port_pin){}))};           \
             tlk_uart_flow_control_configure(TLK_UART##num, &flow_control_config);))               \
    }                                                                                              \
    TLK_REGISTER_PRE_INIT(tlk_uart##num##_auto_config, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_LOW)

#define TLK_UART_DECLARE_AUTO_CONFIG_IF_ENABLED(num)                                               \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_AUTO_CONFIG, (TLK_UART_DECLARE_AUTO_CONFIG(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_AUTO_CONFIG_IF_ENABLED)
#endif
/* ==== AUTO CONFIG ==== */
