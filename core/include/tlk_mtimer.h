#ifndef TLK_INCLUDE_DRIVERS_MTIMER_H_
#define TLK_INCLUDE_DRIVERS_MTIMER_H_

/**
 * @brief       This function servers to set machine timer counter value.
 * @param[in]   time - the time in mtimer ticks.
 * @return      none.
 */
void tlk_mtimer_set_mtime(unsigned long long time);

/**
 * @brief       This function servers to get machine timer counter value.
 * @return      mtimer counter value.
 */
unsigned long long tlk_mtimer_get_mtime(void);

/**
 * @brief       This function servers to set machine timer compare register value.
 * @param[in]   time - the compare time in mtimer ticks.
 * @return      none.
 */
void tlk_mtimer_set_mtime_compare(unsigned long long time);

/**
 * @brief       This function servers to get machine timer compare register value.
 * @return      mtimer compare register value.
 */
unsigned long long tlk_mtimer_get_mtime_compare(void);

#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_ENABLE)

/**
 * @brief       This function servers to set machine timer IRQ fire callback.
 * @param[in]   cb - the callback fires in case of machine timer IRQ.
 * @return      none.
 */
void tlk_mtimer_register_callback(void (*cb)(void));

#endif

#endif
