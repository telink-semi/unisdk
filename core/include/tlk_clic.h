#ifndef TLK_INCLUDE_DRIVERS_CLIC_H_
#define TLK_INCLUDE_DRIVERS_CLIC_H_

#include "tlk_core.h"

#define __attribute_trap_entry_  __attribute__((section(".ram_code"))) __attribute__((noinline)) \
                                __attribute__((interrupt("machine"))) __attribute__((aligned(4))) \
                                __attribute__((weak))

/**
 * @brief Enable specific interrupt source 
 */
void tlk_clic_enable_interrupt_source(tlk_mie_e source);
/**
 * @brief Disable specific interrupt source 
 */
void tlk_clic_disable_interrupt_source(tlk_mie_e source);

#endif
