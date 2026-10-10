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

#include "core/include/tlk_pmp.h"
#include "system/debug/log/tlk_log.h"

#if defined(CONFIG_TLK_PMP_DEMO_SLEEP)
#include "api/include/tlk_sleep.h"
#endif

TLK_LOG_CREATE(pmp_demo, "PMP_Demo");

static uint8_t demo_array[16]
    __attribute__((aligned(8))) = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

int main(void)
{

#if defined(CONFIG_TLK_PMP_DEMO_TOR) || defined(CONFIG_TLK_PMP_DEMO_NAPOT)
    struct tlk_pmp_config pmp_config = {
        .r = 0,
        .w = 0,
        .x = 0,
        .l = 1,
    };

#if defined(CONFIG_TLK_PMP_DEMO_TOR)
    TLK_LOG_INFO(pmp_demo,
                 "PMP TOR: protected [0x%08x, 0x%08x), length %d bytes",
                 (uint32_t) (demo_array + 8),
                 (uint32_t) (demo_array + 16),
                 8);

    tlk_pmp_disable_entry(TLK_PMP_ENTRY0, (void*) (demo_array + 8));
    tlk_pmp_configure_tor(TLK_PMP_ENTRY1, (void*) (demo_array + 16), &pmp_config);

#elif defined(CONFIG_TLK_PMP_DEMO_NAPOT)
    TLK_LOG_INFO(pmp_demo,
                 "PMP NAPOT: protected [0x%08x, 0x%08x), length %d bytes",
                 (uint32_t) (demo_array + 8),
                 (uint32_t) (demo_array + 8 + 8),
                 8);

    tlk_pmp_configure_napot(TLK_PMP_ENTRY0, (void*) (demo_array + 8), 8, &pmp_config);
#endif

#if defined(CONFIG_TLK_PMP_DEMO_SLEEP)
    TLK_LOG_INFO(pmp_demo, "Before sleep");
    tlk_api_sleep(500);
    TLK_LOG_INFO(pmp_demo, "After sleep");
#endif

#endif

#if defined(CONFIG_TLK_PMP_DEMO_READ)
    for (int i = 0; i < 16; i++)
    {
        TLK_LOG_INFO(pmp_demo, "read array[%d] = %d", i, demo_array[i]);
    }
#elif defined(CONFIG_TLK_PMP_DEMO_WRITE)
    for (int i = 0; i < 16; i++)
    {
        demo_array[i] = (uint8_t) (i + 1);
        TLK_LOG_INFO(pmp_demo, "write array[%d] = %d", i, demo_array[i]);
    }
#endif

    return 0;
}
