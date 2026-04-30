#ifndef TLK_INCLUDE_DRIVERS_PLIC_SW_H_
#define TLK_INCLUDE_DRIVERS_PLIC_SW_H_

/**
 * @brief       This function serves to set software interrupt pending. its means trigger a software interrupt.
 * @return      None.
 */
void tlk_plic_sw_set_pending(void);

/**
 * @brief    This function serves to enable software interrupt.
 * @return   None.
 */
void tlk_plic_sw_interrupt_enable(void);

/**
 * @brief    This function serves to disable software interrupt.
 * @return   None.
 */
void tlk_plic_sw_interrupt_disable(void);

/**
 * @brief    This function serves to claim software interrupt.
 * @return   Software interrupt source id.
 * @note     The claim signal needs to be generated manually in the interrupt service routine.
 */
uint32_t tlk_plic_sw_interrupt_claim(void);

/**
 * @brief    This function serves to send an interrupt complete message to allow a new software interrupt request when current software interrupt done.
 * @return   None.
 * @note     Software interrupts need to be completed before exiting the interrupt service routine to ensure that software interrupts are fully released.
 */
void tlk_plic_sw_interrupt_complete(void);

#endif
