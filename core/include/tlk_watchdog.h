#ifndef TLK_INCLUDE_DRIVERS_WATCHDOG_H_
#define TLK_INCLUDE_DRIVERS_WATCHDOG_H_

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
