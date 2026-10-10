#include "core/cpp_wrappers/tlk_pmp.hpp"
#include "core/cpp_wrappers/tlk_uart.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
#include "api/include/tlk_sleep.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_trap.h"
#include "registers/tlk_uart.h"
#include <string.h>
#ifdef __cplusplus
}
#endif

#define MEMORY_SIZE        0x100000000
#define UART_START_ADDRESS (void*) TLK_UART0_ADDR
#define UART_MEMORY_SIZE   ((uint32_t) TLK_UART1_ADDR - (uint32_t) TLK_UART0_ADDR)

typedef enum
{
    USER_REQUEST_NONE  = 0,
    USER_REQUEST_SLEEP = 1,
} user_request_t;

static volatile user_request_t user_request = USER_REQUEST_NONE;
static volatile uint8_t        tx_done      = 0;

UARTDriver uart0(TLK_UART0);

/* --- Helpers --- */
static inline void user_request_set(user_request_t req)
{
    user_request = req;
}

void tx_handler(void)
{
    tx_done = 1;
}

void uart_print(const char* text, uint32_t timeout_us)
{
    uart0.sendBytes((const uint8_t*) text, strlen(text), tx_handler, timeout_us);
    while (!tx_done)
    {
    }
    tx_done = 0;
}
/* --- Helpers --- */

/* --- Service side --- */
void task_handler(void)
{
    __asm__ volatile("ecall");
}

void ecall_handler(void)
{
    switch (user_request)
    {
    case USER_REQUEST_SLEEP:
        uart_print("Ecall. Sleep request.\n", TLK_SEC_TO_US(1));
        tlk_api_sleep(TLK_SEC_TO_MS(1));

        user_request = USER_REQUEST_NONE;
        break;
    default:
        uart_print("Ecall. No requests.\n", TLK_SEC_TO_US(1));
        break;
    }

#ifdef CONFIG_TLK_UMODE_DEMO_ENABLE_MMODE
    uart_print("Ecall. Turning on M-mode.\n", TLK_SEC_TO_US(1));
    tlk_core_mmode();
#endif
}
/* --- Service side --- */

/* --- User side --- */
void user_task(void)
{
    uart_print("User task.\n", TLK_SEC_TO_US(1));

#ifdef CONFIG_TLK_UMODE_DEMO_ENABLE_SLEEP
    user_request_set(USER_REQUEST_SLEEP);
#endif
}
/* --- User side --- */

int main(void)
{
    tlk_core_interrupt_enable();
    tlk_trap_register_ecall_callback(ecall_handler);

    struct tlk_pmp_config pmp_all_memory_config = {
        .r        = 1,
        .w        = 1,
        .x        = 1,
        .reserved = 0,
        .l        = 0,
    };
    PMPDriver pmp_all(TLK_PMP_ENTRY7, (void*) 0x0);
    pmp_all.napotConfig(MEMORY_SIZE, &pmp_all_memory_config);

#ifndef CONFIG_TLK_UMODE_DEMO_ENABLE_UART
    struct tlk_pmp_config pmp_uart_config = {
        .r        = 0,
        .w        = 0,
        .x        = 0,
        .reserved = 0,
        .l        = 0,
    };
    PMPDriver pmp_uart(TLK_PMP_ENTRY0, UART_START_ADDRESS);
    pmp_uart.napotConfig(UART_MEMORY_SIZE, &pmp_uart_config);
#endif

    tlk_core_umode(user_task, task_handler);

    /*
     * Privilege check: csrr mstatus succeeds in M-mode and fails with an
     * illegal-instruction exception in U-mode. If CONFIG_TLK_UMODE_SAMPLE_POST_MMODE
     * is not set, ecall_handler never calls tlk_core_mmode(), so execution
     * returns here still in U-mode and the read below traps.
     */
    uart_print("Main. Trying M-mode.\n", TLK_SEC_TO_US(1));

    volatile uint32_t mstatus = tlk_read_csr(NDS_MSTATUS);
    (void) mstatus;

    uart_print("Main. M-mode OK.\n", TLK_SEC_TO_US(1));

    return 0;
}
