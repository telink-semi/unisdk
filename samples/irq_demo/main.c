#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_mtimer.h"
#include "core/include/tlk_plic.h"
#include "core/include/tlk_plic_sw.h"
#include "core/include/tlk_uart.h"
#include "properties/tlk_plic.h"
#include <stdio.h>

// This demo uses the basic UART driver APIs, instead of the Logger to ensure correct interrupts
// operation.

#define TLK_DEBUG_PRINT_BUFFER_SIZE 1000
#define TLK_SAMPLE_PLIC_SRC_LOW     12
#define TLK_SAMPLE_PLIC_SRC_HIGH    13

static char             tlk_print_buffer[TLK_DEBUG_PRINT_BUFFER_SIZE];
static volatile uint8_t tlk_tx_done      = 0;
static volatile uint8_t tlk_log_depth    = 0;
static const char*      tlk_log_indent[] = {"", "  ", "    ", "      ", "        "};

/* --- Print Helpers --- */
static void tlk_tx_handler(void)
{
    tlk_tx_done = 1;
}

void tlk_print(const char* __restrict format, ...)
{
    va_list args;
    va_start(args, format);
    int size = vsnprintf(tlk_print_buffer, TLK_DEBUG_PRINT_BUFFER_SIZE, format, args);
    va_end(args);

    if (size > 0)
    {
        tlk_uart_send_bytes(
            TLK_UART0, (uint8_t*) tlk_print_buffer, size, tlk_tx_handler, TLK_SEC_TO_US(5));
        while (!tlk_tx_done)
        {
        }
        tlk_tx_done = 0;
    }
}

#define tlk_sample_log(fmt, ...)                                                                   \
    tlk_print("%s" fmt "\n", tlk_log_indent[tlk_log_depth], ##__VA_ARGS__)

static void tlk_sample_log_mie(void)
{
    uint32_t mie = tlk_read_csr(NDS_MIE);

    tlk_sample_log("MIE  msie=%u mtie=%u meie=%u",
                   (unsigned) ((mie & TLK_FLD_MIE_MSIE) != 0),
                   (unsigned) ((mie & TLK_FLD_MIE_MTIE) != 0),
                   (unsigned) ((mie & TLK_FLD_MIE_MEIE) != 0));
}
/* --- Print Helpers --- */

/* --- Interrupt handlers --- */
static void tlk_sample_msi_handler(void)
{
    tlk_log_depth++;

    tlk_print("\n");
    tlk_sample_log("Enter PLIC_SW (MSI)");
    tlk_sample_log_mie();

#if TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC_SW)

    tlk_print("\n");
    tlk_sample_log("Pending MTIMER (MTI)");
    tlk_mtimer_set_mtime_compare(tlk_mtimer_get_mtime());

    tlk_print("\n");
    tlk_sample_log("Pending PLIC low (MEI)");
    tlk_plic_set_pending(TLK_SAMPLE_PLIC_SRC_LOW);

    tlk_print("\n");
    tlk_sample_log("Pending PLIC high (MEI)");
    tlk_plic_set_pending(TLK_SAMPLE_PLIC_SRC_HIGH);

#endif

    tlk_sample_log("Leave PLIC_SW (MSI)");
    tlk_log_depth--;
}

static void tlk_sample_mti_handler(void)
{
    tlk_log_depth++;

    tlk_print("\n");
    tlk_sample_log("Enter MTIMER (MTI)");
    tlk_sample_log_mie();

#if TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_IRQ_BASE_MTIMER)

    tlk_print("\n");
    tlk_sample_log("Pending PLIC_SW (MSI)");
    tlk_plic_sw_set_pending();

    tlk_print("\n");
    tlk_sample_log("Pending PLIC low (MEI)");
    tlk_plic_set_pending(TLK_SAMPLE_PLIC_SRC_LOW);

    tlk_print("\n");
    tlk_sample_log("Pending PLIC high (MEI)");
    tlk_plic_set_pending(TLK_SAMPLE_PLIC_SRC_HIGH);

#endif

    tlk_mtimer_set_mtime_compare(0xFFFFFFFFFFFFFFFFULL);

    tlk_sample_log("Leave MTIMER (MTI)");
    tlk_log_depth--;
}

static void tlk_sample_mei_low_isr(void)
{
    tlk_log_depth++;

    tlk_print("\n");
    tlk_sample_log("Enter PLIC low (MEI)");
    tlk_sample_log_mie();

#if TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC)

    static volatile uint8_t processed = 0;

    if (processed)
    {
        tlk_sample_log("Exit. The handler has already been processed.");
    }
    else
    {
        processed = 1;

        tlk_print("\n");
        tlk_sample_log("Pending PLIC_SW (MSI)");
        tlk_plic_sw_set_pending();

        tlk_print("\n");
        tlk_sample_log("Pending MTIMER (MTI)");
        tlk_mtimer_set_mtime_compare(tlk_mtimer_get_mtime());

        tlk_print("\n");
        tlk_sample_log("Pending PLIC high (MEI)");
        tlk_plic_set_pending(TLK_SAMPLE_PLIC_SRC_HIGH);

        // delay to set the pending for high-priority IRQ
        tlk_api_time_delay(100);
    }

#endif

    tlk_sample_log("Leave PLIC low (MEI)");
    tlk_log_depth--;
}

static void tlk_sample_mei_high_isr(void)
{
    tlk_log_depth++;

    tlk_print("\n");
    tlk_sample_log("Enter PLIC high (MEI)");
    tlk_sample_log_mie();

#if TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC)

    static volatile uint8_t processed = 0;

    if (processed)
    {
        tlk_sample_log("Exit. The handler has already been processed.");
    }
    else
    {
        processed = 1;

        tlk_print("\n");
        tlk_sample_log("Pending PLIC_SW (MSI)");
        tlk_plic_sw_set_pending();

        tlk_print("\n");
        tlk_sample_log("Pending MTIMER (MTI)");
        tlk_mtimer_set_mtime_compare(tlk_mtimer_get_mtime());

        tlk_print("\n");
        tlk_sample_log("Pending PLIC low (MEI)");
        tlk_plic_set_pending(TLK_SAMPLE_PLIC_SRC_LOW);

        // delay to set the pending for low-priority IRQ
        tlk_api_time_delay(100);
    }

#endif

    tlk_sample_log("Leave PLIC high (MEI)");
    tlk_log_depth--;
}

TLK_PLIC_ISR_REGISTER(tlk_sample_mei_low_isr, TLK_SAMPLE_PLIC_SRC_LOW)
TLK_PLIC_ISR_REGISTER(tlk_sample_mei_high_isr, TLK_SAMPLE_PLIC_SRC_HIGH)

int main(void)
{
    // MSI Configuration
    tlk_plic_sw_interrupt_enable();
    tlk_plic_sw_register_callback(tlk_sample_msi_handler);

    // MTI Configuration
    tlk_mtimer_irq_register_callback(tlk_sample_mti_handler);

    // MEI Configuration
    tlk_plic_set_priority(TLK_SAMPLE_PLIC_SRC_LOW, TLK_IRQ_PRI_LEV1);
    tlk_plic_set_priority(TLK_SAMPLE_PLIC_SRC_HIGH, TLK_IRQ_PRI_LEV2);
    tlk_plic_set_priority(UNISDK_PLIC_IRQ_NUM_UART0, TLK_IRQ_PRI_LEV3); // TODO: move to UART

    tlk_plic_interrupt_enable(TLK_SAMPLE_PLIC_SRC_LOW);
    tlk_plic_interrupt_enable(TLK_SAMPLE_PLIC_SRC_HIGH);

#if TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC) &&                                           \
    TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_PLIC_PREEMPT)
    tlk_plic_enable_preempt();
#endif

    // M-mode interrupts configuration
    tlk_core_mie_enable(TLK_FLD_MIE_MSIE);
    tlk_core_mie_enable(TLK_FLD_MIE_MTIE);
    tlk_core_mie_enable(TLK_FLD_MIE_MEIE);
    tlk_core_interrupt_enable();

    // Demo starting
    tlk_sample_log("IRQ nesting sample started");

#if TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC_SW)

    tlk_sample_log("\nMAIN: Panding PLIC_SW (MSI)");
    tlk_plic_sw_set_pending();

#elif TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_IRQ_BASE_MTIMER)

    tlk_sample_log("\nMAIN: Panding MTIMER (MTI)");
    tlk_mtimer_set_mtime_compare(tlk_mtimer_get_mtime());

#elif TLK_IS_ENABLED(CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC)

    tlk_sample_log("\nMAIN: Panding PLIC low (MEI)");
    tlk_plic_set_pending(TLK_SAMPLE_PLIC_SRC_LOW);

#endif
}
