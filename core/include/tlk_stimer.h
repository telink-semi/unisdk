/********************************************************************************************************
 * @file    stimer.h
 *
 * @brief   This is the header file for B92
 *
 * @author  Driver Group
 * @date    2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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

#ifndef TLK_STIMER_H_
#define TLK_STIMER_H_

#include "properties/tlk_stimer.h"
#include "registers/tlk_stimer.h"
#include <stdbool.h>

/**********************************************************************************************************************
 *                                         global data type                   *
 *********************************************************************************************************************/
/**
 * @brief define system clock tick per us/ms/s.
 */
enum tlk_system_timer_tick
{
    TLK_SYSTEM_TIMER_TICK_1US = UNISDK_STIMER_CLOCK / 1000000,
    TLK_SYSTEM_TIMER_TICK_1MS = UNISDK_STIMER_CLOCK / 1000,
    TLK_SYSTEM_TIMER_TICK_1S  = UNISDK_STIMER_CLOCK,

    TLK_SYSTEM_TIMER_TICK_625US  = 625 * TLK_SYSTEM_TIMER_TICK_1US,  // 625*24
    TLK_SYSTEM_TIMER_TICK_1250US = 1250 * TLK_SYSTEM_TIMER_TICK_1US, // 1250*24
};

/**********************************************************************************************************************
 *                                      global function prototype                   *
 *********************************************************************************************************************/

/**
 * @brief       This function servers to enable system timer.
 * @return      none.
 */
void tlk_stimer_enable(void);

/**
 * @brief       This function servers to disable system timer.
 * @return      none.
 */
void tlk_stimer_disable(void);

/**
 * @brief       This function servers to set system timer irq mask.
 * @param[in]   mask - the irq mask.
 * @return      none.
 */
void tlk_stimer_irq_set_mask(tlk_stimer_irq_mask_e mask);

/**
 * @brief       This function servers to clear system timer irq mask.
 * @param[in]   mask - the irq mask.
 * @return      none.
 */
void tlk_stimer_irq_clr_mask(tlk_stimer_irq_mask_e mask);

/**
 * @brief       This function servers to clear system timer irq status.
 * @param[in]   status - the irq status.
 * @return      none.
 */
void tlk_stimer_irq_clr_status(tlk_stimer_irq_status_e status);

/**
 * @brief       This function servers to set tick irq capture.
 * @param[in]   tick - the value of irq tick.
 * @return      none.
 */
void tlk_stimer_irq_set_capture(uint32_t tick);

/**
 * @brief       This function is used to set the tick value of the system timer.
 * @param[in]   tick - the value of system timer tick.
 * @return      none.
 */
void tlk_stimer_set_tick(uint32_t tick);

/**
 * @brief     This function performs to set stimer auto count mode.
 * @param[in] enable - allow auto count mode
 * @return    none.
 */
void tlk_stimer_set_auto_mode(bool enable);

/**
 * @brief     This function performs to set stimer write enable for 32k mode.
 * @param[in] enable - 32k write mode
 * @return    none.
 */
void tlk_stimer_set_32k_write(bool enable);

/**
 * @brief       This function servers to get system timer irq status.
 * @param[in]   status - the irq status.
 * @retval      non-zero -  the interrupt occurred.
 * @retval      zero  -  the interrupt did not occur.
 */
tlk_stimer_irq_status_e tlk_stimer_irq_get_status(tlk_stimer_irq_status_e status);

/**
 * @brief     This function performs to get system timer tick.
 * @return    system timer tick value.
 */
uint32_t tlk_stimer_get_tick(void);

/**
 * @brief     This function performs to set the main stimer clock tracking the shadow 32k timer
 * tick.
 * @param[in] enable - allow 32k tracking
 * @return    none.
 */
void tlk_stimer_set_32k_clock_tracking(bool enable);

/**
 * @brief       This function servers to get system timer status register state.
 * @retval      tlk_system_timer_state_e  -  the current stimer status.
 */
tlk_stimer_state_e tlk_stimer_get_state(void);

/**
 * @brief     This function performs to set the main stimer clock tracking the shadow 32k timer
 * tick.
 * @param[in] state - set\clear stimer state register
 * @return    none.
 */
void tlk_stimer_set_state(tlk_stimer_state_e state);

void tlk_stimer_register_callback(void (*cb)(void));

#endif /* TLK_STIMER_H_ */
