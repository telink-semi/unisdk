#include "core/include/tlk_mtimer.h"
#include "core/include/tlk_core.h"
#include "registers/tlk_mtimer.h"

#define TLK_MTIMER_MIE_MASK                                                                        \
    ((TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_NEST_PLIC_SW) ? TLK_FLD_MIE_MSIE : 0) |                 \
     (TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_NEST_PLIC) ? TLK_FLD_MIE_MEIE : 0))

/**
 * @brief Set Machine Timer value.
 */
void tlk_mtimer_set_mtime(unsigned long long time)
{
    tlk_mtimer_reg.time_l = 0;
    tlk_mtimer_reg.time_h = (unsigned int) (time >> 32);
    tlk_mtimer_reg.time_l = (unsigned int) time;
}

/**
 * @brief Get Machine Timer value.
 */
unsigned long long tlk_mtimer_get_mtime(void)
{
    unsigned int mtime_l, mtime_h;

    do
    {
        mtime_h = tlk_mtimer_reg.time_h;
        mtime_l = tlk_mtimer_reg.time_l;
    } while (mtime_h != tlk_mtimer_reg.time_h);

    return (((unsigned long long) mtime_h) << 32) | mtime_l;
}

/**
 * @brief Set Machine Timer Compare value.
 */
void tlk_mtimer_set_mtime_compare(unsigned long long time)
{
    tlk_mtimer_reg.compare_l = 0xffffffff;
    tlk_mtimer_reg.compare_h = (unsigned int) (time >> 32);
    tlk_mtimer_reg.compare_l = (unsigned int) time;
}

/**
 * @brief Get Machine Timer Compare value.
 */
unsigned long long tlk_mtimer_get_mtime_compare(void)
{
    unsigned int mtimecmp_l, mtimecmp_h;

    mtimecmp_h = tlk_mtimer_reg.compare_h;
    mtimecmp_l = tlk_mtimer_reg.compare_l;

    return (((unsigned long long) mtimecmp_h) << 32) | mtimecmp_l;
}

#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_ENABLE)

static void (*tlk_mtimer_callback)(void) = NULL;

void tlk_mtimer_register_callback(void (*cb)(void))
{
    tlk_mtimer_callback = cb;
}

_tlk_attribute_ram_code_sec_ void tlk_mtimer_handler(void)
{
#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_NEST_SELECTED)
    /* save CSRs to stack */
    tlk_core_save_nested_context();
    /* 1. set global interupts
     * 2. before enable global interrupts, disable the timer interrupt to prevent re-entry */
    tlk_write_csr(NDS_MIE, TLK_MTIMER_MIE_MASK & tlk_read_csr(NDS_MIE));
    /* enable global interupts */
    tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
#endif

    /* call the handler */
    if (tlk_mtimer_callback)
        tlk_mtimer_callback();

#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_NEST_SELECTED)
    /* disable global interrupts before critical section */
    tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* restore CSRs */
    tlk_core_restore_nested_context();
#endif
}

#endif
