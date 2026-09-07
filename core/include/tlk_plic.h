#ifndef TLK_INCLUDE_DRIVERS_PLIC_H_
#define TLK_INCLUDE_DRIVERS_PLIC_H_

#include "core/include/tlk_core.h"
#include <stdbool.h>

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

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC_NEST_PLIC)

/**
 * @brief Enable PLIC interrupt preemption feature.
 *
 * @return None.
 */
void tlk_plic_enable_preempt(void);

/**
 * @brief Disable PLIC interrupt preemption feature.
 *
 * @return None.
 */
void tlk_plic_disable_preempt(void);

#endif

/**
 * @brief Manually sets an interrupt source as pending.
 *
 * @param src The interrupt source ID.
 *
 * @return None.
 */
void tlk_plic_set_pending(uint32_t src);

/**
 * @brief Clears all pending interrupt requests.
 *
 * @return int32_t Status code (1 for success).
 */
int32_t tlk_plic_clr_all_request(void);

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
void tlk_plic_preprocess_irqs_for_wfi(uint8_t flag, tlk_mie_e mie);

/**
 * @brief Restores the PLIC state after WFI.
 *
 * Restores the PLIC state and interrupt configuration after the system wakes up
 * from WFI or sleep.
 *
 * @return None.
 */
void tlk_plic_postprocess_irqs_for_wfi(void);

bool tlk_plic_is_in_isr(void);

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC)
extern volatile uint8_t tlk_plic_nesting_counter;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC_VECTOR_MODE)
extern void tlk_plic_isr(tlk_func_isr_t func, uint32_t src);
#endif

#define TLK_ISR_ENTRY_NAME(irq_num) entry_irq##irq_num
#define TLK_PLIC_ISR_REGISTER(isr, irq_num)                                                        \
    TLK_IF_ENABLED(CONFIG_TLK_PLIC_VECTOR_MODE,                                                    \
                   (__attribute__((interrupt("machine"), aligned(4)))))                            \
    _tlk_attribute_ram_code_sec_noinline_ void TLK_ISR_ENTRY_NAME(irq_num)(void)                   \
    {                                                                                              \
        TLK_IF_ENABLED(CONFIG_TLK_PLIC, (tlk_plic_nesting_counter++;))                             \
        TLK_IF_ENABLED_ELSE(CONFIG_TLK_PLIC_VECTOR_MODE, (tlk_plic_isr(isr, irq_num);), (isr();))  \
        TLK_IF_ENABLED(CONFIG_TLK_PLIC, (tlk_plic_nesting_counter--;))                             \
    }

#endif
