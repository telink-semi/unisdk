/********************************************************************************************************
 * @file    main.cpp
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
#include "core/cpp_wrappers/tlk_uart_cpp.h"
#include "core/include/tlk_core.h"
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_DEMO_SLEEP)
#include "api/include/tlk_sleep.h"
#endif

#ifdef __cplusplus
}
#endif

#define BUFFER_SIZE 20

UARTDriver uart(TLK_UART1);

unsigned char                data[BUFFER_SIZE];
static volatile bool         rx_done  = false;
static volatile unsigned int rx_count = 0;

void print(const char* text, unsigned int timeout_us)
{
    uart.send_bytes((const unsigned char*) text, strlen(text), timeout_us);
}

void rx_handler(unsigned int count)
{
    rx_done  = true;
    rx_count = count;
}

int main(void)
{
#if TLK_IS_DISABLED(CONFIG_TLK_UART_USED_AUTO_CONFIG)
    uart.configure(115200,
                   TLK_UART_PARITY_NONE,
                   TLK_UART_STOP_BIT_ONE,
                   TLK_UART_FLOW_RTS_CTS,
                   TLK_UART_ACTIVE_LOW);
    uart.configure_pinmux(
        (struct tlk_gpio_port_pin) {
            .port = TLK_GPIO_PORT_C,
            .pin  = TLK_GPIO_PIN_4,
        },
        (struct tlk_gpio_port_pin) {
            .port = TLK_GPIO_PORT_C,
            .pin  = TLK_GPIO_PIN_5,
        },
        (struct tlk_gpio_port_pin) {
            .port = TLK_GPIO_PORT_A,
            .pin  = TLK_GPIO_PIN_1,
        },
        (struct tlk_gpio_port_pin) {
            .port = TLK_GPIO_PORT_A,
            .pin  = TLK_GPIO_PIN_2,
        });
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_DEMO_SLEEP)
    print("Before sleep\n", TLK_SEC_TO_US(5));
    api_sleep(TLK_SEC_TO_MS(2));

    print("After sleep\n", TLK_SEC_TO_US(5));
#endif

    tlk_core_interrupt_enable();

    while (1)
    {
        memset(data, 0, BUFFER_SIZE);
        print("Start listening: ", TLK_SEC_TO_US(5));

        enum tlk_uart_status recieve_status =
            uart.receive_bytes(data, BUFFER_SIZE, rx_handler, TLK_SEC_TO_US(5));
        switch (recieve_status)
        {
        case TLK_UART_OK:
            print("OK\n", TLK_SEC_TO_US(5));
            break;
        case TLK_UART_TIMEOUT:
            print("TIMEOUT\n", TLK_SEC_TO_US(5));
            continue;
        case TLK_UART_ERROR:
            print("ERR\n", TLK_SEC_TO_US(5));
            continue;
        }

        while (!rx_done)
            ;

        print("Recieved: ", TLK_SEC_TO_US(5));
        uart.send_bytes(data, rx_count, TLK_SEC_TO_US(5));
        print("\n", TLK_SEC_TO_US(5));

        rx_done  = false;
        rx_count = 0;
    }
}
