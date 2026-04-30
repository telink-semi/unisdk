#include "core/include/tlk_uart.h"
#include "common/include/tlk_init.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static bool tlk_critical_print_hw_inited = false;

extern void tlk_uart_configure_critical(enum tlk_uart_module uart_num, uint32_t baudrate, struct tlk_gpio_port_pin tx_port_pin);
extern void tlk_uart_send_critical(enum tlk_uart_module uart_num, const char * buf, uint32_t buff_size);

static void tlk_critical_print_hw_init(void)
{
    if (!tlk_critical_print_hw_inited)
    {
        tlk_critical_print_hw_inited = true;
        tlk_uart_configure_critical(CONFIG_TLK_CRITICAL_UART, CONFIG_TLK_CRITICAL_UART_BAUDRATE, 
            (struct tlk_gpio_port_pin) {  .port = PINMUX_CRITICAL_PRINT_TX_PORT, .pin = PINMUX_CRITICAL_PRINT_TX_PIN, });
    }
}

void _tlk_critical_print(const char *buf) {
    if (buf == NULL) return;

    if ( !tlk_critical_print_hw_inited ) {
        tlk_critical_print_hw_init();
    }
    uint32_t len = strlen(buf);
    if (len)
    tlk_uart_send_critical(CONFIG_TLK_CRITICAL_UART, buf, len);
}


int _tlk_critical_printf(const char *fmt, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    (void) vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    _tlk_critical_print(buf);
    return sizeof(buf);
}

void tlk_critical_print_pm_reinit(void)
{
    tlk_critical_print_hw_inited = false;
}

TLK_REGISTER_AFTER_SLEEP(tlk_critical_print_pm_reinit, TLK_INIT_LEVEL_SYSTEM, TLK_INIT_PRIORITY_HIGH);
// TODO: Check with disabled flash, move necessary APIs to RAM
