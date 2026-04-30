#include "core/include/tlk_plic.h"
#include "registers/tlk_plic.h"
#include "properties/tlk_plic.h"

uint32_t tlk_g_irq_src0_value = 0;
uint32_t tlk_g_irq_src1_value = 0;
uint32_t tlk_g_mie_value = 0;
uint32_t tlk_g_mstatus_value = 0;
uint32_t tlk_g_wfi_disable_int = 0;

volatile uint8_t nesting_counter = 0;

void tlk_plic_all_interrupt_save_and_disable(void)
{
    /* save enabled PLIC irq and disable. */
    tlk_g_irq_src0_value = tlk_plic_reg.irq_en_l;
    tlk_g_irq_src1_value = tlk_plic_reg.irq_en_h;

    tlk_plic_reg.irq_en_l= 0;
    tlk_plic_reg.irq_en_h = 0;
}

void tlk_plic_all_interrupt_restore(void)
{
    /* restore PLIC irq. */
    tlk_plic_reg.irq_en_l = tlk_g_irq_src0_value;
    tlk_plic_reg.irq_en_h = tlk_g_irq_src1_value;
}

_tlk_always_inline _tlk_attribute_ram_code_sec_ void tlk_plic_interrupt_enable(uint32_t src)
{
    (src <= 31) ? (tlk_plic_reg.irq_en_l |= TLK_BIT(src % 32)) : (tlk_plic_reg.irq_en_h |= TLK_BIT(src % 32));
}

_tlk_always_inline _tlk_attribute_ram_code_sec_ void tlk_plic_interrupt_disable(uint32_t src)
{
    (src <= 31) ? (tlk_plic_reg.irq_en_l &= (~TLK_BIT(src % 32)) ) : (tlk_plic_reg.irq_en_h &= (~TLK_BIT(src % 32)) );
}

_tlk_attribute_ram_code_sec_ int32_t tlk_plic_clr_all_request(void)
{
    uint32_t claim_cnt = 0;
    uint32_t cur_claim = 0;

    /**
     * when global interrupts are disabled, software needs to ensure that all interrupts have been claimed and completed, \n
     * When global interrupt enable, hardware handles the claim, complete is handled in the tlk_plic_isr function.
     */
    do {
        cur_claim = tlk_plic_interrupt_claim();
        tlk_plic_interrupt_complete(cur_claim);
        claim_cnt++;
    } while ((cur_claim != 0) && (claim_cnt <= (UNISDK_PLIC_IRQ_COUNT - 1) * 2));

    /* Clearing all PLIC interrupt requests fails, returning an error code to notify the application program. */
    if (claim_cnt > (UNISDK_PLIC_IRQ_COUNT - 1) * 2) {
        return 0;
    }

    return 1;
}

_tlk_always_inline void tlk_plic_set_priority(uint32_t src, enum tlk_irq_priority priority)
{
    tlk_plic_reg.irq_priority[src] = priority;
}

_tlk_always_inline void tlk_plic_set_threshold(enum tlk_irq_priority threshold)
{
    uint32_t r = tlk_core_interrupt_disable();
    tlk_plic_reg.irq_threshold = threshold;
    tlk_fence_iorw; /* Hardware may change this value, fence IO ensures that software changes are valid. */
    tlk_core_restore_interrupt(r);
}

_tlk_always_inline void tlk_plic_set_pending(uint32_t src)
{
    (src <= 31) ? (tlk_plic_reg.irq_pending_l = TLK_BIT(src % 32)) : (tlk_plic_reg.irq_pending_h = TLK_BIT(src % 32));
}

_tlk_always_inline uint32_t tlk_plic_interrupt_claim(void)
{
    return tlk_plic_reg.irq_claim_compl;
}

_tlk_always_inline void tlk_plic_interrupt_complete(uint32_t src)
{
    tlk_plic_reg.irq_claim_compl = src;
}

void tlk_plic_irqs_preprocess_for_wfi(uint8_t flag, tlk_mie_e mie)
{
    if (flag) {
        tlk_g_wfi_disable_int = 1;
        tlk_g_mstatus_value   = tlk_core_interrupt_disable();
    } else {
        tlk_g_wfi_disable_int = 0;
    }

    tlk_plic_all_interrupt_save_and_disable();
    tlk_g_mie_value = tlk_core_mie_disable(TLK_FLD_MIE_MSIE | TLK_FLD_MIE_MTIE | TLK_FLD_MIE_MEIE);
    tlk_core_mie_enable(mie);
}

void tlk_plic_irqs_postprocess_for_wfi(void)
{
    if (tlk_g_wfi_disable_int) {
        /* When global interrupt is disabled, the application needs to call this interface to ensure that the corresponding PLIC interrupt requests have been processed. */
        tlk_plic_clr_all_request();
    }

    tlk_plic_all_interrupt_restore();
    tlk_core_mie_restore(tlk_g_mie_value);

    if (tlk_g_wfi_disable_int) {
        tlk_core_restore_interrupt(tlk_g_mstatus_value);
    }
}

bool tlk_plic_is_in_isr(void)
{
    return nesting_counter > 0 ? true : false;
}

_tlk_attribute_ram_code_sec_ __attribute__((always_inline)) inline void tlk_plic_isr(tlk_func_isr_t func, uint32_t src)
{
    /**
     * Adding always_inline modifier function is for code reduction and better real-time performance.
     * - if not add always_inline, entry_irqX function in the call tlk_plic_isr, the compiler may be in accordance with the function jump processing, entry_irqX function if there is a function jump, \n
     *   see the compiler's processing is: all the general-purpose registers of the mcu are in the stack and out of the stack protection.
     * - if you add always_inline, and the user's interrupt handling function does not have a complex function call, the compiler can do, only the general-purpose registers used by the program for the stack and out of the stack protection.
     */
#if IS_ENABLED(CONFIG_TLK_PLIC_ENABLE_NESTING)
    /**
        * MEI not interrupted by MSI or MTI handling
        *   -# save MIE register;
        *   -# if MEI cannot be interrupted by MSI and MTI, clear the corresponding bit(The bit corresponding to TLK_CORE_PREEMPT_PRI_MODE3 is an invalid bit, \n
        *      clearing the corresponding bit will not affect the mie function, the purpose of doing this is to reduce the code judgment);
        *   -# restore MIE register.
        */
    /* save CSRs to stack;*/
    tlk_core_save_nested_context();
    /* enable global interupt */
    tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* irq handler */
    func();
    /* complete interrupt */
    tlk_plic_interrupt_complete(src);
    /* disable global interrupts before critical section */
    tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* restore CSRs */
    tlk_core_restore_nested_context();
    /**
        * Fence IO to avoid this competing state of interrupt completion and interrupt claim occurring at the same time. \n
        * PLIC is required to ensure that a complete message for the previous interrupt has reached the PLIC before sending the interrupt claim.
        */
    tlk_fence_iorw;
#else
    func();                           /* irq handler */
    tlk_plic_interrupt_complete(src); /* complete interrupt */
#endif
}

#if IS_DISABLED(CONFIG_TLK_PLIC_VECTOR_MODE)

typedef void (*tlk_irq_handler)(void);
extern tlk_irq_handler __tlk_vectors[UNISDK_PLIC_IRQ_COUNT];

_tlk_attribute_ram_code_sec_ void tlk_plic_handler(void)
{
#if IS_ENABLED(CONFIG_TLK_PLIC_ENABLE_NESTING)
    /* save CSRs to stack;*/
    tlk_core_save_nested_context();
    /* send the claim message to PLIC */
    uint32_t intr = tlk_plic_interrupt_claim();
    /* enable global interupt */
    tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* call the handler */ 
    __tlk_vectors[intr]();
    /* send the complete message to PLIC */
    tlk_plic_interrupt_complete(intr);
    /* disable global interrupts before critical section */
    tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
    /* restore CSRs */
    tlk_core_restore_nested_context();
#else
    /* send the claim message to PLIC */
    uint32_t intr = tlk_plic_interrupt_claim();
    /* call the handler */ 
    __tlk_vectors[intr]();
    /* send the complete message to PLIC */
    tlk_plic_interrupt_complete(intr);
#endif
}

#endif
