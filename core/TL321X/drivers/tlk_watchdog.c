#include "core/include/tlk_watchdog.h"
#include "core/include/tlk_analog.h"
#include "common/include/tlk_init.h"
#include "registers/tlk_timer_reg.h"

#define TLK_PCLK_24MHZ  24

#define TLK_TMR_WDT_ENABLE       TLK_BIT(0)
#define TLK_REG_WDT_CTRL         REG_ADDR8(TIMER_BASE_ADDR + 0x22)
#define TLK_REG_WDT_INTERVAL     REG_ADDR32(TIMER_BASE_ADDR + 0x0c)
#define TLK_REG_WDT_32K_CTRL     0x79
#define TLK_REG_WDT_32K_INTERVAL 0x7A

static volatile bool tlk_is_wdt_start = false;


void tlk_wdt_32k_start(unsigned int timeout_ms)
{
    // @TODO: create a separete clock driver
    extern unsigned int tlk_clock_get_32k_tick(void);
    unsigned int wdt_target_tick = tlk_clock_get_32k_tick() + 32 * timeout_ms;
    tlk_is_wdt_start = true;

    tlk_analog_write_reg8(TLK_REG_WDT_32K_INTERVAL, wdt_target_tick >> 8);
    tlk_analog_write_reg8(TLK_REG_WDT_32K_INTERVAL + 1, wdt_target_tick >> 16);
    tlk_analog_write_reg8(TLK_REG_WDT_32K_INTERVAL + 2, wdt_target_tick >> 24);

    tlk_analog_write_reg8(TLK_REG_WDT_32K_CTRL, tlk_analog_read_reg8(TLK_REG_WDT_32K_CTRL) | 0x01);
}

void tlk_wdt_32k_stop(void)
{
    tlk_analog_write_reg8(TLK_REG_WDT_32K_CTRL, tlk_analog_read_reg8(TLK_REG_WDT_32K_CTRL) & ~0x01);
}

void tlk_wdt_32k_pm_restore(void)
{
    tlk_analog_write_reg8(TLK_REG_WDT_32K_CTRL, tlk_analog_read_reg8(TLK_REG_WDT_32K_CTRL) | tlk_is_wdt_start);
}

unsigned char tlk_wd_32k_get_status(void)
{
    return (tlk_analog_read_reg8(0x69) & 0x80);
}

unsigned char tlk_wd_get_status(void)
{
    return (reg_tmr_sta0 & FLD_TMR_STA_WD);
}

TLK_REGISTER_AFTER_SLEEP(tlk_wdt_32k_pm_restore, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)


_tlk_attribute_ram_code_sec_noinline_ void tlk_wdt_start(unsigned int timeout_ms) {
    unsigned int tmp_period_ms = timeout_ms * TLK_PCLK_24MHZ * 1000;

    TLK_REG_WDT_INTERVAL = tmp_period_ms;
    TLK_REG_WDT_CTRL |= TLK_TMR_WDT_ENABLE;
}

void tlk_wdt_stop(void) {
    TLK_REG_WDT_CTRL &= ~TLK_TMR_WDT_ENABLE;
}

TLK_REGISTER_PRE_INIT(tlk_wdt_stop, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_AFTER_SLEEP(tlk_wdt_stop, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)