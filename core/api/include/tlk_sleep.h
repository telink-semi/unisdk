#ifndef TLK_API_SLEEP_H_
#define TLK_API_SLEEP_H_

#if TLK_IS_DISABLED(CONFIG_TLK_API_SLEEP)
#warning Sleep API is disabled in Kconfig, but its header is being used; note that its implementation will not be linked until you enable it in Kconfig.
#endif

/**
 * @brief     Non-blocking sleep function.
 * @param     duration_ms - sleep duration in milliseconds.
 * @return    None.
 */
void tlk_api_sleep(uint32_t duration_ms);

#endif
