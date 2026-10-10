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

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_GPIO_INPUT)
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_pm.h"
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_LED)
#include "common/include/tlk_init.h"
#include "core/include/tlk_gpio.h"

bool led0_state;
bool led1_state;

void flip_led(void)
{
    led0_state = !led0_state;
    led1_state = !led1_state;
}

TLK_REGISTER_BEFORE_SUSPEND(flip_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_BEFORE_SLEEP(flip_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)

void init_led(void)
{
    tlk_gpio_configure(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN, led0_state);

    tlk_gpio_configure(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN, led1_state);
}

TLK_REGISTER_AFTER_SUSPEND(init_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_AFTER_SLEEP(init_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)

TLK_REGISTER_BEFORE_SUSPEND(init_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_BEFORE_SLEEP(init_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)

#endif

int main(void)
{

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_LED)

    led0_state = true;
    led1_state = false;

    init_led();

#endif

    while (1)
    {
        tlk_api_time_delay(TLK_SEC_TO_US(1));

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_GPIO_INPUT)
        tlk_pm_set_gpio_wakeup(TLK_GPIO_PORT_C, TLK_GPIO_PIN_4, TLK_PM_GPIO_WAKEUP_LEVEL_HIGH);
        tlk_gpio_configure(TLK_GPIO_PORT_C, TLK_GPIO_PIN_4, TLK_GPIO_INPUT_PULL_DOWN);
#endif

        /*
         * If GPIO is enabled, it will not enter deep sleep mode; instead, it will enter a suspend
         * state.
         */
#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_DISABLE_LED_BEFORE_SLEEP)
        tlk_gpio_pin_write(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN, 0);
        tlk_gpio_pin_write(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN, 0);
#endif

        tlk_api_sleep(TLK_SEC_TO_MS(1));
    }
}
