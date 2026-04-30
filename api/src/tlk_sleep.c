#include "api/include/tlk_sleep.h"
#include "core/include/tlk_mtimer.h"
#include "tlk_core.h"
#include "common/include/tlk_init.h"
#include "properties/tlk_mtimer.h"

#if IS_ENABLED(CONFIG_TLK_PM)
    #include "core/include/tlk_pm.h"
#endif

#define TLK_API_MS_DELAY_TO_TICKS(ms)  ((uint32_t)(((uint64_t)(ms) * UNISDK_MTIMER_CYCLES_PER_SECOND) / 1000ULL))
static uint64_t current_time = 0;

void tlk_mtimer_irq_disable_cb(void)
{
    tlk_core_mie_disable(TLK_FLD_MIE_MTIE);
    tlk_mtimer_set_mtime_compare(tlk_mtimer_get_mtime() + 1);
}

void tlk_api_init(void)
{
    tlk_mtimer_register_callback(tlk_mtimer_irq_disable_cb);
}
TLK_REGISTER_PRE_INIT(tlk_api_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL);


void tlk_api_sleep(uint32_t duration_ms) 
{
    if (duration_ms < 1 ) return;

#if IS_ENABLED(CONFIG_TLK_PM)
    enum tlk_pm_sleep_mode mode = IF_ENABLED_ELSE(CONFIG_TLK_PM_RAM_RETENTION_ENABLE, 
        (TLK_PM_SLEEP_MODE_DEEP_RETENTION), 
        (IF_ENABLED_ELSE(CONFIG_TLK_PM_DEEP_SLEEP, 
            (TLK_PM_SLEEP_MODE_DEEP_SLEEP), 
            (TLK_PM_SLEEP_MODE_SUSPEND))));
    enum tlk_pm_sleep_status pm_sleep_state = tlk_pm_sleep(mode, duration_ms);

    if (pm_sleep_state == TLK_PM_SLEEP_OK || pm_sleep_state == TLK_PM_SLEEP_SUSPEND_FALLBACK)
    {
        return;
    }
    else
#endif
    {   
        uint32_t sleep_duration_tick = TLK_API_MS_DELAY_TO_TICKS(duration_ms);
        current_time = tlk_mtimer_get_mtime();

        tlk_mtimer_set_mtime_compare(current_time + sleep_duration_tick); 
        tlk_core_mie_enable(TLK_FLD_MIE_MTIE);
        tlk_core_interrupt_enable();
        
        while (tlk_mtimer_get_mtime() < current_time + sleep_duration_tick)
        {
            __asm__ __volatile__("wfi");
        }
    }
}
