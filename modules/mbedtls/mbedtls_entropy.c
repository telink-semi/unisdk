#include <mbedtls/platform.h>

#if TLK_IS_ENABLED(CONFIG_TLK_CRYPTO_TRNG)

#if (defined(CONFIG_TLK_SOC_SERIES_TLSR952X) && CONFIG_TLK_SOC_SERIES_TLSR952X)
#include "crypto/include/trng.h"
#else
#include "crypto/include/trng/trng_portable.h"
#endif /* CONFIG_TLK_SOC_SERIES_* */

extern void tlk_mbedtls_entropy_lock(void);
extern void tlk_mbedtls_entropy_unlock(void);

int mbedtls_platform_get_entropy(psa_driver_get_entropy_flags_t flags, size_t* estimate_bits,
                                 unsigned char* output, size_t output_size)
{
    (void) flags;
    static volatile bool mbedtls_entropy_inited;
    size_t               output_cnt     = output_size / sizeof(uint32_t);
    size_t               output_cnt_rem = output_size - output_cnt * sizeof(uint32_t);

    if (!mbedtls_entropy_inited)
    {
        tlk_mbedtls_entropy_lock();
        if (!mbedtls_entropy_inited)
        {
            (void) trng_init();
            mbedtls_entropy_inited = true;
        }
        tlk_mbedtls_entropy_unlock();
    }
    tlk_mbedtls_entropy_lock();
    for (size_t i = 0; i < output_cnt; ++i)
    {
        *(uint32_t*) output = trng_rand();
        output += sizeof(uint32_t);
    }
    if (output_cnt_rem)
    {
        uint32_t rnd = trng_rand();
        for (size_t i = 0; i < output_cnt_rem; ++i)
        {
            *output = rnd;
            output++;
            rnd >>= 8;
        }
    }
    *estimate_bits = output_size * 8;
    tlk_mbedtls_entropy_unlock();
    return 0;
}

#else

#pragma message                                                                                    \
    "Entropy not reliable! (Implement your mbedtls_platform_get_entropy platform function)"

#include <core/include/tlk_clock.h>
#include <stdlib.h>

__attribute__((weak)) int mbedtls_platform_get_entropy(psa_driver_get_entropy_flags_t flags,
                                                       size_t* estimate_bits, unsigned char* output,
                                                       size_t output_size)
{
    (void) flags;
    static bool mbedtls_entropy_inited;
    size_t      output_cnt     = output_size / sizeof(uint32_t);
    size_t      output_cnt_rem = output_size - output_cnt * sizeof(uint32_t);

    if (!mbedtls_entropy_inited)
    {
        srand(tlk_clock_cclk_get_tick());
        mbedtls_entropy_inited = true;
    }
    for (size_t i = 0; i < output_cnt; ++i)
    {
        *(uint32_t*) output = rand();
        output += sizeof(uint32_t);
    }
    if (output_cnt_rem)
    {
        uint32_t rnd = rand();
        for (size_t i = 0; i < output_cnt_rem; ++i)
        {
            *output = rnd;
            output++;
            rnd >>= 8;
        }
    }
    *estimate_bits = output_size * 8;
    return 0;
}

#endif /* CONFIG_TLK_CRYPTO_TRNG */
