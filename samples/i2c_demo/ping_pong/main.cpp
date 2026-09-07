#include "core/cpp_wrappers/tlk_gpio.hpp"
#include "core/cpp_wrappers/tlk_i2c.hpp"

#ifdef __cplusplus
extern "C"
{
#include "system/debug/log/tlk_log.h"
#endif
#include "api/include/tlk_sleep.h"
#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "tlk_board_pinout.h"
#include <string.h>
#ifdef __cplusplus
}
#endif

#define SLAVE_ADDRESS 0x5a
#define BUFFER_SIZE   8

TLK_LOG_CREATE(i2c_demo, "I2C_DEMO");

uint8_t tx_buffer[BUFFER_SIZE]
    __attribute__((aligned(4))) = {'i', '2', 'c', '_', 'd', 'e', 'm', 'o'};
uint8_t rx_buffer[BUFFER_SIZE] __attribute__((aligned(4))) = {0};

const char* status_to_string(enum tlk_i2c_status status)
{
    switch (status)
    {
    case TLK_I2C_OK:
        return "ok";
    case TLK_I2C_TIMEOUT:
        return "timeout";
    case TLK_I2C_NACK:
        return "nack";
    default:
        return "unknown";
    }
}

void handler(struct tlk_i2c_device* dev, enum tlk_i2c_event event)
{
    (void) dev;

    switch (event)
    {
    case TLK_I2C_EVENT_WRITE_REQUEST:
    {
        GPIODriver led0(UNISDK_BOARD_LED_0_PORT);
        led0.toggle(UNISDK_BOARD_LED_0_PIN);
        break;
    }
    case TLK_I2C_EVENT_READ_REQUEST:
    {
        GPIODriver led1(UNISDK_BOARD_LED_1_PORT);
        led1.toggle(UNISDK_BOARD_LED_1_PIN);
        memcpy(tx_buffer, rx_buffer, BUFFER_SIZE);
        break;
    }
    case TLK_I2C_EVENT_STOP:
        TLK_LOG_INFO(i2c_demo, "Stop received.");
        break;
    default:
        break;
    }
}

int main(void)
{
    I2CDriver i2c(&tlk_i2c0);

    struct tlk_i2c_pinmux_config pinmux = {
        .sda_port_pin = {.port = TLK_GPIO_PORT_C, .pin = TLK_GPIO_PIN_7},
        .scl_port_pin = {.port = TLK_GPIO_PORT_C, .pin = TLK_GPIO_PIN_6},
    };

    tlk_core_interrupt_enable();

    i2c.configurePinmux(&pinmux);

#if TLK_IS_ENABLED(CONFIG_TLK_I2C_DEMO_ROLE_MASTER)

    struct tlk_i2c_master_config config = {
        .clock_speed = 100000,
        .stretch_en  = 1,
    };

    i2c.masterConfigure(&config);

    while (1)
    {
        enum tlk_i2c_status status = TLK_I2C_OK;

        memset(rx_buffer, 0, BUFFER_SIZE);

        status = i2c.masterWrite(SLAVE_ADDRESS, tx_buffer, BUFFER_SIZE);
        TLK_LOG_INFO(i2c_demo, "Master write status: %s", status_to_string(status));

        tlk_api_delay(TLK_MS_TO_US(10));

        status = i2c.masterRead(SLAVE_ADDRESS, rx_buffer, BUFFER_SIZE);
        TLK_LOG_INFO(i2c_demo, "Master read status: %s", status_to_string(status));

        if (tx_buffer[0] == rx_buffer[0] &&
            tx_buffer[BUFFER_SIZE - 1] == rx_buffer[BUFFER_SIZE - 1])
        {
            TLK_LOG_INFO(i2c_demo, "Transmission successful.");
        }
        else
        {
            TLK_LOG_INFO(i2c_demo, "Error: Wrong data received.");
        }

        tlk_api_delay(TLK_MS_TO_US(500));

#if TLK_IS_ENABLED(CONFIG_TLK_I2C_DEMO_PM)
        tlk_api_sleep(500);
#else
        tlk_api_delay(TLK_MS_TO_US(500));
#endif
    }

#elif TLK_IS_ENABLED(CONFIG_TLK_I2C_DEMO_ROLE_SLAVE)

    struct tlk_i2c_slave_config config = {
        .address    = SLAVE_ADDRESS,
        .stretch_en = 1,
    };

    i2c.slaveConfigure(&config);
    i2c.slaveSetHandler(handler);
    i2c.slaveSetTx(tx_buffer, BUFFER_SIZE);
    i2c.slaveSetRx(rx_buffer, BUFFER_SIZE);

    GPIODriver led0(UNISDK_BOARD_LED_0_PORT);
    led0.configure(UNISDK_BOARD_LED_0_PIN, TLK_GPIO_OUTPUT);

    GPIODriver led1(UNISDK_BOARD_LED_1_PORT);
    led1.configure(UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);

    while (1)
    {
    }

#endif
}
