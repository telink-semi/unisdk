#include "core/cpp_wrappers/tlk_gpio.hpp"
#include "core/cpp_wrappers/tlk_spi.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "system/debug/log/tlk_log.h"
#include "tlk_board_pinout.h"
#ifdef __cplusplus
}
#endif

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
    GPIODriver led0(UNISDK_BOARD_LED_0_PORT);
    led0.toggle(UNISDK_BOARD_LED_0_PIN);

    SpiDriver spi(dev);

    switch (cmd)
    {
    case COMMAND_WRITE:
        spi.slaveRead(rx_data, BUFFER_SIZE);
        TLK_LOG_INFO(
            spi_demo, "Slave RX Data: %d %d %d %d", rx_data[0], rx_data[1], rx_data[2], rx_data[3]);
        break;
    case COMMAND_READ:
        spi.slaveWrite(tx_data, BUFFER_SIZE);
        TLK_LOG_INFO(spi_demo, "Slave TX Data Sent");
        break;
    default:
        break;
    }
}

int main(void)
{
    SpiDriver spi(&tlk_spi0);

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
        .io2  = {},
        .io3  = {},
    };

    GPIODriver led0(UNISDK_BOARD_LED_0_PORT);
    led0.configure(UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);

    GPIODriver led1(UNISDK_BOARD_LED_1_PORT);
    led1.configure(UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);

    tlk_core_interrupt_enable();

    spi.configurePinmux(&pinmux);

#if TLK_IS_ENABLED(CONFIG_TLK_SPI_DEMO_ROLE_MASTER)

    spi.masterConfigure(&config);

    enum tlk_spi_status status;

    struct tlk_spi_xfer xfer = {
        .type      = TLK_SPI_NO_DATA,
        .tx_buffer = tx_data,
        .tx_size   = BUFFER_SIZE,
        .rx_buffer = rx_data,
        .rx_size   = BUFFER_SIZE,
        .cmd       = 0,
        .use_cmd   = 1,
    };

    while (1)
    {
        xfer.type = TLK_SPI_DUMMY_TX;
        xfer.cmd  = COMMAND_WRITE;

        status = spi.masterXfer(&xfer);

        TLK_LOG_INFO(spi_demo, "Xfer status: %x", status);

        led0.toggle(UNISDK_BOARD_LED_0_PIN);

        tlk_api_delay(TLK_MS_TO_US(10));

        xfer.type = TLK_SPI_DUMMY_RX;
        xfer.cmd  = COMMAND_READ;

        status = spi.masterXfer(&xfer);

        TLK_LOG_INFO(spi_demo, "Xfer status: %x", status);

        if (tx_data[0] == rx_data[0])
        {
            led1.toggle(UNISDK_BOARD_LED_1_PIN);
        }

        tlk_api_delay(TLK_SEC_TO_US(1));
    }

#elif TLK_IS_ENABLED(CONFIG_TLK_SPI_DEMO_ROLE_SLAVE)

    spi.slaveConfigure(&config);

    spi.slaveSetHandler(handler);

    while (1)
    {
    }

#endif
}
