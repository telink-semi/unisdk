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

#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_random.h"
#include "system/debug/log/tlk_log.h"

#ifdef CONFIG_TLK_RANDOM_DEMO_SLEEP
#include "api/include/tlk_sleep.h"
#endif

TLK_LOG_CREATE(random_demo, "RANDOM_DEMO");

int main(void)
{
    while (1)
    {
        TLK_LOG_INFO(random_demo, "Random numbers:\n");

        for (uint8_t i = 0; i < CONFIG_TLK_RANDOM_DEMO_NUMBERS_COUNT; i++)
        {
            TLK_LOG_INFO(random_demo,
                         "\t%u: %u\n",
                         (i + 1),
                         tlk_random() % (CONFIG_TLK_RANDOM_DEMO_MAX_NUMBER + 1));
        }

        TLK_LOG_INFO(random_demo, "\n");

#ifndef CONFIG_TLK_RANDOM_DEMO_SLEEP
        tlk_api_delay(TLK_SEC_TO_US(4));
#else
        tlk_api_sleep(TLK_SEC_TO_MS(4));
#endif
    }
}
