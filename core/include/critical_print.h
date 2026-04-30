#ifndef TLK_CORE_CRITICAL_PRINT_H_
#define TLK_CORE_CRITICAL_PRINT_H_

/**
 * @brief Outputs a critical-level diagnostic message.
 *
 * This function transmits a null-terminated string containing
 * a critical diagnostic message to the UART backend,
 * when CONFIG_TLK_CRITICAL_PRINTOUT is enabled.
 *
 * The macro @ref tlk_critical_print() expands to this function
 * only if CONFIG_TLK_CRITICAL_PRINTOUT is defined at compile time.
 * Otherwise, the macro expands to an empty statement and the
 * call is compiled out.
 *
 * @note The input string must be null-terminated.
 * @note This function does not perform formatting. The caller
 *       must provide a fully formatted string.
 * @note Behavior (blocking)
 *
 * @param[in] buf Pointer to a null-terminated character buffer
 *                containing the message to output.
 *
 * @see tlk_critical_print
 */
#if IS_ENABLED(CONFIG_TLK_CRITICAL_PRINTOUT)
    void _tlk_critical_print(const char *buf);
    void _tlk_critical_printf(const char *fmt, ...);

    #define tlk_critical_print(buf) _tlk_critical_print(buf)
    #define tlk_critical_printf(fmt, ...) _tlk_critical_printf((fmt), ##__VA_ARGS__)
#else
    #define tlk_critical_print(buf)
    #define tlk_critical_printf(fmt, ...)
#endif

#endif
