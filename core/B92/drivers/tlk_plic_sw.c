#include "core/include/tlk_plic_sw.h"
#include "tlk_core.h"
#include "registers/tlk_plic_sw.h"

#define PLIC_SW_IRQ_NUM 1 //TODO: move to yaml

_tlk_always_inline void tlk_plic_sw_set_pending(void)
{
    tlk_plic_sw_reg.irq_pending = TLK_BIT(PLIC_SW_IRQ_NUM);
}

_tlk_always_inline void tlk_plic_sw_interrupt_enable(void)
{
    /* Enable interrupts in PLIC SW */
    tlk_plic_sw_reg.irq_en |= TLK_BIT(PLIC_SW_IRQ_NUM);

    /* Enable PLIC SW interrupt source in CLIC */
    tlk_set_csr(NDS_MIE, TLK_FLD_MIE_MSIE);
}

_tlk_always_inline void tlk_plic_sw_interrupt_disable(void)
{
    /* Disable interrupts in PLIC SW */
    tlk_plic_sw_reg.irq_en &= (~TLK_BIT(PLIC_SW_IRQ_NUM));

    /* Disable PLIC SW interrupt source in CLIC */
    tlk_clear_csr(NDS_MIE, TLK_FLD_MIE_MSIE);
}

_tlk_always_inline uint32_t tlk_plic_sw_interrupt_claim(void)
{
    return tlk_plic_sw_reg.irq_claim_compl;
}

_tlk_always_inline void tlk_plic_sw_interrupt_complete(void)
{
    tlk_plic_sw_reg.irq_claim_compl = PLIC_SW_IRQ_NUM;
}

/**
 * @brief     Default swi irq handler.
 * @return    None.
 */
__attribute__((weak)) void tlk_mswi_irq_handler(void)
{
    tlk_plic_sw_interrupt_complete();
}

_tlk_attribute_ram_code_sec_ void tlk_plic_sw_handler(void)
{
#if IS_ENABLED(CONFIG_TLK_PLIC_SW_ENABLE_NESTING)
    /* save CSRs to stack;*/
    tlk_core_save_nested_context();
    /* send the claim message to PLIC */
    tlk_plic_sw_interrupt_claim();
    /* enable global interupt */
    tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* call the handler */
    tlk_mswi_irq_handler();
    /* send the complete message to PLIC */
    tlk_plic_sw_interrupt_complete();
    /* disable global interrupts before critical section */
    tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* restore CSRs */
    tlk_core_restore_nested_context();
#else
    tlk_plic_sw_interrupt_claim();
    tlk_mswi_irq_handler();
    tlk_plic_sw_interrupt_complete();
#endif
}
