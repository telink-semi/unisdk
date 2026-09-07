#include "core/include/tlk_random.h"
#include "common/include/tlk_init.h"
#include "common/include/tlk_utils.h"
#include "crypto/include/trng.h"
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
    trng_hw_enable();
    trng_disable();
    trng_set_error_timeout(TLK_MS_TO_US(CONFIG_TLK_RANDOM_TRNG_TIMEOUT));

    trng_enable_sources(TLK_RANDOM_GET_ENABLED_SOURCES);
    trng_enable_ros(0, TLK_RANDOM_GET_ENABLED_SOURCE_ROS(0));
    trng_enable_ros(1, TLK_RANDOM_GET_ENABLED_SOURCE_ROS(1));
    trng_enable_ros(2, TLK_RANDOM_GET_ENABLED_SOURCE_ROS(2));
    trng_enable_ros(3, TLK_RANDOM_GET_ENABLED_SOURCE_ROS(3));
    trng_set_mode(TLK_IF_ENABLED_ELSE(CONFIG_TLK_RANDOM_TRNG_DRBG, (1), (0)));
    trng_set_fsel(CONFIG_TLK_RANDOM_TRNG_FSEL);

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
    return trng_get_data();
#endif
}
/* ==== DRIVER APIs ==== */
