#include "core/cpp_wrappers/tlk_uart.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_sys.h"
#include "tlk_ble_controller_hci.h"
#include "tlk_ble_controller_initial.h"
#include <string.h>
#ifdef __cplusplus
}
#endif

#define UART_NUM        TLK_UART1
#define UART_TIMEOUT    TLK_SEC_TO_US(5)
#define HCI_BUFFER_SIZE 256

UARTDriver uart(UART_NUM);

uint8_t                 hci_rx_buffer[HCI_BUFFER_SIZE];
uint8_t                 hci_tx_buffer[HCI_BUFFER_SIZE];
static volatile uint8_t tx_done  = 0;
static volatile uint8_t rx_count = 0;

void tx_handler(void)
{
    tx_done = 1;
}

void rx_handler(uint32_t count)
{
    rx_count = count;
}

void uart_send_bytes(const uint8_t* data, uint32_t len, uint32_t timeout_us)
{
    uart.sendBytes(data, len, tx_handler, timeout_us);
    while (!tx_done)
    {
    }
    tx_done = 0;
}

static int app_ble_controller_hci_send_data(uint32_t h, uint8_t* para, uint32_t n)
{
    (void) h;
    if (n != 0)
    {
        // Send HCI event to UART
        uart_send_bytes(para, n, UART_TIMEOUT);
    }

    return 0;
}

int main(void)
{
    struct tlk_uart_config config = {.baudrate = 115200,
                                     .parity   = TLK_UART_PARITY_NONE,
                                     .stop_bit = TLK_UART_STOP_BIT_ONE,
                                     .tx_port_pin =
                                         (struct tlk_gpio_port_pin) {
                                             .port = TLK_GPIO_PORT_B,
                                             .pin  = TLK_GPIO_PIN_3,
                                         },
                                     .rx_port_pin = (struct tlk_gpio_port_pin) {
                                         .port = TLK_GPIO_PORT_B,
                                         .pin  = TLK_GPIO_PIN_2,
                                     }};
    uart.configure(&config);

    // Initialize BLE controller
    tlk_ble_controller_initial();
    tlk_ble_controller_hci_registerEventHandler(app_ble_controller_hci_send_data);

    tlk_core_interrupt_enable();
    // Main loop
    while (1)
    {
        // Receive HCI command from UART
        enum tlk_uart_status receive_status =
            uart.receiveBytes(hci_rx_buffer, HCI_BUFFER_SIZE, rx_handler);

        if (receive_status == TLK_UART_OK && rx_count > 0)
        {
            tlk_ble_controller_hci_handler(hci_rx_buffer, rx_count);
            rx_count = 0;
        }
    }

    return 0;
}
