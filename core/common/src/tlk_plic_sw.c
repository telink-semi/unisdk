#include "core/include/tlk_plic_sw.h"
#include "core/include/tlk_core.h"
#include "properties/tlk_plic_sw.h"
#include "registers/tlk_plic_sw.h"

/* ==== MACROS ==== */
#define TLK_PLIC_SW_MIE_MASK                                                                       \
    ((TLK_IS_ENABLED(CONFIG_TLK_PLIC_SW_NEST_PLIC_SW) ? TLK_FLD_MIE_MSIE : 0) |                    \
     (TLK_IS_ENABLED(CONFIG_TLK_PLIC_SW_NEST_MTIMER) ? TLK_FLD_MIE_MTIE : 0) |                     \
     (TLK_IS_ENABLED(CONFIG_TLK_PLIC_SW_NEST_PLIC) ? TLK_FLD_MIE_MEIE : 0))
/* ==== MACROS ==== */

static void (*tlk_plic_sw_callback)(void) = NULL;

/* ==== DRIVER APIs ==== */
_tlk_attribute_ram_code_sec_ void tlk_plic_sw_interrupt_enable(void)
{
    /* Enable interrupts in PLIC SW */
    tlk_plic_sw_reg.irq_en |= TLK_BIT(UNISDK_PLIC_SW_IRQ_NUM);

    /* Enable PLIC SW interrupt source */
    tlk_set_csr(NDS_MIE, TLK_FLD_MIE_MSIE);
}

_tlk_attribute_ram_code_sec_ void tlk_plic_sw_interrupt_disable(void)
{
    /* Disable interrupts in PLIC SW */
    tlk_plic_sw_reg.irq_en &= (~TLK_BIT(UNISDK_PLIC_SW_IRQ_NUM));

    /* Disable PLIC SW interrupt source */
    tlk_clear_csr(NDS_MIE, TLK_FLD_MIE_MSIE);
}

_tlk_attribute_ram_code_sec_ void tlk_plic_sw_set_pending(void)
{
    tlk_plic_sw_reg.irq_pending = TLK_BIT(UNISDK_PLIC_SW_IRQ_NUM);
}

_tlk_attribute_ram_code_sec_ uint32_t tlk_plic_sw_interrupt_claim(void)
{
    return tlk_plic_sw_reg.irq_claim_compl;
}

_tlk_attribute_ram_code_sec_ void tlk_plic_sw_interrupt_complete(void)
{
    tlk_plic_sw_reg.irq_claim_compl = UNISDK_PLIC_SW_IRQ_NUM;
}

_tlk_attribute_ram_code_sec_ void tlk_plic_sw_register_callback(void (*cb)(void))
{
    tlk_plic_sw_callback = cb;
}
/* ==== DRIVER APIs ==== */

/* ==== IRQ HELPERS ==== */
_tlk_attribute_ram_code_sec_ void tlk_plic_sw_handle(void)
{
#if TLK_IS_ENABLED(CONFIG_TLK_PLIC_SW_NEST_SELECTED)
    /* save CSRs to stack */
    tlk_core_save_nested_context();
    /* send the claim message to PLIC */
    tlk_plic_sw_interrupt_claim();
    /* set global interupts */
    tlk_write_csr(NDS_MIE, TLK_PLIC_SW_MIE_MASK & tlk_read_csr(NDS_MIE));
    /* enable global interupts */
    tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);

    /* call the handler */
    if (tlk_plic_sw_callback)
        tlk_plic_sw_callback();

    /* send the complete message to PLIC */
    tlk_plic_sw_interrupt_complete();
    /* disable global interrupts before critical section */
    tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* restore CSRs */
    tlk_core_restore_nested_context();
#else
    tlk_plic_sw_interrupt_claim();
    if (tlk_plic_sw_callback)
        tlk_plic_sw_callback();
    tlk_plic_sw_interrupt_complete();
#endif
}
/* ==== IRQ HELPERS ==== */
