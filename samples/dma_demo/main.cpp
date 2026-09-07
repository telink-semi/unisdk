#include "core/cpp_wrappers/tlk_dma.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "system/debug/log/tlk_log.h"
#include <string.h>

#if TLK_IS_ENABLED(CONFIG_TLK_DMA_DEMO_SLEEP)
#include "api/include/tlk_sleep.h"
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DMA_DEMO_PREVENT)
#include "core/include/tlk_pm.h"
#endif
#ifdef __cplusplus
}
#endif

#if CONFIG_TLK_MEMORY_STACK_SIZE < 2
#error This DMA demo requires a minimum stack size of 2KB!
#endif

/* ==== DEFINES ==== */
#define ARRAY_LEN     300
#define TRANSFER_SIZE 32 // must be less than ARRAY_LEN
/* ==== DEFINES ==== */

TLK_LOG_CREATE(dma_demo, "DMA_DEMO");

/* ==== VARIABLES ==== */
struct tlk_dma_config chn_config = {
    .dst_req_sel    = 0,
    .src_req_sel    = 0,
    .dst_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
    .src_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
    .dstmode        = TLK_DMA_NORMAL_MODE,
    .srcmode        = TLK_DMA_NORMAL_MODE,
    .dstwidth       = TLK_DMA_BYTE_WIDTH,
    .srcwidth       = TLK_DMA_BYTE_WIDTH,
    .src_burst_size = TLK_DMA_BURST_1_TRANSFER,
    .vacant_bit     = 0,
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0,
};

uint8_t src[ARRAY_LEN];
uint8_t dst[ARRAY_LEN];
/* ==== VARIABLES ==== */

/* ==== HELPERS ==== */
void print_array(const char* label, uint8_t* array, uint32_t size)
{
    TLK_LOG_INFO(dma_demo, "%s", label);
}

#if TLK_IS_ENABLED(CONFIG_TLK_DMA_DEMO_INTERRUPT)
volatile uint8_t dma_complete = 0;
volatile uint8_t dma_abort    = 0;

void chn_handler(enum tlk_dma_irq_type irq_type)
{
    switch (irq_type)
    {
    case TLK_DMA_IRQ_TC:
        dma_complete = 1;
        break;
    case TLK_DMA_IRQ_ABT:
        dma_abort = 1;
        break;
    default:
        break;
    }
}
#endif
/* ==== HELPERS ==== */

int main(void)
{
    tlk_core_interrupt_enable();

    DmaDriver dma = DmaDriver::request();
    dma.configure(&chn_config);

#if TLK_IS_ENABLED(CONFIG_TLK_DMA_DEMO_INTERRUPT)
    dma.addCallback(chn_handler);
#endif

    while (1)
    {
#if TLK_IS_ENABLED(CONFIG_TLK_DMA_DEMO_PREVENT)
        dma.startTransfer((uint32_t) src, (uint32_t) dst, ARRAY_LEN, TLK_DMA_BYTE_WIDTH);
        enum tlk_pm_sleep_status sleep_status = tlk_pm_sleep(TLK_PM_SLEEP_MODE_SUSPEND, 10);

        if (sleep_status == TLK_PM_SLEEP_DENIED)
        {
            TLK_LOG_INFO(dma_demo, "The sleep was prevented due to active receiving\n");
        }
#endif

        for (uint8_t i = 0; i < TRANSFER_SIZE; i++)
        {
            src[i] = i + 1;
        }
        memset(dst, 0, TRANSFER_SIZE);

        print_array("Initial src: ", src, TRANSFER_SIZE);
        print_array("Initial dst: ", dst, TRANSFER_SIZE);

        dma.startTransfer((uint32_t) src, (uint32_t) dst, TRANSFER_SIZE, TLK_DMA_BYTE_WIDTH);
        dma.abort();

#if TLK_IS_ENABLED(CONFIG_TLK_DMA_DEMO_INTERRUPT)
        while (!dma_abort)
            ;
        dma_abort = 0;
        TLK_LOG_INFO(dma_demo, "DMA IRQ: abort");
#endif

        print_array("src after aborting transfer: ", src, TRANSFER_SIZE);
        print_array("dst after aborting transfer: ", dst, TRANSFER_SIZE);

        memset(dst, 0, TRANSFER_SIZE);
        dma.startTransfer((uint32_t) src, (uint32_t) dst, TRANSFER_SIZE, TLK_DMA_BYTE_WIDTH);

#if TLK_IS_ENABLED(CONFIG_TLK_DMA_DEMO_INTERRUPT)
        while (!dma_complete)
            ;
        dma_complete = 0;
        TLK_LOG_INFO(dma_demo, "DMA IRQ: complete");
#endif

        print_array("src after transfer: ", src, TRANSFER_SIZE);
        print_array("dst after transfer: ", dst, TRANSFER_SIZE);

#if TLK_IS_ENABLED(CONFIG_TLK_DMA_DEMO_SLEEP)
        tlk_api_sleep(TLK_SEC_TO_MS(5));
#else
        tlk_api_delay(TLK_SEC_TO_US(5));
#endif
    }
}
