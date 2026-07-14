#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_i2c.h"
#include "system/debug/log/tlk_log.h"
#include "tlk_board_pinout.h"
#include "tlk_core.h"
#include <string.h>

#include "ds1307.h"

TLK_LOG_CREATE(i2c_demo, "I2C_DEMO");

struct tlk_i2c_device* dev = &tlk_i2c0;

void set_time_cb(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    (void) port;
    (void) pin;

    struct ds1307_time time = {
        .year  = 2026,
        .month = 5,
        .date  = 12,
        .hour  = 10,
        .min   = 30,
        .sec   = 00,
    };

    if (ds1307_set_time(dev, &time) == DS1307_OK)
    {
        TLK_LOG_INFO(i2c_demo, "The time has been set.");
    }
}

void enable_time_setting(void)
{
    // Applying HIGH to KEY_2 pin to detect a logic '1' on the KEY_0, when pressed.
    tlk_gpio_configure(UNISDK_BOARD_KEY_2_PORT, UNISDK_BOARD_KEY_2_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(UNISDK_BOARD_KEY_2_PORT, UNISDK_BOARD_KEY_2_PIN, 1);

    tlk_gpio_configure(UNISDK_BOARD_KEY_0_PORT, UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INPUT_PULL_DOWN);

    static struct tlk_gpio_irq_callback callback = {
        .handler = set_time_cb,
        .pin     = UNISDK_BOARD_KEY_0_PIN,
    };

    tlk_gpio_irq_add_callback(UNISDK_BOARD_KEY_0_PORT, &callback);
    tlk_gpio_irq_configure(
        UNISDK_BOARD_KEY_0_PORT, UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INTR_RISING_EDGE);
}

int main(void)
{
    struct tlk_i2c_pinmux_config pinmux = {
        .scl_port_pin = {.port = TLK_GPIO_PORT_C, .pin = TLK_GPIO_PIN_6},
        .sda_port_pin = {.port = TLK_GPIO_PORT_C, .pin = TLK_GPIO_PIN_7},
    };

    tlk_core_interrupt_enable();

    tlk_i2c_configure_pinmux(dev, &pinmux);

    struct tlk_i2c_master_config config = {
        .clock_speed = 100000,
        .stretch_en  = 1,
    };

    tlk_i2c_master_configure(dev, &config);

    enable_time_setting();

    while (1)
    {
        struct ds1307_time time = {0};

        memset(&time, 0, sizeof(time));

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
