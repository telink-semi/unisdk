#include "common/include/tlk_str_utils.h"
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
    tlk_core_interrupt_enable();

    TLK_LOG_INFO(mbedtls_demo, "version: %s", MBEDTLS_VERSION_STRING_FULL);

    if (psa_crypto_init() == PSA_SUCCESS)
    {

        bool tests_ok = true;

        do
        {
            {
                TLK_LOG_INFO(mbedtls_demo, "random testing...");

                uint8_t buf[31];
                char    str[sizeof(buf) * 2 + 1];

                if (psa_generate_random(buf, sizeof(buf)) != PSA_SUCCESS)
                {
                    tests_ok = false;
                    TLK_LOG_ERROR(mbedtls_demo, "psa_generate_random failed");
                    break;
                }
                (void) tlk_str_utils_convert_buf_to_hex(str, sizeof(str), buf, sizeof(buf));
                TLK_LOG_INFO(mbedtls_demo, "random[%zu] %s", sizeof(buf), str);
            }
            {
                TLK_LOG_INFO(mbedtls_demo, "EC DSA testing...");

                psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;

                psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
                psa_set_key_bits(&attr, 256);
                psa_set_key_usage_flags(&attr,
                                        PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH |
                                            PSA_KEY_USAGE_EXPORT);
                psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

                psa_key_id_t key;

                if (psa_generate_key(&attr, &key) != PSA_SUCCESS)
                {
                    tests_ok = false;
                    TLK_LOG_ERROR(mbedtls_demo, "psa_generate_key failed");
                    break;
                }

                uint8_t buf[65];
                char    str[sizeof(buf) * 2 + 1];
                size_t  len;

                if (psa_export_key(key, buf, sizeof(buf), &len) != PSA_SUCCESS)
                {
                    psa_destroy_key(key);
                    tests_ok = false;
                    TLK_LOG_ERROR(mbedtls_demo, "psa_export_key failed");
                    break;
                }
                (void) tlk_str_utils_convert_buf_to_hex(str, sizeof(str), buf, len);
                TLK_LOG_INFO(mbedtls_demo, "pri key[%zu] %s", len, str);

                if (psa_export_public_key(key, buf, sizeof(buf), &len) != PSA_SUCCESS)
                {
                    psa_destroy_key(key);
                    tests_ok = false;
                    TLK_LOG_ERROR(mbedtls_demo, "psa_export_public_key failed");
                    break;
                }
                (void) tlk_str_utils_convert_buf_to_hex(str, sizeof(str), buf, len);
                TLK_LOG_INFO(mbedtls_demo, "pub key[%zu] %s", len, str);

                const uint8_t hash[32] = {0};

                if (psa_sign_hash(key,
                                  PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                                  hash,
                                  sizeof(hash),
                                  buf,
                                  sizeof(buf),
                                  &len) != PSA_SUCCESS)
                {
                    psa_destroy_key(key);
                    tests_ok = false;
                    TLK_LOG_ERROR(mbedtls_demo, "psa_sign_hash failed");
                    break;
                }
                (void) tlk_str_utils_convert_buf_to_hex(str, sizeof(str), buf, len);
                TLK_LOG_INFO(mbedtls_demo, "signature[%zu] %s", len, str);

                if (psa_verify_hash(
                        key, PSA_ALG_ECDSA(PSA_ALG_SHA_256), hash, sizeof(hash), buf, len) !=
                    PSA_SUCCESS)
                {
                    psa_destroy_key(key);
                    tests_ok = false;
                    TLK_LOG_ERROR(mbedtls_demo, "psa_verify_hash failed");
                    break;
                }
                TLK_LOG_INFO(mbedtls_demo, "signature OK");

                psa_destroy_key(key);
            }
        } while (0);

        if (tests_ok)
        {
            TLK_LOG_INFO(mbedtls_demo, "All tests success!");
        }
        else
        {
            TLK_LOG_ERROR(mbedtls_demo, "Some tests failed!");
        }
    }
    else
    {
        TLK_LOG_ERROR(mbedtls_demo, "psa_crypto_init failed");
    }

    /* Magic adjustment to start binary (commenting next line) */
    for (;;)
    {
    }

    return 0;
}
