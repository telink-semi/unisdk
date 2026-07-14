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
#include "core/include/tlk_core.h"
#include "core/include/tlk_uart.h"
#include <string.h>

#if TLK_IS_ENABLED(CONFIG_TLK_UART_DEMO_SLEEP)
#include "api/include/tlk_sleep.h"
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_DEMO_PREVENT)
#include "core/include/tlk_pm.h"
#endif

#define UART_TIMEOUT TLK_SEC_TO_US(5)

/* ==== VARIABLES ==== */
uint8_t                  data[CONFIG_TLK_UART_DEMO_BUFFER_SIZE];
static volatile uint8_t  tx_done  = 0;
static volatile uint32_t rx_count = 0;
/* ==== VARIABLES ==== */

/* ==== ISR HELPERS ==== */
void tx_handler(void)
{
    tx_done = 1;
}

void rx_handler(uint32_t count)
{
    rx_count = count;
}
/* ==== ISR HELPERS ==== */

/* ==== PRINT HELPERS ==== */
void uart_print(enum tlk_uart_module uart_num, const char* text, uint32_t timeout_us)
{
    tlk_uart_send_bytes(uart_num, (const uint8_t*) text, strlen(text), tx_handler, timeout_us);
    while (!tx_done)
    {
    }
    tx_done = 0;
}
/* ==== PRINT HELPERS ==== */

int main(void)
{
    tlk_core_interrupt_enable();

#if TLK_IS_DISABLED(CONFIG_TLK_UART_DEMO_AUTO_CONFIG)
    struct tlk_uart_config config = {.baudrate       = 115200,
                                     .parity         = TLK_UART_PARITY_NONE,
                                     .stop_bit       = TLK_UART_STOP_BIT_ONE,
                                     .rx_timeout_mul = TLK_UART_RX_TIMEOUT_MUL_2,
                                     .tx_port_pin =
                                         (struct tlk_gpio_port_pin){
                                             .port = TLK_GPIO_PORT_C,
                                             .pin  = TLK_GPIO_PIN_4,
                                         },
                                     .rx_port_pin = (struct tlk_gpio_port_pin){
                                         .port = TLK_GPIO_PORT_C,
                                         .pin  = TLK_GPIO_PIN_5,
                                     }};
    tlk_uart_configure(CONFIG_TLK_UART_DEMO_UART, &config);

    // TODO: change the CONFIG_TLK_UART0_FLOW_CONTROL to CONFIG_TLK_UART_DEMO_FLOW_CONTROL
    TLK_IF_ENABLED(CONFIG_TLK_UART0_FLOW_CONTROL,
                   (struct tlk_uart_flow_control_config flow_control_config =
                        {.flow_control  = TLK_UART_FLOW_RTS_CTS,
                         .flow_polarity = TLK_UART_ACTIVE_LOW,
                         .rts_port_pin =
                             (struct tlk_gpio_port_pin){
                                 .port = TLK_GPIO_PORT_C,
                                 .pin  = TLK_GPIO_PIN_6,
                             },
                         .cts_port_pin =

                             (struct tlk_gpio_port_pin){
                                 .port = TLK_GPIO_PORT_C,
                                 .pin  = TLK_GPIO_PIN_7,
                             }};) tlk_uart_configure_flow_control(CONFIG_TLK_UART_DEMO_UART,
                                                                  &flow_control_config););

    uart_print(CONFIG_TLK_UART_DEMO_UART, "Configured manually\n", UART_TIMEOUT);
#endif

    while (1)
    {

#if TLK_IS_ENABLED(CONFIG_TLK_UART_DEMO_SLEEP)
        uart_print(CONFIG_TLK_UART_DEMO_UART, "Before sleep\n", UART_TIMEOUT);
        tlk_api_sleep(TLK_SEC_TO_MS(2));
        uart_print(CONFIG_TLK_UART_DEMO_UART, "After sleep\n", UART_TIMEOUT);
#endif

        memset(data, 0, CONFIG_TLK_UART_DEMO_BUFFER_SIZE);
        uart_print(CONFIG_TLK_UART_DEMO_UART, "Start listening!\n", UART_TIMEOUT);

        enum tlk_uart_status recieve_status = tlk_uart_receive_bytes(
            CONFIG_TLK_UART_DEMO_UART, data, CONFIG_TLK_UART_DEMO_BUFFER_SIZE, rx_handler);

#if TLK_IS_ENABLED(CONFIG_TLK_UART_DEMO_PREVENT)
        enum tlk_pm_sleep_status sleep_status = tlk_pm_sleep(TLK_PM_SLEEP_MODE_SUSPEND, 10);

        if (sleep_status == TLK_PM_SLEEP_DENIED)
        {
            uart_print(CONFIG_TLK_UART_DEMO_UART,
                       "The sleep was prevented due to active receiving\n",
                       UART_TIMEOUT);
        }
#endif

        switch (recieve_status)
        {
        case TLK_UART_OK:
            while (!rx_count)
            {
            }

            uart_print(CONFIG_TLK_UART_DEMO_UART, "OK\n", UART_TIMEOUT);
            uart_print(CONFIG_TLK_UART_DEMO_UART, "Recieved: ", UART_TIMEOUT);

            tlk_uart_send_bytes(
                CONFIG_TLK_UART_DEMO_UART, data, rx_count, tx_handler, UART_TIMEOUT);

            while (!tx_done)
            {
            }

            tx_done  = 0;
            rx_count = 0;

            uart_print(CONFIG_TLK_UART_DEMO_UART, "\n", UART_TIMEOUT);
            break;

        case TLK_UART_TIMEOUT:
            uart_print(CONFIG_TLK_UART_DEMO_UART, "TIMEOUT\n", UART_TIMEOUT);
            break;

        case TLK_UART_ERROR:
            uart_print(CONFIG_TLK_UART_DEMO_UART, "ERR\n", UART_TIMEOUT);
            break;
        }

        uart_print(CONFIG_TLK_UART_DEMO_UART, "\n", UART_TIMEOUT);
    }
}
