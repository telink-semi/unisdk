#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_sys.h"
#include "core/include/tlk_watchdog.h"
#include "system/debug/log/tlk_log.h"
#include "tlk_board_pinout.h"

#if TLK_IS_ENABLED(CONFIG_TLK_WDT_DEMO_GPIO_INDICATION)
#include "core/include/tlk_gpio.h"
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_WDT_DEMO_PM)
#include "api/include/tlk_sleep.h"
#endif

TLK_LOG_CREATE(wdt_demo, "WDT_DEMO");

int main(void)
{
    tlk_core_interrupt_enable();

    enum tlk_sys_reboot_reason reboot_reason = tlk_sys_get_reboot_reason();

    if (reboot_reason == TLK_SYS_REBOOT_REASON_WDT)
    {
        TLK_LOG_INFO(wdt_demo, "Reboot by a watchdog.");
    }
    else
    {
        TLK_LOG_INFO(wdt_demo, "Reboot by a non-watchdog source.");
    }

#if TLK_IS_ENABLED(CONFIG_TLK_WDT_DEMO_GPIO_INDICATION)
    tlk_gpio_configure(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, 1);

    tlk_gpio_configure(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, 0);
#endif

    tlk_api_delay(TLK_SEC_TO_US(2));

    // Run the WDT, expecting passing the WDT window with blocking delay or sleep and lighting LED1.
#if TLK_IS_ENABLED(CONFIG_TLK_WDT_DEMO_STARTUP)
    tlk_wdt_feed();
#else
    tlk_wdt_start(TLK_SEC_TO_MS(5));
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_WDT_DEMO_PM)
    tlk_api_sleep(TLK_SEC_TO_MS(2));
#else
    tlk_api_delay(TLK_SEC_TO_US(2));
#endif

    // Switch on the LED1 if passed the WDT window with blocking delay or sleep.
#if TLK_IS_ENABLED(CONFIG_TLK_WDT_DEMO_GPIO_INDICATION)
    tlk_gpio_pin_write(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, 1);
#endif

    // Feed the WDT and wait until it fires.
    tlk_wdt_feed();

    while (1)
    {
    }
}
