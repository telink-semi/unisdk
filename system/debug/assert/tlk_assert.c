#include "system/debug/log/tlk_log.h"

TLK_LOG_CREATE(assert_log, "ASSERT");

void __assert_func(const char* file, uint32_t line, const char* func, const char* expr)
{
    TLK_LOG_ERROR(assert_log,
                  "ASSERT FAILED: (%s) at %s:%lu, %s",
                  expr ? expr : "?",
                  file ? file : "?",
                  (unsigned long) line,
                  func ? func : "?");

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_ASSERT_MCU_MODE_REBOOT)
    TLK_LOG_ERROR(assert_log, "MCU action: reboot");

#elif TLK_IS_ENABLED(CONFIG_TLK_DEBUG_ASSERT_MCU_MODE_STALL)
    TLK_LOG_ERROR(assert_log, "MCU action: stall");
    while (1)
        ;

#elif TLK_IS_ENABLED(CONFIG_TLK_DEBUG_ASSERT_MCU_MODE_DEEP_SLEEP)
    TLK_LOG_ERROR(assert_log, "MCU action: deep sleep");

#endif
}
