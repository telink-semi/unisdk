#include "include/uart_boot_serial.h"

#include "boot_serial/boot_serial.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_uart.h"

#include <stdbool.h>
#include <stdint.h>

#define BOOT_SERIAL_UART ((enum tlk_uart_module) CONFIG_TLK_MCUBOOT_SERIAL_UART)

#define BOOT_SERIAL_TX_TIMEOUT_US TLK_SEC_TO_US(1)
#define BOOT_SERIAL_RX_BUF_SIZE   256

static uint8_t tlk_uart_rx_buf[BOOT_SERIAL_RX_BUF_SIZE];

static volatile uint32_t tlk_uart_rx_count;
static uint32_t          tlk_uart_rx_pos;

static volatile bool tlk_uart_tx_done;

static void tlk_uart_rx_handler(uint32_t count)
{
    tlk_uart_rx_count = count;
}

static void tlk_uart_rx_arm(void)
{
    tlk_uart_rx_count = 0;
    tlk_uart_rx_pos   = 0;

    (void) tlk_uart_receive_bytes(
        BOOT_SERIAL_UART, tlk_uart_rx_buf, sizeof(tlk_uart_rx_buf), tlk_uart_rx_handler);
}

static void tlk_uart_tx_handler(void)
{
    tlk_uart_tx_done = true;
}

static int tlk_boot_uart_read(char* str, int cnt, int* newline)
{
    int off = 0;

    *newline = 0;

    if (tlk_uart_rx_count == 0)
    {
        return 0;
    }

    while ((off < cnt) && (tlk_uart_rx_pos < tlk_uart_rx_count))
    {
        uint8_t byte = tlk_uart_rx_buf[tlk_uart_rx_pos++];

        if (byte == '\n')
        {
            str[off++] = '\0';
            *newline   = 1;
            break;
        }

        str[off++] = (char) byte;
    }

    if (tlk_uart_rx_pos >= tlk_uart_rx_count)
    {
        tlk_uart_rx_arm();
    }

    return off;
}

static void tlk_boot_uart_write(const char* ptr, int cnt)
{
    tlk_uart_tx_done = false;

    tlk_uart_send_bytes(BOOT_SERIAL_UART,
                        (const uint8_t*) ptr,
                        (uint32_t) cnt,
                        tlk_uart_tx_handler,
                        BOOT_SERIAL_TX_TIMEOUT_US);

    while (!tlk_uart_tx_done)
    {
    }
}

static const struct boot_uart_funcs tlk_uart_funcs = {
    .read  = tlk_boot_uart_read,
    .write = tlk_boot_uart_write,
};

const struct boot_uart_funcs* tlk_mcuboot_serial_uart_init(void)
{
    tlk_uart_rx_count = 0;
    tlk_uart_rx_pos   = 0;
    tlk_uart_tx_done  = false;

    tlk_uart_rx_arm();

    return &tlk_uart_funcs;
}
