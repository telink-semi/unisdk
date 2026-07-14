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
#include "api/include/tlk_sleep.h"
#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_plic.h"
#include "core/include/tlk_stimer.h"
#include "properties/tlk_plic.h"
#include "tlk_board_pinout.h"

void timer_cb(void);

int main(void)
{

    tlk_gpio_configure(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_configure(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);

    tlk_stimer_enable();
    tlk_stimer_set_tick(0);
    tlk_stimer_set_auto_mode(true);
    tlk_stimer_register_callback(timer_cb);

    tlk_stimer_set_irq_capture(tlk_stimer_get_tick() + TLK_SYSTEM_TIMER_TICK_1MS * 500);
    tlk_stimer_set_irq_mask(TLK_STIMER_IRQ_MASK_TIMER_IRQ_EN);
    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_STIMER);

    tlk_core_interrupt_enable();

    while (1)
    {
        tlk_gpio_pin_toggle(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN);
        tlk_api_sleep(TLK_SEC_TO_MS(1));
    }
}

void timer_cb(void)
{
    tlk_stimer_set_irq_capture(tlk_stimer_get_tick() + TLK_SYSTEM_TIMER_TICK_1MS * 500);
    tlk_stimer_clr_irq_status(TLK_STIMER_IRQ_STATUS_TIMER_IRQ);
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
}
