#include "core/include/tlk_watchdog.h"
#include "common/include/tlk_init.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "drivers/include/tlk_advanced_stimer.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_timer.h"

static struct tlk_wdt_device
{
    uint32_t timeout_ms;
} tlk_wdt;

/* Digital WDT API (hidden) */

_tlk_attribute_ram_code_sec_ static void tlk_wdt_dig_start(uint32_t timeout_ms)
{
    uint32_t period = timeout_ms * tlk_sys_clk.pclk * 1000;

    tlk_timer_reg.wdt_interval        = period;
    tlk_timer_reg.control2_bit.wdt_en = 1;
}

_tlk_attribute_ram_code_sec_ static void tlk_wdt_dig_stop(void)
{
    tlk_timer_reg.control2_bit.wdt_en = 0;
}

/* Helper functions */

#if TLK_IS_ENABLED(CONFIG_TLK_WDT_STARTUP_ENABLE)

_tlk_attribute_ram_code_sec_ void tlk_wdt_switch(void)
{
    tlk_wdt_start(CONFIG_TLK_WDT_STARTUP_TIMEOUT);
    tlk_wdt_dig_stop();
}

TLK_REGISTER_PRE_INIT(tlk_wdt_switch, TLK_INIT_LEVEL_SYSTEM, TLK_INIT_PRIORITY_LOW)

#endif

/* WDT driver API */

_tlk_attribute_ram_code_sec_ void tlk_wdt_start(uint32_t timeout_ms)
{
    uint32_t wdt_target_tick = tlk_stimer_get_32k_tick() + 32 * timeout_ms;
    tlk_wdt.timeout_ms       = timeout_ms;

    tlk_analog_write_reg8(TLK_AREG_WDT_INTERVAL(0), wdt_target_tick >> 8);
    tlk_analog_write_reg8(TLK_AREG_WDT_INTERVAL(1), wdt_target_tick >> 16);
    tlk_analog_write_reg8(TLK_AREG_WDT_INTERVAL(2), wdt_target_tick >> 24);

    TLK_ANALOG_MODIFY(TLK_AREG_WDT_CTRL, (value.bit.wdt_en = 1;));
}

_tlk_attribute_ram_code_sec_ void tlk_wdt_stop(void)
{
    TLK_ANALOG_MODIFY(TLK_AREG_WDT_CTRL, (value.bit.wdt_en = 0;));
}

_tlk_attribute_ram_code_sec_ void tlk_wdt_feed(void)
{
    tlk_wdt_dig_start(5000);
    tlk_wdt_stop();
    tlk_wdt_start(tlk_wdt.timeout_ms);
    tlk_wdt_dig_stop();
}
