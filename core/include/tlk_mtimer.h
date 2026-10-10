#ifndef TLK_INCLUDE_DRIVERS_MTIMER_H_
#define TLK_INCLUDE_DRIVERS_MTIMER_H_

#if TLK_IS_DISABLED(CONFIG_TLK_MTIMER)
#warning Mtimer driver is disabled in Kconfig, but its header is being used; note that its implementation will not be linked until you enable it in Kconfig.
#endif

/**
 * @brief       This function servers to set machine timer counter value.
 * @param[in]   time - the time in mtimer ticks.
 * @return      none.
 */
void tlk_mtimer_set_mtime(uint64_t time);

/**
 * @brief       This function servers to get machine timer counter value.
 * @return      mtimer counter value.
 */
uint64_t tlk_mtimer_get_mtime(void);

/**
 * @brief       This function servers to set machine timer compare register value.
 * @param[in]   time - the compare time in mtimer ticks.
 * @return      none.
 */
void tlk_mtimer_set_mtime_compare(uint64_t time);

/**
 * @brief       This function servers to get machine timer compare register value.
 * @return      mtimer compare register value.
 */
uint64_t tlk_mtimer_get_mtime_compare(void);

#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ)

/**
 * @brief       This function servers to set machine timer IRQ fire callback.
 * @param[in]   cb - the callback fires in case of machine timer IRQ.
 * @return      none.
 */
void tlk_mtimer_irq_register_callback(void (*cb)(void));

#endif

#endif
