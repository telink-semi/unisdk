#ifndef TLK_API_TIME_H_
#define TLK_API_TIME_H_

#if TLK_IS_DISABLED(CONFIG_TLK_API_TIME)
#warning Time API is disabled in Kconfig, but its header is being used; note that its implementation will not be linked until you enable it in Kconfig.
#endif

/**
 * @brief     Blocking delay function.
 * @param     duration_us delay duration in microseconds.
 * @return    None.
 */
void tlk_api_time_delay(uint32_t duration_us);

/**
 * @brief     Gets the current microsecond from ticks.
 * @return    Current microseconds for the time comparison.
 */
uint32_t tlk_api_time_micros(void);

/**
 * @brief     Gets the current millisecond from ticks.
 * @return    Current milliseconds for the time comparison.
 */
uint32_t tlk_api_time_millis(void);

#endif
