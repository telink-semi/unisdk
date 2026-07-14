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

#if CONFIG_TLK_DEBUG_LOG_LEVEL > TLK_LOG_LEVEL_NONE_DEF
#define TLK_LOG_CREATE(logObject, log_module_name)                                                 \
    __attribute__((unused)) static const tlk_log_t logObject = {log_module_name}
#else
#define TLK_LOG_CREATE(logObject, log_module_name)
#endif

typedef enum
{
    TLK_LOG_LEVEL_NONE  = TLK_LOG_LEVEL_NONE_DEF,
    TLK_LOG_LEVEL_ERROR = TLK_LOG_LEVEL_ERROR_DEF,
    TLK_LOG_LEVEL_WARN  = TLK_LOG_LEVEL_WARN_DEF,
    TLK_LOG_LEVEL_INFO  = TLK_LOG_LEVEL_INFO_DEF,
    TLK_LOG_LEVEL_DEBUG = TLK_LOG_LEVEL_DEBUG_DEF
} tlk_log_level_t;

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC) &&                                      \
    TLK_IS_DISABLED(CONFIG_TLK_TASK_PLANNER)
void tlk_log_interface_loop(void);
#define tlk_log_iface_loop() tlk_log_interface_loop();
#else
#define tlk_log_iface_loop()
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_LOW) || TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH)
/**
 * @brief       This function serves to periodically extract
 *              and print the logs from log queue buffer.
 * @return      none.
 */
void tlk_log_pump(void);
#define tlk_logrotate() tlk_log_pump();

/**
 * @brief       This function serves to set the log message to print queue.
 * @param[in]   log - the log object.
 * @param[in]   level - the log level.
 * @param[in]   fmt - message with formatters.
 * @return      none.
 */
void tlk_log_queue(const tlk_log_t* log, tlk_log_level_t level, const char* fmt, ...);
#define tlk_log(tlk_log_obj, logLevel, fmt, ...)                                                   \
    tlk_log_queue(&(tlk_log_obj), (logLevel), (fmt), ##__VA_ARGS__)
#elif TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_IMMEDIATE)

/**
 * @brief       This function serves to print the log message immediately.
 * @param[in]   log - the log object.
 * @param[in]   level - the log level.
 * @param[in]   fmt - message with formatters.
 * @return      none.
 */
void tlk_log_blocking(const tlk_log_t* log, tlk_log_level_t level, const char* fmt, ...);
#define tlk_log(tlk_log_obj, logLevel, fmt, ...)                                                   \
    tlk_log_blocking(&(tlk_log_obj), (logLevel), (fmt), ##__VA_ARGS__)
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
void tlk_log_isr(const tlk_log_t* log, tlk_log_level_t level, const char* msg);
#define TLK_LOG_ISR(log, level, msg) tlk_log_isr(&(log), level, msg);
#else
#define TLK_LOG_ISR(log, level, msg)                                                               \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#endif

#if CONFIG_TLK_DEBUG_LOG_LEVEL >= TLK_LOG_LEVEL_DEBUG_DEF
#define TLK_LOG_DEBUG(log, fmt, ...) tlk_log((log), TLK_LOG_LEVEL_DEBUG_DEF, (fmt), ##__VA_ARGS__)
#define TLK_LOG_DEBUG_ISR(log, msg)  TLK_LOG_ISR((log), TLK_LOG_LEVEL_DEBUG_DEF, (msg))
#else
#define TLK_LOG_DEBUG(log, fmt, ...)                                                               \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#define TLK_LOG_DEBUG_ISR(log, msg)                                                                \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#endif

#if CONFIG_TLK_DEBUG_LOG_LEVEL >= TLK_LOG_LEVEL_INFO_DEF
#define TLK_LOG_INFO(log, fmt, ...) tlk_log((log), TLK_LOG_LEVEL_INFO_DEF, (fmt), ##__VA_ARGS__)
#define TLK_LOG_INFO_ISR(log, msg)  TLK_LOG_ISR((log), TLK_LOG_LEVEL_INFO_DEF, (msg))
#else
#define TLK_LOG_INFO(log, fmt, ...)                                                                \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#define TLK_LOG_INFO_ISR(log, msg)                                                                 \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#endif

#if CONFIG_TLK_DEBUG_LOG_LEVEL >= TLK_LOG_LEVEL_WARN_DEF
#define TLK_LOG_WARN(log, fmt, ...) tlk_log((log), TLK_LOG_LEVEL_WARN_DEF, (fmt), ##__VA_ARGS__)
#define TLK_LOG_WARN_ISR(log, msg)  TLK_LOG_ISR((log), TLK_LOG_LEVEL_WARN_DEF, (msg))
#else
#define TLK_LOG_WARN(log, fmt, ...)                                                                \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#define TLK_LOG_WARN_ISR(log, msg)                                                                 \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#endif

#if CONFIG_TLK_DEBUG_LOG_LEVEL >= TLK_LOG_LEVEL_ERROR_DEF
#define TLK_LOG_ERROR(log, fmt, ...) tlk_log((log), TLK_LOG_LEVEL_ERROR_DEF, (fmt), ##__VA_ARGS__)
#define TLK_LOG_ERROR_ISR(log, msg)  TLK_LOG_ISR((log), TLK_LOG_LEVEL_ERROR_DEF, (msg))
#else
#define TLK_LOG_ERROR(log, fmt, ...)                                                               \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#define TLK_LOG_ERROR_ISR(log, msg)                                                                \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#endif

#endif
