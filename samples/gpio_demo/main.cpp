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
#include "core/cpp_wrappers/tlk_gpio_cpp.h"
#include "core/include/tlk_core.h"
#include "tlk_board_pinout.h"
#include "tlk_stimer.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "common/include/tlk_utils.h"

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PM_DEVICE)
#include "api/include/tlk_time.h"
#endif

#ifdef __cplusplus
}
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_SAMPLE_PLIC)
GPIODriver GPIO_Port_Led_1(UNISDK_BOARD_LED_1_PORT);

void gpio_cb(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    (void) port;
    (void) pin;
    GPIO_Port_Led_1.toggle(UNISDK_BOARD_LED_1_PIN);
}
#endif

int main(void)
{
    GPIODriver GPIO_Port_Led_0(UNISDK_BOARD_LED_0_PORT);
    GPIO_Port_Led_0.configure(UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);

    GPIODriver GPIO_Port_Key_3(UNISDK_BOARD_KEY_3_PORT);
    GPIODriver GPIO_Port_Key_0(UNISDK_BOARD_KEY_0_PORT);

    GPIO_Port_Key_3.configure(UNISDK_BOARD_KEY_3_PIN, TLK_GPIO_OUTPUT);
    GPIO_Port_Key_0.configure(UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INPUT_PULL_DOWN);

    GPIO_Port_Key_3.write(UNISDK_BOARD_KEY_3_PIN, 1);

    GPIO_Port_Led_0.toggle(UNISDK_BOARD_LED_0_PIN);
    tlk_sys_delay(TLK_SEC_TO_US(1));
    GPIO_Port_Led_0.toggle(UNISDK_BOARD_LED_0_PIN);

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_SAMPLE_PLIC)
    GPIO_Port_Led_1.setPort(UNISDK_BOARD_LED_1_PORT);
    GPIO_Port_Led_1.configure(UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);

    GPIODriver GPIO_Port_Key_1(UNISDK_BOARD_KEY_1_PORT);
    GPIO_Port_Key_1.configure(UNISDK_BOARD_KEY_1_PIN, TLK_GPIO_INPUT_PULL_DOWN);

    struct tlk_gpio_irq_callback callback = {
        .node = {}, .handler = gpio_cb, .pin = UNISDK_BOARD_KEY_1_PIN};

    GPIO_Port_Key_1.irq_add_callback(&callback);
    GPIO_Port_Key_1.irq_configure(UNISDK_BOARD_KEY_1_PIN, TLK_GPIO_INTR_RISING_EDGE);

    tlk_core_interrupt_enable();
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_SAMPLE_PM)
    GPIODriver GPIO_Port_Led_3(UNISDK_BOARD_LED_3_PORT);
    GPIO_Port_Led_3.configure(UNISDK_BOARD_LED_3_PIN, TLK_GPIO_OUTPUT);
    GPIO_Port_Led_3.write(UNISDK_BOARD_LED_3_PIN, true);

    unsigned int last_us = tlk_api_micros();
#endif

    while (1)
    {
        bool key0_state = GPIO_Port_Key_0.read(UNISDK_BOARD_KEY_0_PIN);
        GPIO_Port_Led_0.write(UNISDK_BOARD_LED_0_PIN, key0_state);

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_SAMPLE_PM)
        if (tlk_api_micros() - last_us >= TLK_SEC_TO_US(4))
        {
            GPIO_Port_Led_3.write(UNISDK_BOARD_LED_3_PIN, false);
            tlk_api_sleep(TLK_SEC_TO_MS(4));

            last_us = tlk_api_micros();
            GPIO_Port_Led_3.write(UNISDK_BOARD_LED_3_PIN, true);
        }
#endif
    }
}
