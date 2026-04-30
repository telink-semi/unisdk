/********************************************************************************************************
 * @file    stimer.c
 *
 * @brief   This is the source file for TL721X
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
#include "core/include/tlk_stimer.h"
#include "registers/tlk_stimer.h"

enum tlk_system_timer_control_e
{
    FLD_SYSTEM_32K_WR_EN    = TLK_BIT(0),
    FLD_SYSTEM_TIMER_EN     = TLK_BIT(1),
    FLD_SYSTEM_TIMER_AUTO   = TLK_BIT(2),
    FLD_SYSTEM_32K_TRACK_EN = TLK_BIT(3),
    FLD_SYSTEM_32K_CAL_MODE = TLK_BIT_RNG(4, 7),
};

/**
 * @brief       This function servers to enable system timer.
 * @return      none.
 */
_tlk_attribute_ram_code_sec_ void tlk_stimer_enable(void)
{
    tlk_stimer_reg.control_bit.timer_en = 1;
}

/**
 * @brief       This function servers to disable system timer.
 * @return      none.
 */
_tlk_attribute_ram_code_sec_ void tlk_stimer_disable(void)
{
    tlk_stimer_reg.control_bit.timer_en = 0;
}

/**
 * @brief     This function performs to set stimer auto count mode.
 * @param[in] enable - allow auto count mode
 * @return    none.
 */
_tlk_attribute_ram_code_sec_ void tlk_stimer_set_auto_mode(bool enable)
{
    tlk_stimer_reg.control_bit.timer_auto = enable;
}

/**
 * @brief     This function performs to set the main stimer clock tracking the shadow 32k timer tick.
 * @param[in] enable - allow 32k tracking
 * @return    none.
 */
_tlk_attribute_ram_code_sec_ void tlk_stimer_set_32k_clock_tracking(bool enable)
{
    tlk_stimer_reg.control_bit.cal_32k_en = enable;
}

/**
 * @brief     This function performs to set stimer write enable for 32k mode.
 * @param[in] enable - 32k write mode
 * @return    none.
 */
_tlk_attribute_ram_code_sec_ void tlk_stimer_set_32k_write(bool enable)
{
    tlk_stimer_reg.control_bit.write_32k = enable;
}

/**
 * @brief       This function servers to get system timer status register state.
 * @retval      tlk_system_timer_state_e  -  the current stimer status.
 */
_tlk_attribute_ram_code_sec_ enum tlk_system_timer_state_e tlk_stimer_get_state(void)
{
    return tlk_stimer_reg.state;
}

/**
 * @brief     This function performs to set the main stimer clock tracking the shadow 32k timer tick.
 * @param[in] state - set\clear stimer state register
 * @return    none.
 */
_tlk_attribute_ram_code_sec_ void tlk_stimer_set_state(enum tlk_system_timer_state_e state)
{
    tlk_stimer_reg.state = state;
}

/**
 * @brief This function servers to set stimer irq mask.
 * @param[in]   mask - the irq mask.
 * @return      none.
 */
void tlk_stimer_set_irq_mask(tlk_stimer_irq_e mask)
{
    tlk_stimer_reg.irq_mask |= mask;
}

/**
 * @brief This function servers to clear stimer irq mask.
 * @param[in]   mask - the irq mask.
 * @return      none.
 */
void tlk_stimer_clr_irq_mask(tlk_stimer_irq_e mask)
{
    tlk_stimer_reg.irq_mask &= (~mask);
}

/**
 * @brief This function servers to clear stimer irq status.
 * @param[in]   status - the irq status.
 * @return      none.
 */
void tlk_stimer_clr_irq_status(tlk_stimer_irq_e status)
{
    tlk_stimer_reg.irq_status = status;
}

/**
 * @brief       This function servers to get stimer irq status.
 * @param[in]   status    - the irq status.
 * @retval      non-zero      - the interrupt occurred.
 * @retval      zero  - the interrupt did not occur.
 */
tlk_stimer_irq_e tlk_stimer_get_irq_status(tlk_stimer_irq_e status)
{
    return tlk_stimer_reg.irq_status & status;
}

/**
 * @brief This function servers to set tick irq capture.
 * @param[in] tick - the value of irq tick.
 * @return    none.
 */
void tlk_stimer_set_irq_capture(uint32_t tick)
{
    tlk_stimer_reg.irq_time = tick;
}

/**
 * @brief This function servers to set stimer tick.
 * @param[in] tick - the value of tick.
 * @return    none.
 */
_tlk_attribute_ram_code_sec_ void tlk_stimer_set_tick(uint32_t tick)
{
    tlk_stimer_reg.current_time = tick;
}

/*
 * @brief     This function performs to get system timer tick.
 * @return    system timer tick value.
**/
_tlk_attribute_ram_code_sec_ uint32_t tlk_stimer_get_tick(void)
{
    return tlk_stimer_reg.current_time;
}
