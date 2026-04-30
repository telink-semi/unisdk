#include "core/include/tlk_mtimer.h"
#include "tlk_core.h"
#include "common/include/tlk_init.h"
#include "system/debug/log/tlk_log.h"
#include "registers/tlk_mtimer.h"

/**
 * @brief Get Machine Timer Compare value.
 */
unsigned long long tlk_mtimer_get_mtime_compare(void)
{
    unsigned int mtimecmp_l, mtimecmp_h;

    mtimecmp_h = tlk_mtimer_reg.compare_h;
    mtimecmp_l = tlk_mtimer_reg.compare_l;

    return (((unsigned long long)mtimecmp_h) << 32) | mtimecmp_l;
}

/**
 * @brief Get Machine Timer value.
 */
unsigned long long tlk_mtimer_get_mtime(void)
{
    unsigned int mtime_l, mtime_h;

    do {
        mtime_h = tlk_mtimer_reg.time_h;
        mtime_l = tlk_mtimer_reg.time_l;
    } while (mtime_h != tlk_mtimer_reg.time_h);
    return (((unsigned long long)mtime_h) << 32) | mtime_l;
}

/**
 * @brief Set Machine Timer Compare value.
 */
void tlk_mtimer_set_mtime_compare(unsigned long long time)
{
    tlk_mtimer_reg.compare_l = 0xffffffff;
    tlk_mtimer_reg.compare_h = (unsigned int)(time >> 32);
    tlk_mtimer_reg.compare_l = (unsigned int)time;
}

/**
 * @brief Set Machine Timer value.
 */
void tlk_mtimer_set_mtime(unsigned long long time)
{
    tlk_mtimer_reg.time_l = 0;
    tlk_mtimer_reg.time_h = (unsigned int)(time >> 32);
    tlk_mtimer_reg.time_l = (unsigned int)time;
}

static void mtimer_init(void)
{
    tlk_mtimer_set_mtime(0);
}

TLK_REGISTER_PRE_INIT(mtimer_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL);

#if IS_ENABLED(CONFIG_TLK_MTIMER_ENABLE_IRQ)

TLK_LOG_CREATE(mtimer_log, "MTIMER");

/**
 * @brief     Default mtime irq handler.
 * @return    None.
 */
void tlk_mtime_default_handler(void)
{
    TLK_LOG_WARN(mtimer_log, "Warning! Default Machine Timer IRQ Handler!\n");
}

void (*tlk_mtimer_callback)(void) = tlk_mtime_default_handler;

void tlk_mtimer_register_callback(void (*cb)(void))
{
    tlk_mtimer_callback = cb;
}

void tlk_mtimer_handler(void)
{
#if IS_ENABLED(CONFIG_TLK_MTIMER_ENABLE_NESTING)
    /* save CSRs to stack;*/
    tlk_core_save_nested_context();
    /* before enable global interrupt,disable the timer interrupt to prevent re-entry */
    tlk_core_mie_disable(TLK_FLD_MIE_MTIE);
    /* enable global interupt */
    tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
#endif

    tlk_mtimer_callback();

#if IS_ENABLED(CONFIG_TLK_MTIMER_ENABLE_NESTING)
    /* disable global interrupts before critical section */
    tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* restore CSRs */
    tlk_core_restore_nested_context();
    /* re-enable the timer interrupt. */
    tlk_core_mie_enable(TLK_FLD_MIE_MTIE);
#endif
}

#endif
