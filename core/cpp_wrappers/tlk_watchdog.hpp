#ifndef WDT_DRIVER_HPP
#define WDT_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_watchdog.h"
#ifdef __cplusplus
}
#endif

class WDTDriver
{
  public:
    explicit WDTDriver() = delete;

    static void start(uint32_t timeout_ms)
    {
        tlk_wdt_start(timeout_ms);
    }

    static void stop(void)
    {
        tlk_wdt_stop();
    }

    static void feed(void)
    {
        tlk_wdt_feed();
    }
};

#endif // WDT_DRIVER_HPP
