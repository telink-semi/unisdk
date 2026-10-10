#ifndef TLK_LOG_H_
#define TLK_LOG_H_

#include <stdarg.h>
#include <stdio.h>

#define TLK_LOG_LEVEL_NONE_DEF  0
#define TLK_LOG_LEVEL_ERROR_DEF 1
#define TLK_LOG_LEVEL_WARN_DEF  2
#define TLK_LOG_LEVEL_INFO_DEF  3
#define TLK_LOG_LEVEL_DEBUG_DEF 4

#ifndef CONFIG_TLK_DEBUG_LOG_LEVEL
#define CONFIG_TLK_DEBUG_LOG_LEVEL TLK_LOG_LEVEL_NONE_DEF
#endif

#ifndef CONFIG_TLK_DEBUG_LOG_QUEUE_SIZE
#define CONFIG_TLK_DEBUG_LOG_QUEUE_SIZE 1
#endif

typedef struct
{
    const char* module;
} tlk_log_t;

#define TLK_LOG_CREATE(logObject, log_module_name)                                                 \
    __attribute__((unused)) static const tlk_log_t logObject = {log_module_name}

typedef enum
{
    TLK_LOG_LEVEL_NONE  = TLK_LOG_LEVEL_NONE_DEF,
    TLK_LOG_LEVEL_ERROR = TLK_LOG_LEVEL_ERROR_DEF,
    TLK_LOG_LEVEL_WARN  = TLK_LOG_LEVEL_WARN_DEF,
    TLK_LOG_LEVEL_INFO  = TLK_LOG_LEVEL_INFO_DEF,
    TLK_LOG_LEVEL_DEBUG = TLK_LOG_LEVEL_DEBUG_DEF
} tlk_log_level_t;

static inline void tlk_log_dummy(const tlk_log_t* log __unused, tlk_log_level_t level __unused,
                                 const char* fmt __unused, ...)
{
}

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC) &&                                      \
    TLK_IS_DISABLED(CONFIG_TLK_TASK_PLANNER)
void tlk_debug_log_interface_loop(void);
#define tlk_log_iface_loop() tlk_debug_log_interface_loop()
#else
#define tlk_log_iface_loop()
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_LOW) || TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH)
/**
 * @brief       This function serves to periodically extract
 *              and print the logs from log queue buffer.
 * @return      none.
 */
void tlk_debug_log_pump(void);
#define tlk_logrotate() tlk_debug_log_pump()

/**
 * @brief       This function serves to set the log message to print queue.
 * @param[in]   log - the log object.
 * @param[in]   level - the log level.
 * @param[in]   fmt - message with formatters.
 * @return      none.
 */
void tlk_debug_log_queue(const tlk_log_t* log, tlk_log_level_t level, const char* fmt, ...);
#define tlk_log(tlk_log_obj, logLevel, fmt, ...)                                                   \
    tlk_debug_log_queue(&(tlk_log_obj), (logLevel), (fmt), ##__VA_ARGS__)

#elif TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_IMMEDIATE)
/**
 * @brief       This function serves to print the log message immediately.
 * @param[in]   log - the log object.
 * @param[in]   level - the log level.
 * @param[in]   fmt - message with formatters.
 * @return      none.
 */
void tlk_debug_log_blocking(const tlk_log_t* log, tlk_log_level_t level, const char* fmt, ...);
#define tlk_log(tlk_log_obj, logLevel, fmt, ...)                                                   \
    tlk_debug_log_blocking(&(tlk_log_obj), (logLevel), (fmt), ##__VA_ARGS__)
#define tlk_logrotate()

#else

#define tlk_log(tlk_log_obj, logLevel, fmt, ...)                                                   \
    tlk_log_dummy(&(tlk_log_obj), (logLevel), (fmt), ##__VA_ARGS__)
#define tlk_logrotate()

#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_FROM_ISR)
/**
 * @brief       This function serves to set the log message to print queue from ISR without
 * formatting.
 * @param[in]   log - the log object.
 * @param[in]   level - the log level.
 * @param[in]   msg - message without formatters.
 * @return      none.
 */
void tlk_debug_log_isr(const tlk_log_t* log, tlk_log_level_t level, const char* msg);
#define TLK_LOG_ISR(log, level, msg) tlk_debug_log_isr(&(log), level, msg);
#else
#define TLK_LOG_ISR(log, level, msg) tlk_log_dummy(&(log), level, msg)
#endif

/**
 * @brief       This function serves to log a memory buffer as a hex dump
 *              (offset, hex bytes and printable ASCII), one 16-byte line at a time.
 * @param[in]   log - the log object.
 * @param[in]   level - the log level.
 * @param[in]   data - pointer to the memory to dump.
 * @param[in]   len - number of bytes to dump.
 * @return      none.
 */
void tlk_log_hexdump(const tlk_log_t* log, tlk_log_level_t level, const void* data, uint32_t len);

#define TLK_LOG_IS_DEBUG_LEVEL_ENABLED (CONFIG_TLK_DEBUG_LOG_LEVEL >= TLK_LOG_LEVEL_DEBUG_DEF)
#define TLK_LOG_DEBUG(log, fmt, ...)                                                               \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_DEBUG_LEVEL_ENABLED)                                                        \
            tlk_log((log), (tlk_log_level_t) TLK_LOG_LEVEL_DEBUG_DEF, (fmt), ##__VA_ARGS__);       \
    } while (0)
#define TLK_LOG_DEBUG_ISR(log, msg)                                                                \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_DEBUG_LEVEL_ENABLED)                                                        \
            TLK_LOG_ISR((log), (tlk_log_level_t) TLK_LOG_LEVEL_DEBUG_DEF, (msg));                  \
    } while (0)
#define TLK_LOG_HEXDUMP_DEBUG(log, data, len)                                                      \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_DEBUG_LEVEL_ENABLED)                                                        \
            tlk_log_hexdump(&(log), (tlk_log_level_t) TLK_LOG_LEVEL_DEBUG_DEF, (data), (len));     \
    } while (0)

#define TLK_LOG_IS_INFO_LEVEL_ENABLED (CONFIG_TLK_DEBUG_LOG_LEVEL >= TLK_LOG_LEVEL_INFO_DEF)
#define TLK_LOG_INFO(log, fmt, ...)                                                                \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_INFO_LEVEL_ENABLED)                                                         \
            tlk_log((log), (tlk_log_level_t) TLK_LOG_LEVEL_INFO_DEF, (fmt), ##__VA_ARGS__);        \
    } while (0)
#define TLK_LOG_INFO_ISR(log, msg)                                                                 \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_INFO_LEVEL_ENABLED)                                                         \
            TLK_LOG_ISR((log), (tlk_log_level_t) TLK_LOG_LEVEL_INFO_DEF, (msg));                   \
    } while (0)
#define TLK_LOG_HEXDUMP_INFO(log, data, len)                                                       \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_INFO_LEVEL_ENABLED)                                                         \
            tlk_log_hexdump(&(log), (tlk_log_level_t) TLK_LOG_LEVEL_INFO_DEF, (data), (len));      \
    } while (0)

#define TLK_LOG_IS_WARN_LEVEL_ENABLED (CONFIG_TLK_DEBUG_LOG_LEVEL >= TLK_LOG_LEVEL_WARN_DEF)
#define TLK_LOG_WARN(log, fmt, ...)                                                                \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_WARN_LEVEL_ENABLED)                                                         \
            tlk_log((log), (tlk_log_level_t) TLK_LOG_LEVEL_WARN_DEF, (fmt), ##__VA_ARGS__);        \
    } while (0)
#define TLK_LOG_WARN_ISR(log, msg)                                                                 \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_WARN_LEVEL_ENABLED)                                                         \
            TLK_LOG_ISR((log), (tlk_log_level_t) TLK_LOG_LEVEL_WARN_DEF, (msg));                   \
    } while (0)
#define TLK_LOG_HEXDUMP_WARN(log, data, len)                                                       \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_WARN_LEVEL_ENABLED)                                                         \
            tlk_log_hexdump(&(log), (tlk_log_level_t) TLK_LOG_LEVEL_WARN_DEF, (data), (len));      \
    } while (0)

#define TLK_LOG_IS_ERROR_LEVEL_ENABLED (CONFIG_TLK_DEBUG_LOG_LEVEL >= TLK_LOG_LEVEL_ERROR_DEF)
#define TLK_LOG_ERROR(log, fmt, ...)                                                               \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_ERROR_LEVEL_ENABLED)                                                        \
            tlk_log((log), (tlk_log_level_t) TLK_LOG_LEVEL_ERROR_DEF, (fmt), ##__VA_ARGS__);       \
    } while (0)
#define TLK_LOG_ERROR_ISR(log, msg)                                                                \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_ERROR_LEVEL_ENABLED)                                                        \
            TLK_LOG_ISR((log), (tlk_log_level_t) TLK_LOG_LEVEL_ERROR_DEF, (msg));                  \
    } while (0)
#define TLK_LOG_HEXDUMP_ERROR(log, data, len)                                                      \
    do                                                                                             \
    {                                                                                              \
        if (TLK_LOG_IS_ERROR_LEVEL_ENABLED)                                                        \
            tlk_log_hexdump(&(log), TLK_LOG_LEVEL_ERROR_DEF, (data), (len));                       \
    } while (0)

#endif
