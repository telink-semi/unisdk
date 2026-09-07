
#ifdef __cplusplus
extern "C"
{
#endif
#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_gpio.h"
#include "system/debug/log/tlk_log.h"
#include "tlk_board_pinout.h"
#ifdef __cplusplus
}
#endif

TLK_LOG_CREATE(demo_log, "LOG_DEMO");

#if TLK_IS_DISABLED(CONFIG_TLK_TASK_PLANNER)

static uint32_t last = 0;
int             main(void)
{
    while (1)
    {
        tlk_log_iface_loop();
        tlk_logrotate();
        uint32_t now = tlk_api_micros();
        if (now - last >= 1000000)
        {
            last = now;
            tlk_log(demo_log, TLK_LOG_LEVEL_ERROR, "Test log USB");
        }
    }
}

#elif TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_LOOP)

#ifdef __cplusplus
extern "C"
{
#endif
    void user_setup(void) {}

    void user_loop(void)
    {
        static uint32_t last = 0;
        uint32_t        now  = tlk_api_micros();
        if (now - last >= 1000000)
        {
            last = now;
            tlk_log(demo_log, TLK_LOG_LEVEL_ERROR, "Test log USB from Telink Loop");
        }
    }
#ifdef __cplusplus
}
#endif

#elif TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER)

#ifdef __cplusplus
extern "C"
{
#endif
    void user_setup(void) {}

    void user_loop(void)
    {
        tlk_log(demo_log, TLK_LOG_LEVEL_ERROR, "Test log USB with Telink Scheduler");
    }
#ifdef __cplusplus
}
#endif
#endif
