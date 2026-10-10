#ifndef TLK_INCLUDE_DRIVERS_WATCHDOG_H_
#define TLK_INCLUDE_DRIVERS_WATCHDOG_H_

#if TLK_IS_DISABLED(CONFIG_TLK_WDT)
#warning WDT driver is disabled in Kconfig, but its header is being used; note that its implementation will not be linked until you enable it in Kconfig.
#endif

/**
 * @brief Start the watchdog with the given timeout.
 *
 * @param timeout_ms Timeout in milliseconds.
 *
 * @return None.
 */
void tlk_wdt_start(uint32_t timeout_ms);

/**
 * @brief Stop the watchdog.
 *
 * @return None.
 */
void tlk_wdt_stop(void);

/**
 * @brief Feed the watchdog.
 *
 * @return None.
 */
void tlk_wdt_feed(void);

#endif
