#include "core/include/tlk_random.h"
#include "common/include/tlk_init.h"
#include "common/include/tlk_utils.h"
#include "crypto/include/trng/trng.h"
#include "crypto/include/trng/trng_portable.h"
#include "properties/tlk_random.h"
#include "registers/tlk_trng_reg.h"

/* ==== HELPERS ==== */
#ifdef CONFIG_TLK_RANDOM_USED_TRNG

#define TLK_RANDOM_ADD_SOURCE_TLK_IF_ENABLED(i)                                                    \
    TLK_IF_ENABLED(CONFIG_TLK_RANDOM_TRNG_SOURCE##i, (| FLD_TRNG_CR0_ROSEN##i))

#define TLK_RANDOM_GET_ENABLED_SOURCES                                                             \
    (0 TLK_FOR_CALL(UNISDK_TRNG_SOURCE_COUNT, TLK_RANDOM_ADD_SOURCE_TLK_IF_ENABLED))

#define TLK_RANDOM_ADD_SOURCE0_RO_TLK_IF_ENABLED(i)                                                \
    TLK_IF_ENABLED(CONFIG_TLK_RANDOM_TRNG_SOURCE0_RO##i, (| TLK_BIT(i)))

#define TLK_RANDOM_ADD_SOURCE1_RO_TLK_IF_ENABLED(i)                                                \
    TLK_IF_ENABLED(CONFIG_TLK_RANDOM_TRNG_SOURCE1_RO##i, (| TLK_BIT(i)))

#define TLK_RANDOM_ADD_SOURCE2_RO_TLK_IF_ENABLED(i)                                                \
    TLK_IF_ENABLED(CONFIG_TLK_RANDOM_TRNG_SOURCE2_RO##i, (| TLK_BIT(i)))

#define TLK_RANDOM_ADD_SOURCE3_RO_TLK_IF_ENABLED(i)                                                \
    TLK_IF_ENABLED(CONFIG_TLK_RANDOM_TRNG_SOURCE3_RO##i, (| TLK_BIT(i)))

#define TLK_RANDOM_GET_ENABLED_SOURCE_ROS(i)                                                       \
    (0 TLK_FOR_CALL(UNISDK_TRNG_RO_COUNT, TLK_RANDOM_ADD_SOURCE##i##_RO_TLK_IF_ENABLED))

static void tlk_random_init(void)
{
#ifdef CONFIG_TLK_RANDOM_PRNG_TRNG
    trng_init();
#else
    trng_dig_en();
    trng_disable();
    trng_set_error_timeout(TLK_MS_TO_US(CONFIG_TLK_RANDOM_TRNG_TIMEOUT));

    trng_ro_entropy_config(TLK_RANDOM_GET_ENABLED_SOURCES >> 4);
    trng_ro_sub_entropy_config(0, TLK_RANDOM_GET_ENABLED_SOURCE_ROS(0));
    trng_ro_sub_entropy_config(1, TLK_RANDOM_GET_ENABLED_SOURCE_ROS(1));
    trng_ro_sub_entropy_config(2, TLK_RANDOM_GET_ENABLED_SOURCE_ROS(2));
    trng_ro_sub_entropy_config(3, TLK_RANDOM_GET_ENABLED_SOURCE_ROS(3));

    trng_global_int_enable();
    trng_data_int_enable();
    trng_empty_read_int_disable();

    trng_set_mode(TLK_IF_ENABLED_ELSE(CONFIG_TLK_RANDOM_TRNG_DRBG, (1), (0)));
    trng_set_freq(CONFIG_TLK_RANDOM_TRNG_FSEL);

#ifdef CONFIG_TLK_RANDOM_TRNG_DRBG
    // investigation showed that if the tlk_trng_extended_reg.control.skip_startup bit is TRUE, the
    // DRBG always returns the same numbers after each reset
    trng_skip_startup_disable();
#endif

    trng_enable();
#endif
}

TLK_REGISTER_PRE_INIT(tlk_random_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_LOW)

#ifdef CONFIG_TLK_RANDOM_PM

TLK_REGISTER_AFTER_SUSPEND(tlk_random_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_LOW)
TLK_REGISTER_AFTER_SLEEP(tlk_random_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_LOW)

#endif
#endif
/* ==== HELPERS ==== */

/* ==== DRIVER APIs ==== */
_tlk_attribute_ram_code_sec_ uint32_t tlk_random(void)
{
#ifdef CONFIG_TLK_RANDOM_USED_PRNG
    return trng_rand();
#else
    uint32_t random_number = 0;

    if (get_rand_internal((unsigned char*) &random_number, 4) == TRNG_HT_ERROR)
    {
        // If the HTF occurred (from the datasheet):
        // The CPU can clear this interrupt by writing 1 to register TRNG_SR.HTF. After clearing,
        // the TRNG still does not work, it is needed to write 0 to register TRNG_CR.RNGEN to stop
        // it, reconfigure it and start it again. The TRNG can also be reset globally to make it
        // work again.
        trng_disable();
        trng_reseed(); // This API will make: { TRNG_SR |= 0x07; // write 1 to clear }
        trng_enable();
    }

    return random_number;
#endif
}
/* ==== DRIVER APIs ==== */
