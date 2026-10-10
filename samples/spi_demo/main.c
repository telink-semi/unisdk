#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_spi.h"
#include "properties/tlk_spi.h"
#include "system/debug/log/tlk_log.h"

TLK_LOG_CREATE(spi_demo, "SPI_DEMO");

#define BUFFER_SIZE 8

enum demo_command
{
    COMMAND_WRITE = 1,
    COMMAND_READ  = 2,
};

static uint8_t tx_data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
static uint8_t rx_data[8] = {0};

void handler(struct tlk_spi_device* dev, tlk_spi_cmd_t cmd)
{
    tlk_gpio_pin_toggle(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN);

    switch (cmd)
    {
    case COMMAND_WRITE:
        tlk_spi_slave_read(dev, rx_data, BUFFER_SIZE);

        break;
    case COMMAND_READ:
        tlk_spi_slave_write(dev, tx_data, BUFFER_SIZE);

        break;
    default:
        break;
    }
}

int main(void)
{
    struct tlk_spi_device* dev = &tlk_spi0;

    struct tlk_spi_config config = {
        .mode    = TLK_SPI_MODE0,
        .io_mode = TLK_SPI_SINGLE_MODE,
        /* lower the frequency in case of errors on the slave side, if the master's div allows it */
        .frequency = 2 * 1000 * 1000,
    };

    struct tlk_spi_pinmux pinmux = {
        .cs   = {.port = TLK_GPIO_PORT_E, .pin = TLK_GPIO_PIN_0},
        .sck  = {.port = TLK_GPIO_PORT_E, .pin = TLK_GPIO_PIN_1},
        .mosi = {.port = TLK_GPIO_PORT_E, .pin = TLK_GPIO_PIN_2},
        .miso = {.port = TLK_GPIO_PORT_E, .pin = TLK_GPIO_PIN_3},
    };

    tlk_gpio_configure(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_configure(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN, TLK_GPIO_OUTPUT);

    tlk_core_interrupt_enable();

    tlk_spi_pinmux_configure(dev, &pinmux);

#if TLK_IS_ENABLED(CONFIG_TLK_SPI_DEMO_ROLE_MASTER)

    tlk_spi_master_configure(dev, &config);

    enum tlk_spi_status status;

    struct tlk_spi_xfer xfer = {
        .tx_buffer = tx_data,
        .tx_size   = BUFFER_SIZE,
        .rx_buffer = rx_data,
        .rx_size   = BUFFER_SIZE,
        .use_cmd   = 1,
    };

    while (1)
    {
        xfer.type = TLK_SPI_DUMMY_TX;
        xfer.cmd  = COMMAND_WRITE;

        status = tlk_spi_master_xfer(dev, &xfer);

        TLK_LOG_INFO(spi_demo, "Xfer status: %x", status);

        tlk_gpio_pin_toggle(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN);

        tlk_api_time_delay(TLK_MS_TO_US(10));

        xfer.type = TLK_SPI_DUMMY_RX;
        xfer.cmd  = COMMAND_READ;

        status = tlk_spi_master_xfer(dev, &xfer);

        TLK_LOG_INFO(spi_demo, "Xfer status: %x", status);

        if (tx_data[0] == rx_data[0])
        {
            tlk_gpio_pin_toggle(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN);
        }

        tlk_api_time_delay(TLK_SEC_TO_US(1));
    }

#elif TLK_IS_ENABLED(CONFIG_TLK_SPI_DEMO_ROLE_SLAVE)

    tlk_spi_slave_configure(dev, &config);

    tlk_spi_slave_set_handler(dev, handler);

    while (1)
    {
    }

#endif
}
