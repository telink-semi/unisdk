#ifndef TLK_INCLUDE_DRIVERS_PM_H_
#define TLK_INCLUDE_DRIVERS_PM_H_

#if IS_DISABLED(CONFIG_TLK_PM_SUSPEND) && IS_DISABLED(CONFIG_TLK_PM_DEEP_SLEEP)
    #error "All sleep modes are disabled. Is there any need to include the PM driver?"
#endif

enum tlk_pm_sleep_mode {
#if IS_ENABLED(CONFIG_TLK_PM_SUSPEND)
    TLK_PM_SLEEP_MODE_SUSPEND,
#endif
#if IS_ENABLED(CONFIG_TLK_PM_DEEP_SLEEP)
    TLK_PM_SLEEP_MODE_DEEP_SLEEP,
#endif
#if IS_ENABLED(CONFIG_TLK_PM_RAM_RETENTION_ENABLE)    
    TLK_PM_SLEEP_MODE_DEEP_RETENTION,
#endif
    TLK_PM_SLEEP_MAX_LEVEL
};

enum tlk_pm_gpio_wakeup_level {
    TLK_PM_GPIO_WAKEUP_LEVEL_LOW  = 0,
    TLK_PM_GPIO_WAKEUP_LEVEL_HIGH = 1,
};

enum tlk_pm_sleep_status {
    TLK_PM_SLEEP_OK,
    TLK_PM_SLEEP_UNSUPPORTED,
    TLK_PM_SLEEP_TOO_SHORT,
    TLK_PM_SLEEP_TOO_LONG,
    TLK_PM_SLEEP_NO_WAKEUP_SOURCES,
    TLK_PM_SLEEP_DENIED,
    TLK_PM_SLEEP_WAKEUP_TRIGGER_PREVENT,
    TLK_PM_SLEEP_SUSPEND_FALLBACK
};

enum tlk_pm_wakeup_source {
    TLK_PM_WAKEUP_SOURCE_NONE = 0,
    TLK_PM_WAKEUP_SOURCE_PAD = TLK_BIT(0),
    TLK_PM_WAKEUP_SOURCE_CORE = TLK_BIT(1),
    TLK_PM_WAKEUP_SOURCE_TIMER = TLK_BIT(2),
    TLK_PM_WAKEUP_SOURCE_COMPARATOR = TLK_BIT(3),
};

/**
 * @brief Returns the wakeup reason.
 *
 * Returns the source that caused the system to wake up from the last sleep cycle.
 *
 * @return enum tlk_pm_wakeup_source A bitmask indicating the wakeup source.
 */
enum tlk_pm_wakeup_source tlk_pm_get_wakeup_reason(void);

/**
 * @brief Clears wakeup source flags.
 *
 * Clears the flags for wakeup sources. This should be called after handling the wakeup event to prepare for the next sleep cycle.
 * 
 * @return None.
 */
void tlk_pm_clear_wakeup_sources(void);

/**
 * @brief Requests the system to enter a specific low-power mode.
 *
 * Requests the system to enter a specific low-power mode for a specific duration.
 *
 * @param mode The desired low-power state.
 * @param duration_us The duration to sleep in milliseconds.
 * 
 * @return enum tlk_pm_sleep_status The result of the operation (e.g., TLK_PM_SLEEP_OK or error code).
 */
enum tlk_pm_sleep_status tlk_pm_sleep(enum tlk_pm_sleep_mode mode, uint32_t duration_ms);

#if IS_ENABLED(CONFIG_TLK_GPIO)

#include "core/include/tlk_gpio.h"

/**
 * @brief Configures a GPIO pin as a wake-up source.
 *
 * Configures a specific GPIO pin as a wake-up source. The system will exit sleep mode when the selected pin detects the configured wake-up polarity (logic level).
 *
 * @param port GPIO port.
 * @param pin GPIO pin.
 * @param polarity The logic level that triggers wakeup (TLK_PM_GPIO_WAKEUP_LEVEL_LOW or HIGH).
 * 
 * @return enum tlk_pm_sleep_status TLK_PM_SLEEP_OK if successful, or TLK_PM_SLEEP_UNSUPPORTED if the pin cannot be used as a wakeup source.
 */
enum tlk_pm_sleep_status tlk_pm_set_gpio_wakeup(enum tlk_gpio_port port, enum tlk_gpio_pin pin, enum tlk_pm_gpio_wakeup_level polarity);

#endif

#endif