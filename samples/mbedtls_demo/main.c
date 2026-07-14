#include "core/include/tlk_core.h"
#include "system/debug/log/tlk_log.h"

#if defined(__GNUC__) && (__GNUC__ >= 14)
#pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

#include <mbedtls/build_info.h>
#include <psa/crypto.h>

TLK_LOG_CREATE(mbedtls_demo, "MBEDTLS_DEMO");

int main(void)
{
    /* Magic adjustment to start binary (commenting next line) */
    // tlk_core_interrupt_enable();

    TLK_LOG_INFO(mbedtls_demo, "version: %s", MBEDTLS_VERSION_STRING_FULL);

    if (psa_crypto_init() == PSA_SUCCESS)
    {

        uint8_t buf[32];

        if (psa_generate_random(buf, sizeof(buf)) == PSA_SUCCESS)
        {
            TLK_LOG_INFO(mbedtls_demo, "random[%zu]", sizeof(buf));
            for (size_t i = 0; i < sizeof(buf); ++i)
            {
                TLK_LOG_INFO(mbedtls_demo, " %02x", buf[i]);
            }
        }
        else
        {
            TLK_LOG_ERROR(mbedtls_demo, "psa_generate_random failed");
        }
    }
    else
    {
        TLK_LOG_ERROR(mbedtls_demo, "psa_crypto_init failed");
    }

    return 0;
}

/* TODO: Dummy stub, just to compile now, should use trng driver */
#include <mbedtls/platform.h>
#include <string.h>

int mbedtls_platform_get_entropy(psa_driver_get_entropy_flags_t flags, size_t* estimate_bits,
                                 unsigned char* output, size_t output_size)
{
    (void) flags;

    memset(output, 0, output_size);
    *estimate_bits = output_size * 8;
    return 0;
}
