/********************************************************************************************************
 * @file    tlk_compiler.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
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
#ifndef TLK_COMPILER_H_
#define TLK_COMPILER_H_

#define _tlk_attribute_ram_code_sec_ __attribute__((section(".ram_code")))
#define _tlk_attribute_ram_code_sec_noinline_                                                      \
    __attribute__((section(".ram_code"))) __attribute__((noinline))

#define _tlk_attribute_text_sec_noinline_                                                          \
    __attribute__((section(".text"))) __attribute__((noinline))

/**
 *  _tlk_always_inline needs to be added in the following two cases:
 * 1. The subfunctions in the pm_sleep_wakeup function need to use _tlk_always_inline and
 * _tlk_attribute_ram_code_sec_noinline_, as detailed in the internal comments of pm_sleep_wakeup.
 * 2. The BLE SDK uses interrupt preemption, flash interface operations can be interrupted by
 * high-priority interrupts, which requires that the high-priority interrupt handler function,can
 * not have a text segment of code. So the BLE SDK provides the following requirements: Add the
 * following function to _tlk_always_inline: rf_set_tx_rx_off, rf_set_ble_crc_adv,
 * rf_set_ble_crc_value, rf_set_rx_maxlen, tlk_stimer_get_tick, tlk_clock_time_exceed,
 * rf_receiving_flag, tlk_dma_config, gpio_toggle, tlk_gpio_set_low_level, gpio_set_level.
 */
#define _tlk_always_inline inline __attribute__((always_inline))

#define _tlk_attribute_data_retention_sec_ __attribute__((section(".retention_data")))

#define _tlk_attribute_aes_data_sec_ __attribute__((section(".aes_data")))

#endif
