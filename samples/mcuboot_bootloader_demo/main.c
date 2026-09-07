#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_uart.h"
#include "tlk_mcuboot.h"

#include <stdbool.h>
#include <stdint.h>

#define RECOVERY_UART_TX_PORT TLK_GPIO_PORT_C
#define RECOVERY_UART_TX_PIN  TLK_GPIO_PIN_4
#define RECOVERY_UART_RX_PORT TLK_GPIO_PORT_C
#define RECOVERY_UART_RX_PIN  TLK_GPIO_PIN_5

#define DFU_BUTTON_DEBOUNCE_MS 30

static void configure_uart(enum tlk_uart_module uart, uint32_t baudrate,
                           enum tlk_uart_rx_timeout_mul rx_timeout_mul)
{
    struct tlk_uart_config config = {
        .baudrate       = baudrate,
        .parity         = TLK_UART_PARITY_NONE,
        .stop_bit       = TLK_UART_STOP_BIT_ONE,
        .rx_timeout_mul = rx_timeout_mul,
        .tx_port_pin =
            (struct tlk_gpio_port_pin){
                .port = RECOVERY_UART_TX_PORT,
                .pin  = RECOVERY_UART_TX_PIN,
            },
        .rx_port_pin =
            (struct tlk_gpio_port_pin){
                .port = RECOVERY_UART_RX_PORT,
                .pin  = RECOVERY_UART_RX_PIN,
            },
    };

    tlk_uart_configure(uart, &config);
}

bool tlk_mcuboot_recovery_requested(void)
{
    tlk_gpio_configure(PINMUX_KEY_2_PORT, PINMUX_KEY_2_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(PINMUX_KEY_2_PORT, PINMUX_KEY_2_PIN, 1);

    tlk_gpio_configure(PINMUX_KEY_0_PORT, PINMUX_KEY_0_PIN, TLK_GPIO_INPUT_PULL_DOWN);

    tlk_api_time_delay(TLK_MS_TO_US(1));

    if (!tlk_gpio_pin_read(PINMUX_KEY_0_PORT, PINMUX_KEY_0_PIN))
    {
        return false;
    }

    tlk_api_time_delay(TLK_MS_TO_US(DFU_BUTTON_DEBOUNCE_MS));

    return tlk_gpio_pin_read(PINMUX_KEY_0_PORT, PINMUX_KEY_0_PIN) != 0;
}

void tlk_mcuboot_prepare_recovery(void)
{
    tlk_gpio_configure(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN, 1);

    configure_uart(
        (enum tlk_uart_module) CONFIG_TLK_MCUBOOT_SERIAL_UART, 115200, TLK_UART_RX_TIMEOUT_MUL_4);
}

int main(void)
{
    tlk_core_interrupt_enable();

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_UART)
    configure_uart(TLK_UART0, CONFIG_TLK_UART0_BAUDRATE, TLK_UART_RX_TIMEOUT_MUL_2);
#endif

    tlk_mcuboot_run();

    while (1)
    {
    }
}
