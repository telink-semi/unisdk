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
#include "core/include/tlk_gpio.h"
#include "common/include/tlk_loop.h"
#include "common/include/tlk_utils.h"
#include "api/include/tlk_time.h"
#include "system/task_planner/task_planner.h"
#include "tlk_board_pinout.h"

#if IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)
/**
 * @brief Software timer callback that toggles LED1.
 *
 * This function is invoked by the software timer subsystem
 * when the associated timer expires.
 *
 * It toggles the configured LED1 GPIO pin each time it is called.
 *
 * @param arg Optional user argument passed by the timer system.
 *            Unused in this implementation.
 *
 * @note
 * - Executed in software timer context (not interrupt context,
 *   unless the timer subsystem runs inside an ISR).
 * - Must execute quickly to avoid delaying other timers.
 * - Should not perform blocking operations.
 *
 * @par Behavior
 * - Toggle LED1 state.
 * - No internal rescheduling logic required (handled by timer mode).
 */
void sw_timer_loop(void *arg)
{
    (void)arg;
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN);
}
/**
 * @brief Statically registered periodic software timer instance.
 *
 * Registers `test_timer1` as a periodic software timer using
 * the linker-based timer registry mechanism.
 *
 * Configuration:
 * - Callback: sw_timer_loop
 * - Mode: SW_TIMER_MODE_PERIODIC
 * - Period: 1000 ms (converted to timer cycles via TASK_MS)
 * - Initial state: inactive
 *
 * The timer must be started explicitly using:
 * @code
 * tlk_sw_timer_start(&test_timer1);
 * @endcode
 *
 * @note
 * - Requires SOFTWARE_TIMERS to be enabled.
 * - Registration relies on ".tasks_array.timers" linker section.
 * - Execution timing accuracy depends on scheduler resolution.
 */
TLK_SW_TIMER_REGISTER(test_timer1, sw_timer_loop, SW_TIMER_MODE_PERIODIC, TASK_MS(1000));
#endif

/**
 * @brief User application initialization entry point.
 *
 * This function is called once during system startup,
 * after low-level hardware initialization and before
 * the main execution loop begins.
 *
 * It is intended for application-specific setup such as:
 * - Peripheral configuration
 * - GPIO initialization
 * - Timer setup
 * - Memory allocation
 *
 * @note
 * - Executed exactly once.
 * - Runs in normal thread context (not interrupt context).
 * - Must return; blocking indefinitely will prevent the main loop from running.
 *
 * @warning
 * - Avoid long blocking delays.
 * - Do not depend on components that are initialized later
 *   in the platform startup sequence.
 *
 * @par Execution Model
 * Called during system initialization before `user_loop()` begins
 * repeated execution.
 */
void user_setup(void)
{
    tlk_gpio_configure(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_configure(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_configure(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN, TLK_GPIO_OUTPUT);

    #if IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)
    tlk_sw_timer_start(&test_timer1);
    #endif
}

#if IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_LOOP)

/**
 * @brief User application main execution loop.
 *
 * This function is called repeatedly by the platform loop
 * dispatcher, similar to Arduino's `loop()` function.
 *
 * It represents the primary cooperative execution context
 * for the application.
 *
 * Typical responsibilities:
 * - Polling peripherals
 * - Updating state machines
 * - Processing events
 * - Non-blocking logic execution
 *
 * @note
 * - Executed continuously inside the platform main loop.
 * - Must return promptly to allow other loop handlers
 *   or background processing to execute.
 *
 * @warning
 * - Do not implement infinite loops inside this function.
 * - Avoid long blocking delays.
 * - Ensure execution time is bounded to preserve system responsiveness.
 *
 * @par Execution Model
 * Internally called from:
 * @code
 *
 * void tlk_platform_loop(void) {
 *   while(1)
 *   {
 *       tlk_loop();
 *   }
 * }
 * @endcode
 */
void user_loop(void) 
{
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN);
    tlk_api_delay(TLK_USEC(1));
}

void loop1(void)
{
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
    tlk_api_delay(TLK_USEC(2));
}

TLK_REGISTER_LOOP(loop1)
#endif

#if IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER)
/**
 * @brief Periodic user application execution hook.
 *
 * This function is invoked by the default task when
 * CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK is enabled.
 *
 * Execution occurs periodically with an interval defined by
 * CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK_LOOP_PERIOD_MS.
 *
 * The default task internally performs:
 * @code
 * user_loop();
 * TASK_DELAY(t, TASK_MS(CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK_LOOP_PERIOD_MS));
 * @endcode
 *
 * @note
 * - Executed in task context (not interrupt context).
 * - Execution period depends on scheduler accuracy.
 * - Must return promptly to preserve periodic timing.
 *
 * @warning
 * - Avoid long blocking delays.
 * - Execution time should be less than the configured period.
 * - If execution exceeds the period, scheduling drift may occur.
 *
 * @par Typical Usage
 * - Poll peripherals
 * - Run state machines
 * - Process queued events
 * - Perform periodic housekeeping
 *
 * @par Configuration Dependency
 * Requires:
 * - CONFIG_TLK_TASK_PLANNER=y
 * - CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER=y
 * - CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK=y
 */
void user_loop(void) 
{
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN);
}

/**
 * @brief Sample periodic task that toggles LED1.
 *
 * This task toggles the configured LED1 GPIO pin and then
 * reschedules itself to run again after 500 milliseconds.
 *
 * The delay is implemented using:
 * @code
 * TASK_DELAY(t, TASK_MS(500));
 * @endcode
 *
 * which updates the task wake timestamp in the scheduler.
 *
 * @param t Pointer to the task control structure (::tTask).
 *
 * @note
 * - Executed in task context (not interrupt context).
 * - Uses cooperative scheduling.
 * - The effective period depends on scheduler timing accuracy.
 *
 * @warning
 * - Must call TASK_DELAY() or TASK_YIELD() to prevent continuous execution.
 * - Execution time should be significantly shorter than the delay interval.
 *
 * @par Behavior
 * - Toggles LED1 state.
 * - Sleeps for 500 ms.
 * - Repeats indefinitely.
 *
 * @par Example Usage
 * @code
 * TASK_SPAWN(loop1_task, loop1);
 * @endcode
 */
void loop1(tTask *t)
{
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
    TASK_DELAY(t, TASK_MS(2000));
}

TASK_SPAWN(test_task1, loop1);

#endif
