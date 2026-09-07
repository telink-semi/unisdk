#include "core/cpp_wrappers/tlk_gpio.hpp"
#include "core/cpp_wrappers/tlk_plic.hpp"
#include "core/cpp_wrappers/tlk_stimer.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
#include "api/include/tlk_sleep.h"
#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "properties/tlk_plic.h"
#include "tlk_board_pinout.h"
#ifdef __cplusplus
}
#endif

void timer_cb(void);

GPIODriver led0(UNISDK_BOARD_LED_0_PORT);
GPIODriver led1(UNISDK_BOARD_LED_1_PORT);

int main(void)
{
    led0.configure(UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
    led1.configure(UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);

    STimerDriver::enable();
    STimerDriver::setTick(0);
    STimerDriver::setAutoMode(true);
    STimerDriver::registerCallback(timer_cb);

    STimerDriver::setIrqCapture(STimerDriver::getTick() + TLK_SYSTEM_TIMER_TICK_1MS * 500);
    STimerDriver::setIrqMask(TLK_STIMER_IRQ_MASK_TIMER_IRQ_EN);
    PlicDriver::enable(UNISDK_PLIC_IRQ_NUM_STIMER);

    tlk_core_interrupt_enable();

    while (1)
    {
        led0.toggle(UNISDK_BOARD_LED_0_PIN);
        tlk_api_sleep(TLK_SEC_TO_MS(1));
    }
}

void timer_cb(void)
{
    STimerDriver::setIrqCapture(STimerDriver::getTick() + TLK_SYSTEM_TIMER_TICK_1MS * 500);
    STimerDriver::clrIrqStatus(TLK_STIMER_IRQ_STATUS_TIMER_IRQ);
    led1.toggle(UNISDK_BOARD_LED_1_PIN);
}
