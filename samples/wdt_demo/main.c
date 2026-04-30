#include "core/include/tlk_watchdog.h"
#include "common/include/tlk_utils.h"
#include "api/include/tlk_time.h"
#include "tlk_board_pinout.h"

#if IS_ENABLED(CONFIG_TLK_WDT_SAMPLE_PM)
    #include "api/include/tlk_sleep.h"
#endif

#if IS_ENABLED(CONFIG_TLK_WDT_SAMPLE_GPIO_INDICATION)
    #include "core/include/tlk_gpio.h"

void init_led(void) {
    tlk_gpio_configure(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, 0);

    tlk_gpio_configure(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, 0);

    tlk_gpio_configure(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN, 0);
}
#endif


int main(void) {

#if IS_ENABLED(CONFIG_TLK_WDT_SAMPLE_GPIO_INDICATION)
    init_led();
    tlk_gpio_pin_write(UNISDK_BOARD_LED_0_PORT, UNISDK_BOARD_LED_0_PIN, 1);
#endif

/*
 * Running a 32k WDT for 5 sec. Expect passing the WDT window with blocking delay
 */
    tlk_wdt_32k_start(5000);
    tlk_api_delay(TLK_USEC(2));
    tlk_wdt_32k_stop();

/*
 * Switch on the LED if passed the WDT window with blocking delay
 */
#if IS_ENABLED(CONFIG_TLK_WDT_SAMPLE_GPIO_INDICATION)
    tlk_gpio_pin_write(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, 1);   
#endif

/*
 * Running a 32k WDT for 7 sec. Expect passing the WDT window with low power mode, if enabled
 */
    tlk_wdt_32k_start(7000);
/* 
 * This section enables Sleep Mode with previously configured WDT.
 * If LED2 is not lighting, this means this sample is dead or WDT configuration is wrong or PM driver hangs. 
 * We have to pass this section and run the next code 
 */    
 #if IS_ENABLED(CONFIG_TLK_WDT_SAMPLE_PM)  
    tlk_api_sleep(TLK_MSEC(5));
#else
    tlk_api_delay(TLK_USEC(5));
#endif

    tlk_wdt_32k_stop();
 
/*
 * Switch on the LED if passed the WDT window with low power mode, if enabled
 */
#if IS_ENABLED(CONFIG_TLK_WDT_SAMPLE_GPIO_INDICATION)
    tlk_gpio_pin_write(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN, 1);   
#endif

/*
 * Perform the WDT fire in 5 sec
 */
    tlk_wdt_32k_start(5000);
    
    while (1) {

    }
}