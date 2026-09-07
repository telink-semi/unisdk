#include "core/include/tlk_spi.h"
#include "common/include/tlk_init.h"

#if TLK_IS_ENABLED(CONFIG_TLK_SPI_AUTO_CONFIG)

#define TLK_DECLARE_SPI_AUTO_CONFIG(num)                                                           \
    __attribute__((used, noinline)) static void tlk_spi##num##_auto_config(void)                   \
    {                                                                                              \
        struct tlk_spi_pinmux pinmux = {                                                           \
            .cs   = {PINMUX_SPI##num##_CS_PORT, PINMUX_SPI##num##_CS_PIN},                         \
            .sck  = {PINMUX_SPI##num##_SCK_PORT, PINMUX_SPI##num##_SCK_PIN},                       \
            .mosi = {PINMUX_SPI##num##_MOSI_PORT, PINMUX_SPI##num##_MOSI_PIN},                     \
            .miso = {PINMUX_SPI##num##_MISO_PORT, PINMUX_SPI##num##_MISO_PIN},                     \
        };                                                                                         \
                                                                                                   \
        tlk_spi_pinmux_configure(&tlk_spi##num, &pinmux);                                          \
                                                                                                   \
        struct tlk_spi_config config = {.mode      = CONFIG_TLK_SPI##num##_AC_MODE,                \
                                        .io_mode   = CONFIG_TLK_SPI##num##_AC_IO_MODE,             \
                                        .frequency = CONFIG_TLK_SPI##num##_AC_FREQUENCY};          \
                                                                                                   \
        TLK_IF_ENABLED(CONFIG_TLK_SPI##num##_AC_ROLE_MASTER,                                       \
                       (tlk_spi_master_configure(&tlk_spi##num, &config);))                        \
                                                                                                   \
        TLK_IF_ENABLED(CONFIG_TLK_SPI##num##_AC_ROLE_SLAVE,                                        \
                       (tlk_spi_slave_configure(&tlk_spi##num, &config);))                         \
    }                                                                                              \
    TLK_REGISTER_PRE_INIT(                                                                         \
        tlk_spi##num##_auto_config, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#define TLK_DECLARE_SPI_AUTO_CONFIG_IF_ENABLED(num)                                                \
    TLK_IF_ENABLED(CONFIG_TLK_SPI##num##_AUTO_CONFIG, (TLK_DECLARE_SPI_AUTO_CONFIG(num)))

TLK_FOR_CALL(UNISDK_SPI_COUNT, TLK_DECLARE_SPI_AUTO_CONFIG_IF_ENABLED)

#endif
