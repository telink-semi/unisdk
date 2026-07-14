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
#include "tlk_board_pinout.h"

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_DEMO_INTERRUPT)

void gpio_cb(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    (void) port;
    (void) pin;
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
}
#endif

int main(void)
{
    tlk_gpio_configure(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_configure(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_DEMO_INTERRUPT)

    // Applying HIGH to KEY_2 pin to detect a logic '1' on the KEY_0, when pressed.
    tlk_gpio_configure(UNISDK_BOARD_KEY_2_PORT, UNISDK_BOARD_KEY_2_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(UNISDK_BOARD_KEY_2_PORT, UNISDK_BOARD_KEY_2_PIN, 1);

    tlk_gpio_configure(UNISDK_BOARD_KEY_0_PORT, UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INPUT_PULL_DOWN);

    struct tlk_gpio_irq_callback callback = {.handler = gpio_cb, .pin = UNISDK_BOARD_KEY_0_PIN};

    tlk_gpio_irq_add_callback(UNISDK_BOARD_KEY_0_PORT, &callback);
    tlk_gpio_irq_configure(
        UNISDK_BOARD_KEY_0_PORT, UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INTR_RISING_EDGE);

    tlk_core_interrupt_enable();
#endif

    while (1)
    {
        tlk_gpio_pin_toggle(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN);
        tlk_api_delay(TLK_SEC_TO_US(2));
        tlk_api_sleep(TLK_SEC_TO_MS(2));
    }
}
