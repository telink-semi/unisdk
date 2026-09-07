#include "core/include/tlk_i2c.h"
#include "common/include/tlk_init.h"

#if TLK_IS_ENABLED(CONFIG_TLK_I2C_AUTO_CONFIG)

#define TLK_DECLARE_I2C_AUTO_CONFIG(num)                                                           \
    __attribute__((used, noinline)) static void tlk_i2c##num##_auto_config(void)                   \
    {                                                                                              \
        struct tlk_i2c_pinmux_config pinmux = {                                                    \
            .sda_port_pin =                                                                        \
                {                                                                                  \
                    .port = PINMUX_I2C##num##_SDA_PORT,                                            \
                    .pin  = PINMUX_I2C##num##_SDA_PIN,                                             \
                },                                                                                 \
            .scl_port_pin =                                                                        \
                {                                                                                  \
                    .port = PINMUX_I2C##num##_SCL_PORT,                                            \
                    .pin  = PINMUX_I2C##num##_SCL_PIN,                                             \
                },                                                                                 \
        };                                                                                         \
                                                                                                   \
        tlk_i2c_pinmux_configure(&tlk_i2c##num, &pinmux);                                          \
                                                                                                   \
        TLK_IF_ENABLED(CONFIG_TLK_I2C##num##_AC_MODE_MASTER,                                       \
                       (struct tlk_i2c_master_config config =                                      \
                            {                                                                      \
                                .stretch_en  = CONFIG_TLK_I2C##num##_AC_STRETCH,                   \
                                .clock_speed = CONFIG_TLK_I2C##num##_AC_CLOCK_SPEED,               \
                            };                                                                     \
                                                                                                   \
                        tlk_i2c_master_configure(&tlk_i2c##num, &config);))                        \
        TLK_IF_ENABLED(CONFIG_TLK_I2C##num##_AC_MODE_SLAVE,                                        \
                       (struct tlk_i2c_slave_config config =                                       \
                            {                                                                      \
                                .stretch_en = CONFIG_TLK_I2C##num##_AC_STRETCH,                    \
                                .address    = CONFIG_TLK_I2C##num##_AC_ADDRESS,                    \
                            };                                                                     \
                                                                                                   \
                        tlk_i2c_slave_configure(&tlk_i2c##num, &config);))                         \
    }                                                                                              \
    TLK_REGISTER_PRE_INIT(                                                                         \
        tlk_i2c##num##_auto_config, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#define TLK_DECLARE_I2C_AUTO_CONFIG_IF_ENABLED(num)                                                \
    TLK_IF_ENABLED(CONFIG_TLK_I2C##num##_AUTO_CONFIG, (TLK_DECLARE_I2C_AUTO_CONFIG(num)))

TLK_FOR_CALL(UNISDK_I2C_COUNT, TLK_DECLARE_I2C_AUTO_CONFIG_IF_ENABLED)

#endif
