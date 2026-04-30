/********************************************************************************************************
 * @file    main.c
 *
 * @brief   This is the source file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2019
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#include "core/include/tlk_dma.h"
#include "tlk_core.h"
#include "common/include/tlk_utils.h"
#include "api/include/tlk_time.h"
#include "system/debug/log/tlk_log.h"
#include <string.h>

#if IS_ENABLED(CONFIG_TLK_DMA_DEMO_SLEEP)
    #include "api/include/tlk_sleep.h"
#endif

TLK_LOG_CREATE(dma_demo, "DMA_DEMO");

#if IS_ENABLED(CONFIG_TLK_DMA_DEMO_PREVENT)
    #include "core/include/tlk_pm.h"
#endif

#if CONFIG_TLK_MEMORY_STACK_SIZE < 2
    #error This DMA demo requires a minimum stack size of 2KB!
#endif

/* ==== DEFINES ==== */
#define ARRAY_LEN 300
#define TRANSFER_SIZE 32 // must be less than ARRAY_LEN
/* ==== DEFINES ==== */


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
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0,
};

enum tlk_dma_chn demo_chn;
uint8_t src[ARRAY_LEN];
uint8_t dst[ARRAY_LEN];
/* ==== VARIABLES ==== */


/* ==== HELPERS ==== */
void print_array(const char* label, uint8_t* array, uint32_t size) {
    TLK_LOG_INFO(dma_demo, "%s", label);

    for (uint32_t i = 0; i < size; i++) {
        TLK_LOG_INFO(dma_demo, "%d ", array[i]);
    }
}

#if IS_ENABLED(CONFIG_TLK_DMA_DEMO_INTERRUPT)
volatile uint8_t dma_complete = 0;
volatile uint8_t dma_abort = 0;

void chn_handler(enum tlk_dma_irq_type irq_type) {
    switch (irq_type) {
        case TLK_DMA_IRQ_TC: dma_complete = 1; break;
        case TLK_DMA_IRQ_ABT: dma_abort = 1; break;
        default: break;
    }
}
#endif
/* ==== HELPERS ==== */


int main(void)
{
    tlk_core_interrupt_enable();

    demo_chn = tlk_dma_request_chn();
    tlk_dma_configure(demo_chn, &chn_config);

#if IS_ENABLED(CONFIG_TLK_DMA_DEMO_INTERRUPT)
    tlk_dma_add_callback(demo_chn, chn_handler);
#endif

    while (1) {
    #if IS_ENABLED(CONFIG_TLK_DMA_DEMO_PREVENT)
        tlk_dma_start_transfer(demo_chn, (uint32_t) src, (uint32_t) dst, ARRAY_LEN, TLK_DMA_BYTE_WIDTH);
        enum tlk_pm_sleep_status sleep_status = tlk_pm_sleep(TLK_PM_SLEEP_MODE_SUSPEND, 10);

        if (sleep_status == TLK_PM_SLEEP_DENIED) {
            TLK_LOG_INFO(dma_demo, "The sleep was prevented due to active receiving\n");
        }
    #endif

        for (uint8_t i = 0;i < TRANSFER_SIZE;i++) {
            src[i] = i + 1;
        }
        memset(dst, 0, TRANSFER_SIZE);

        print_array("Initial src: ", src, TRANSFER_SIZE);
        print_array("Initial dst: ", dst, TRANSFER_SIZE);

        // TODO: /n

        tlk_dma_start_transfer(demo_chn, (uint32_t) src, (uint32_t) dst, TRANSFER_SIZE, TLK_DMA_BYTE_WIDTH);
        tlk_dma_abort(demo_chn);

    #if IS_ENABLED(CONFIG_TLK_DMA_DEMO_INTERRUPT)
        while (!dma_abort) ;
        dma_abort = 0;
        TLK_LOG_INFO(dma_demo, "DMA IRQ: abort");
#endif
        
        print_array("src after aborting transfer: ", src, TRANSFER_SIZE);
        print_array("dst after aborting transfer: ", dst, TRANSFER_SIZE);

        // TODO: /n
        
        memset(dst, 0, TRANSFER_SIZE);
        tlk_dma_start_transfer(demo_chn, (uint32_t) src, (uint32_t) dst, TRANSFER_SIZE, TLK_DMA_BYTE_WIDTH);

    #if IS_ENABLED(CONFIG_TLK_DMA_DEMO_INTERRUPT)
        while (!dma_complete) ;
        dma_complete = 0;
        TLK_LOG_INFO(dma_demo,"DMA IRQ: complete");
#endif
        
        print_array("src after transfer: ", src, TRANSFER_SIZE);
        print_array("dst after transfer: ", dst, TRANSFER_SIZE);

        // TODO: /n/n

    #if IS_ENABLED(CONFIG_TLK_DMA_DEMO_SLEEP)
        tlk_api_sleep(TLK_MSEC(5));
    #else
        tlk_api_delay(TLK_USEC(5));
    #endif
    }
}