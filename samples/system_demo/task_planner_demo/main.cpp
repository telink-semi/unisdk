#include "core/cpp_wrappers/tlk_gpio.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
#include "api/include/tlk_time.h"
#include "common/include/tlk_loop.h"
#include "common/include/tlk_utils.h"
#include "system/task_planner/task_planner.h"
#include "tlk_board_pinout.h"
#ifdef __cplusplus
}
#endif

// ============================================================================
// SOFTWARE TIMERS
// ============================================================================
#if TLK_IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)
#ifdef __cplusplus
extern "C"
{
#endif

    void sw_timer_loop(void* arg)
    {
        (void) arg;
        GPIODriver led2(UNISDK_BOARD_LED_2_PORT);
        led2.toggle(UNISDK_BOARD_LED_2_PIN);
    }

    TLK_SW_TIMER_REGISTER(test_timer1, sw_timer_loop, SW_TIMER_MODE_PERIODIC, TASK_MS(1000));

#ifdef __cplusplus
}
#endif
#endif

// ============================================================================
// SYSTEM SETUP
// ============================================================================
#ifdef __cplusplus
extern "C"
{
#endif

    void user_setup(void)
    {
        GPIODriver led0(UNISDK_BOARD_LED_0_PORT);
        GPIODriver led1(UNISDK_BOARD_LED_1_PORT);
        GPIODriver led2(UNISDK_BOARD_LED_2_PORT);

        led0.configure(UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
        led1.configure(UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);
        led2.configure(UNISDK_BOARD_LED_2_PIN, TLK_GPIO_OUTPUT);

#if TLK_IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)
        tlk_sw_timer_start(&test_timer1);
#endif
    }

#ifdef __cplusplus
}
#endif

// ============================================================================
// LOOP MODE
// ============================================================================
#if TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_LOOP)

#ifdef __cplusplus
extern "C"
{
#endif

    void user_loop(void)
    {
        GPIODriver led0(UNISDK_BOARD_LED_0_PORT);
        led0.toggle(UNISDK_BOARD_LED_0_PIN);
        tlk_api_delay(TLK_SEC_TO_US(1));
    }

    void loop1(void)
    {
        GPIODriver led1(UNISDK_BOARD_LED_1_PORT);
        led1.toggle(UNISDK_BOARD_LED_1_PIN);
        tlk_api_delay(TLK_SEC_TO_US(2));
    }

    TLK_REGISTER_LOOP(loop1);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_TLK_TASK_PLANNER_MODE_LOOP

// ============================================================================
// SCHEDULER MODE
// ============================================================================
#if TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER)

#ifdef __cplusplus
extern "C"
{
#endif

    void user_loop(void)
    {
        GPIODriver led0(UNISDK_BOARD_LED_0_PORT);
        led0.toggle(UNISDK_BOARD_LED_0_PIN);
    }

    void loop1(tTask* t)
    {
        GPIODriver led1(UNISDK_BOARD_LED_1_PORT);
        led1.toggle(UNISDK_BOARD_LED_1_PIN);
        TASK_DELAY(t, TASK_MS(2000));
    }

    TASK_SPAWN(test_task1, loop1);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER
