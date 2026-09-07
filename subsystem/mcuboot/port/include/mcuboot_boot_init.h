#ifndef TLK_MCUBOOT_BOOT_INIT_H
#define TLK_MCUBOOT_BOOT_INIT_H

#include <stdbool.h>
#include <stdint.h>

// Must be called once before boot_go()
int32_t tlk_mcuboot_crypto_init(void);

// True while the bootloader still trusts the sdk dev key
bool tlk_mcuboot_using_dev_key(void);

#endif /* TLK_MCUBOOT_BOOT_INIT_H */
