#ifndef TLK_INCLUDE_DRIVERS_PLIC_H_
#define TLK_INCLUDE_DRIVERS_PLIC_H_

#include "tlk_core.h"
#include <stdbool.h>


extern volatile uint8_t nesting_counter;

enum tlk_irq_priority
{
    TLK_IRQ_PRI_LEV0, /* 0 indicates that no interrupt is generated. */
    TLK_IRQ_PRI_LEV1,
    TLK_IRQ_PRI_LEV2,
    TLK_IRQ_PRI_LEV3,
};

typedef void (*tlk_func_isr_t)(void);

/**
 * @brief Enables a specific interrupt source.
 *
 * @param src The interrupt source ID to be enabled.
 * 
 * @return None.
 */
void tlk_plic_interrupt_enable(uint32_t src);

/**
 * @brief Disables a specific interrupt source.
 *
 * @param src The interrupt source ID to be disabled.
 * 
 * @return None.
 */
void tlk_plic_interrupt_disable(uint32_t src);

/**
 * @brief Clears all pending interrupt requests.
 *
 * @return int32_t Status code (1 for success).
 */
_tlk_attribute_ram_code_sec_noinline_ int32_t tlk_plic_clr_all_request(void);

/**
 * @brief Sets the priority level for a specific interrupt source.
 *
 * Higher numbers indicate higher priority.
 *
 * @param src The interrupt source ID.
 * @param priority The priority level to set.
 * 
 * @return None.
 */
void tlk_plic_set_priority(uint32_t src, enum tlk_irq_priority priority);

/**
 * @brief Sets the global interrupt threshold.
 *
 * Only interrupts with a priority strictly higher than this threshold will be serviced.
 *
 * @param threshold The minimum priority threshold.
 * 
 * @return None.
 */
void tlk_plic_set_threshold(enum tlk_irq_priority threshold);

/**
 * @brief Manually sets an interrupt source as pending.
 *
 * @param src The interrupt source ID.
 * 
 * @return None.
 */
void tlk_plic_set_pending(uint32_t src);

uint32_t tlk_plic_interrupt_claim(void);

void tlk_plic_interrupt_complete(uint32_t src);

/**
 * @brief Prepares the PLIC controller for WFI (Wait For Interrupt).
 *
 * Prepares the PLIC controller and interrupt state before the system enters
 * a "Wait For Interrupt" (WFI) or sleep state.
 *
 * @param flag Specific flag for wakeup configuration.
 * @param mie Machine Interrupt Enable state to preserve.
 * 
 * @return None.
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_plic_irqs_preprocess_for_wfi(uint8_t flag, tlk_mie_e mie);

/**
 * @brief Restores the PLIC state after WFI.
 *
 * Restores the PLIC state and interrupt configuration after the system wakes up
 * from WFI or sleep.
 * 
 * @return None.
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_plic_irqs_postprocess_for_wfi(void);

/**
 * @brief Restores the PLIC state after WFI.
 *
 * Restores the PLIC state and interrupt configuration after the system wakes up
 * from WFI or sleep.
 * 
 * @return None.
 */
_tlk_attribute_ram_code_sec_noinline_ bool tlk_plic_is_in_isr(void);

/**
 * @brief The wrapper for calling of the ISR in vector mode.
 * 
 * @param func  ISR to be called.
 * @param src   Interrupt number.
 * @return      None.
 */
_tlk_attribute_ram_code_sec_ void tlk_plic_isr(tlk_func_isr_t func, uint32_t src);

#define TLK_ISR_ENTRY_NAME(irq_num) entry_irq##irq_num
#define TLK_PLIC_ISR_REGISTER(isr, irq_num) \
    IF_ENABLED(CONFIG_TLK_PLIC_VECTOR_MODE, (__attribute__((interrupt("machine"), aligned(4))))) \
    _tlk_attribute_ram_code_sec_noinline_ void TLK_ISR_ENTRY_NAME(irq_num)(void); \
    void                                   TLK_ISR_ENTRY_NAME(irq_num)(void) \
    { \
        IF_ENABLED(CONFIG_TLK_PLIC, (nesting_counter++;)) \
        IF_ENABLED_ELSE(CONFIG_TLK_PLIC_VECTOR_MODE, \
            (tlk_plic_isr(isr, irq_num);), \
            (isr();) \
        ) \
        IF_ENABLED(CONFIG_TLK_PLIC, (nesting_counter--;)) \
    }

#endif
