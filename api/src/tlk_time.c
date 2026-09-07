#include "api/include/tlk_time.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_stimer.h"

void tlk_api_time_delay(uint32_t duration_us)
{
    // TODO: Add option to select the default delay source: cclk or stimer
    // ((unsigned int)(tlk_stimer_get_tick() - ref) > us * TLK_SYSTEM_TIMER_TICK_1US);
    tlk_clock_cclk_delay((uint64_t) (tlk_sys_clk.cclk * duration_us));
}

uint32_t tlk_api_time_get_micros(void)
{
    return tlk_stimer_get_tick() / TLK_SYSTEM_TIMER_TICK_1US;
}

uint32_t tlk_api_time_get_millis(void)
{
    return tlk_stimer_get_tick() / TLK_SYSTEM_TIMER_TICK_1MS;
}
