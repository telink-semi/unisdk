#include "core/cpp_wrappers/tlk_gpio.hpp"
#include "core/cpp_wrappers/tlk_i2c.hpp"

#ifdef __cplusplus
extern "C"
{
#include "system/debug/log/tlk_log.h"
#endif
#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "ds1307.h"
#include "tlk_board_pinout.h"
#include <string.h>
#ifdef __cplusplus
}
#endif

TLK_LOG_CREATE(i2c_demo, "I2C_DEMO");

struct tlk_i2c_device* dev = &tlk_i2c0;
I2CDriver              i2c(dev);

void set_time_cb(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    (void) port;
    (void) pin;

    struct ds1307_time time = {
        .sec   = 00,
        .min   = 30,
        .hour  = 10,
        .day   = 0,
        .date  = 12,
        .month = 5,
        .year  = 2026,
    };

    if (ds1307_set_time(dev, &time) == DS1307_OK)
    {
        TLK_LOG_INFO(i2c_demo, "The time has been set.");
    }
}

void enable_time_setting(void)
{
    // Applying HIGH to KEY_2 pin to detect a logic '1' on the KEY_0, when pressed.
    GPIODriver key2(UNISDK_BOARD_KEY_2_PORT);
    key2.configure(UNISDK_BOARD_KEY_2_PIN, TLK_GPIO_OUTPUT);
    key2.write(UNISDK_BOARD_KEY_2_PIN, 1);

    GPIODriver key0(UNISDK_BOARD_KEY_0_PORT);
    key0.configure(UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INPUT_PULL_DOWN);

    static struct tlk_gpio_irq_callback callback = {
        .node    = {},
        .handler = set_time_cb,
        .pin     = UNISDK_BOARD_KEY_0_PIN,
    };

    key0.irqAddCallback(&callback);
    key0.irqConfigure(UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INTR_RISING_EDGE);
}

int main(void)
{
    struct tlk_i2c_pinmux_config pinmux = {
        .sda_port_pin = {.port = TLK_GPIO_PORT_C, .pin = TLK_GPIO_PIN_7},
        .scl_port_pin = {.port = TLK_GPIO_PORT_C, .pin = TLK_GPIO_PIN_6},
    };

    tlk_core_interrupt_enable();

    i2c.configurePinmux(&pinmux);

    struct tlk_i2c_master_config config = {
        .clock_speed = 100000,
        .stretch_en  = 1,
    };

    i2c.masterConfigure(&config);

    enable_time_setting();

    while (1)
    {
        struct ds1307_time time = {};

        if (ds1307_read_time(dev, &time) == DS1307_OK)
        {
            TLK_LOG_INFO(i2c_demo,
                         "Now is %04d.%02d.%02d %02d:%02d:%02d",
                         time.year,
                         time.month,
                         time.date,
                         time.hour,
                         time.min,
                         time.sec);
        }

        tlk_api_delay(TLK_SEC_TO_US(1));
    }
}
