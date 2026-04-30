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

#include "core/include/tlk_adc.h"
#include "core/include/tlk_dma.h"
#include "core/include/tlk_sys.h"
#include "tlk_core.h"
#include "common/include/tlk_utils.h"
#include "api/include/tlk_time.h"
#include "system/debug/log/tlk_log.h"

#if IS_ENABLED(CONFIG_TLK_ADC_DEMO_PM)
    #include "api/include/tlk_sleep.h"
#endif

#define ADC_BUFFER_SIZE 8

TLK_LOG_CREATE(adc_demo, "ADC_DEMO");

uint16_t adc_values[ADC_BUFFER_SIZE] __attribute__((aligned(4)));

int main(void)
{   
    struct tlk_adc_config cfg = {
        .channel = TLK_ADC_CHANNEL_0,
        .prescale = TLK_ADC_PRESCALE_1F4,
        .resolution = TLK_ADC_RESOLUTION_12,
        .sample_freq = TLK_ADC_SAMPLE_FREQ_96K,
        .vref = TLK_ADC_VREF_1P2V, 
        .sampling_port_pin = {
            .port = TLK_GPIO_PORT_B,
            .pin = TLK_GPIO_PIN_0,
        },
        .is_differential = false,
        .differential_port_pin = {
            .port = TLK_GPIO_PORT_D,
            .pin = TLK_GPIO_PIN_0,
        },
        .timeout_us = 10000,
#if IS_ENABLED(CONFIG_TLK_DMA)
        .dma_en = false,
        .dma_channel = tlk_dma_request_chn(),
#endif
    };

    tlk_adc_configure(&cfg);

    tlk_core_interrupt_enable();

    while (1) {
        enum tlk_adc_status status = tlk_adc_read(adc_values, ADC_BUFFER_SIZE);

        if (status == TLK_ADC_OK) {
            for (uint32_t i = 0; i < ADC_BUFFER_SIZE; i++) {
                TLK_LOG_INFO(adc_demo, "%u", adc_values[i]);
            }
        } else {
            TLK_LOG_ERROR(adc_demo, "Timeout");
        }

#if IS_ENABLED(CONFIG_TLK_ADC_DEMO_PM)
        tlk_api_sleep(TLK_MSEC(1));
#else
        tlk_api_delay(TLK_USEC(1));
#endif
    }
    return 0;
}
