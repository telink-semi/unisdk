#ifndef TLK_INCLUDE_DRIVERS_TRAP_H_
#define TLK_INCLUDE_DRIVERS_TRAP_H_

#define __attribute_trap_entry_                                                                    \
    __attribute__((section(".ram_code"))) __attribute__((noinline))                                \
    __attribute__((interrupt("machine"))) __attribute__((aligned(4))) __attribute__((weak))

/**
 * @brief      Register a callback to be invoked when an ecall exception occurs.
 * @param[in]  cb - pointer to the callback function. Pass NULL to clear.
 * @return     None.
 */
void tlk_trap_register_ecall_callback(void (*cb)(void));

#endif
