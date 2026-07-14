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
#include "common/include/tlk_utils.h"
#include "core/include/tlk_gpio.h"

#include "system/debug/log/tlk_log.h"
#include "tlk_board_pinout.h"

#include "api/include/tlk_time.h"

TLK_LOG_CREATE(demo_log, "LOG_DEMO");

#if TLK_IS_DISABLED(CONFIG_TLK_TASK_PLANNER)

static uint32_t last = 0;
int             main(void)
{
    while (1)
    {
        tlk_log_iface_loop();
        tlk_logrotate();
        uint32_t now = tlk_api_micros();
        if (now - last >= 1000000)
        {
            last = now;
            tlk_log(demo_log, TLK_LOG_LEVEL_ERROR, "Test log USB");
        }
    }
}

#elif TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_LOOP)
void user_setup(void) {}

void user_loop(void)
{
    static uint32_t last = 0;
    uint32_t        now  = tlk_api_micros();
    if (now - last >= 1000000)
    {
        last = now;
        tlk_log(demo_log, TLK_LOG_LEVEL_ERROR, "Test log USB from Telink Loop");
    }
}

#elif TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER)
void user_setup(void) {}

void user_loop(void)
{
    tlk_log(demo_log, TLK_LOG_LEVEL_ERROR, "Test log USB with Telink Scheduler");
}
#endif
