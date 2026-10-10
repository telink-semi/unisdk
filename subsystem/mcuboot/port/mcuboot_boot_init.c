#include "mcuboot_boot_init.h"

#include <psa/crypto.h>

int32_t tlk_mcuboot_crypto_init(void)
{
    return (int32_t) psa_crypto_init();
}
