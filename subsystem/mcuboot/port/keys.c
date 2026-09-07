#include "bootutil/sign_key.h"

#include <stdbool.h>

/*
 * DEVELOPMENT KEY -- NOT FOR PRODUCTION. Public half of a throwaway EC256 pair whose private
 * half ships with the SDK docs; any image signed with it is accepted by this bootloader, by
 * anyone. Before release, generate your own pair and replace the bytes below:
 *
 *     imgtool keygen -k my-release-key.pem -t ecdsa-p256
 *     imgtool getpub -k my-release-key.pem
 *
 * then set CONFIG_TLK_MCUBOOT_PRODUCTION_KEY (*.pem is git-ignored on purpose).
 */
static const uint8_t mcuboot_dev_ec256_pub_key[] = {
    0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x24, 0x8d, 0xc0, 0x43, 0x08,
    0xbb, 0xca, 0x89, 0x72, 0x68, 0xf4, 0x26, 0xab, 0x98, 0x28, 0xdd, 0x28, 0xac, 0x2e, 0xee, 0x9b,
    0x11, 0xea, 0xaa, 0x0d, 0x72, 0x89, 0x65, 0xcf, 0xbe, 0xa7, 0xff, 0x7a, 0x61, 0xac, 0xf4, 0xfe,
    0xfb, 0x19, 0xcf, 0x4f, 0x01, 0xcb, 0x58, 0xc0, 0xb2, 0x9f, 0xe3, 0x73, 0x3a, 0xb8, 0x76, 0x5f,
    0x29, 0xbc, 0xf9, 0x03, 0x46, 0x81, 0x13, 0x31, 0x69, 0x84, 0xa6,
};

static const unsigned int mcuboot_dev_ec256_pub_key_len = sizeof(mcuboot_dev_ec256_pub_key);

const struct bootutil_key bootutil_keys[] = {
    {
        .key = mcuboot_dev_ec256_pub_key,
        .len = &mcuboot_dev_ec256_pub_key_len,
    },
};

const int bootutil_key_cnt = 1;

#if !defined(CONFIG_TLK_MCUBOOT_PRODUCTION_KEY)
/* #pragma message, not #warning: this SDK builds with -Werror, which would turn a warning into
 * a hard failure and break the out-of-the-box build. Same approach as modules/mbedtls. */
#pragma message                                                                                    \
    "MCUboot is using the SDK development signing key -- images signed by anyone will boot. Replace it before release (see keys.c) and set CONFIG_TLK_MCUBOOT_PRODUCTION_KEY."
#endif

bool tlk_mcuboot_using_dev_key(void)
{
#if defined(CONFIG_TLK_MCUBOOT_PRODUCTION_KEY)
    return false;
#else
    return true;
#endif
}
