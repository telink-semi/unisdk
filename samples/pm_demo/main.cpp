#include "core/cpp_wrappers/tlk_pm.hpp"

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_GPIO_INPUT) ||                                        \
    TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_LED)
#include "core/cpp_wrappers/tlk_gpio.hpp"
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include "api/include/tlk_sleep.h"
#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_GPIO_INPUT)
#include "tlk_board_pinout.h"
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_LED)
#include "common/include/tlk_init.h"
#include "tlk_board_pinout.h"
#endif
#ifdef __cplusplus
}
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_LED)
bool led0_state;
bool led1_state;

GPIODriver gpio_led0(UNISDK_BOARD_LED_0_PORT);
GPIODriver gpio_led1(UNISDK_BOARD_LED_1_PORT);

void flip_led(void)
{
    led0_state = !led0_state;
    led1_state = !led1_state;
}

TLK_REGISTER_BEFORE_SUSPEND(flip_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_BEFORE_SLEEP(flip_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)

void init_led(void)
{
    gpio_led0.configure(UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
    gpio_led0.write(UNISDK_BOARD_LED_0_PIN, led0_state);

    gpio_led1.configure(UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);
    gpio_led1.write(UNISDK_BOARD_LED_1_PIN, led1_state);
}

TLK_REGISTER_AFTER_SUSPEND(init_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_AFTER_SLEEP(init_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)

TLK_REGISTER_BEFORE_SUSPEND(init_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_BEFORE_SLEEP(init_led, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL)

#endif

int main(void)
{

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_LED)

    led0_state = true;
    led1_state = false;

    init_led();

#endif

    while (1)
    {
        tlk_api_delay(TLK_SEC_TO_US(1));

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_ENABLE_GPIO_INPUT)
        PMDriver::setGpioWakeup(TLK_GPIO_PORT_C, TLK_GPIO_PIN_4, TLK_PM_GPIO_WAKEUP_LEVEL_HIGH);

        GPIODriver gpio_wakeup(TLK_GPIO_PORT_C);
        gpio_wakeup.configure(TLK_GPIO_PIN_4, TLK_GPIO_INPUT_PULL_DOWN);
#endif

        /*
         * If GPIO is enabled, it will not enter deep sleep mode; instead, it will enter a suspend
         * state.
         */
#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEMO_DISABLE_LED_BEFORE_SLEEP)
        gpio_led0.write(UNISDK_BOARD_LED_0_PIN, 0);
        gpio_led1.write(UNISDK_BOARD_LED_1_PIN, 0);
#endif

        tlk_api_sleep(TLK_SEC_TO_MS(1));
    }
}
