#include "core/include/tlk_mtimer.h"
#include "core/include/tlk_core.h"
#include "registers/tlk_mtimer.h"

/* ==== MACROS ==== */
#define TLK_MTIMER_MIE_MASK                                                                        \
    ((TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_NEST_PLIC_SW) ? TLK_FLD_MIE_MSIE : 0) |                 \
     (TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_NEST_PLIC) ? TLK_FLD_MIE_MEIE : 0))
/* ==== MACROS ==== */

#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_ENABLE)
static void (*tlk_mtimer_callback)(void) = NULL;
#endif

/* ==== DRIVER APIs ==== */
void tlk_mtimer_set_mtime(uint64_t time)
{
    tlk_mtimer_reg.time_l = 0;
    tlk_mtimer_reg.time_h = (unsigned int) (time >> 32);
    tlk_mtimer_reg.time_l = (unsigned int) time;
}

uint64_t tlk_mtimer_get_mtime(void)
{
    unsigned int mtime_l, mtime_h;

    do
    {
        mtime_h = tlk_mtimer_reg.time_h;
        mtime_l = tlk_mtimer_reg.time_l;
    } while (mtime_h != tlk_mtimer_reg.time_h);

    return (((uint64_t) mtime_h) << 32) | mtime_l;
}

void tlk_mtimer_set_mtime_compare(uint64_t time)
{
    tlk_mtimer_reg.compare_l = 0xffffffff;
    tlk_mtimer_reg.compare_h = (unsigned int) (time >> 32);
    tlk_mtimer_reg.compare_l = (unsigned int) time;
}

uint64_t tlk_mtimer_get_mtime_compare(void)
{
    unsigned int mtimecmp_l, mtimecmp_h;

    mtimecmp_h = tlk_mtimer_reg.compare_h;
    mtimecmp_l = tlk_mtimer_reg.compare_l;

    return (((uint64_t) mtimecmp_h) << 32) | mtimecmp_l;
}

#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_ENABLE)
void tlk_mtimer_irq_register_callback(void (*cb)(void))
{
    tlk_mtimer_callback = cb;
}
#endif
/* ==== DRIVER APIs ==== */

/* ==== IRQ HELPERS ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_ENABLE)
_tlk_attribute_ram_code_sec_ void tlk_mtimer_irq_handle(void)
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
/* ==== IRQ HELPERS ==== */
