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

#include "core/cpp_wrappers/tlk_uart.hpp"
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#if TLK_IS_ENABLED(CONFIG_TLK_UART_DEMO_SLEEP)
#include "api/include/tlk_sleep.h"
#endif

#ifdef __cplusplus
}
#endif

#define BUFFER_SIZE 20

UARTDriver uart(CONFIG_TLK_UART_DEMO_UART);

unsigned char                data[BUFFER_SIZE];
static volatile bool         rx_done  = false;
static volatile unsigned int rx_count = 0;

static volatile bool tx_done = false;

void tx_handler(void)
{
    tx_done = true;
}

void print(const char* text, unsigned int timeout_us)
{
    uart.sendBytes((const unsigned char*) text, strlen(text), tx_handler, timeout_us);
    while (!tx_done)
        ;
    tx_done = false;
}

void rx_handler(uint32_t count)
{
    rx_done  = true;
    rx_count = count;
}

int main(void)
{
#if TLK_IS_DISABLED(CONFIG_TLK_UART_USED_AUTO_CONFIG)
    struct tlk_uart_config config = {
        .baudrate       = 115200,
        .parity         = TLK_UART_PARITY_NONE,
        .stop_bit       = TLK_UART_STOP_BIT_ONE,
        .rx_timeout_mul = TLK_UART_RX_TIMEOUT_MUL_2,
        .tx_port_pin =
            (struct tlk_gpio_port_pin) {
                .port = TLK_GPIO_PORT_C,
                .pin  = TLK_GPIO_PIN_4,
            },
        .rx_port_pin =
            (struct tlk_gpio_port_pin) {
                .port = TLK_GPIO_PORT_C,
                .pin  = TLK_GPIO_PIN_5,
            },
    };
    uart.configure(&config);
    uart.configurePinmux(&config);

#if TLK_IS_ENABLED(CONFIG_TLK_UART1_FLOW_CONTROL)
    struct tlk_uart_flow_control_config flow_control_config = {
        .flow_control  = TLK_UART_FLOW_RTS_CTS,
        .flow_polarity = TLK_UART_ACTIVE_LOW,
        .rts_port_pin =
            (struct tlk_gpio_port_pin) {
                .port = TLK_GPIO_PORT_A,
                .pin  = TLK_GPIO_PIN_1,
            },
        .cts_port_pin =
            (struct tlk_gpio_port_pin) {
                .port = TLK_GPIO_PORT_A,
                .pin  = TLK_GPIO_PIN_2,
            },
    };
    uart.configureFlowControl(&flow_control_config);
    uart.configureFlowControlPinmux(&flow_control_config);
#endif
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

        enum tlk_uart_status recieve_status = uart.receiveBytes(data, BUFFER_SIZE, rx_handler);
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
        uart.sendBytes(data, rx_count, tx_handler, TLK_SEC_TO_US(5));
        while (!tx_done)
            ;
        tx_done = false;
        print("\n", TLK_SEC_TO_US(5));

        rx_done  = false;
        rx_count = 0;
    }
}
