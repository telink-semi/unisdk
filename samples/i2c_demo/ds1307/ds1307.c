#include "ds1307.h"
#include "system/debug/log/tlk_log.h"
#include <stdint.h>

TLK_LOG_CREATE(ds1307, "DS1307");

char* status_to_string(enum tlk_i2c_status status)
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

uint8_t bcd_to_bin(uint8_t v)
{
    return ((v >> 4) * 10) + (v & 0x0F);
}

uint8_t bin_to_bcd(uint8_t v)
{
    return ((v / 10) << 4) | (v % 10);
}

enum ds1307_status ds1307_set_time(struct tlk_i2c_device* dev, const struct ds1307_time* t)
{
    uint8_t buf[8];

    buf[0] = 0x00; // start register address

    buf[1] = bin_to_bcd(t->sec);
    buf[2] = bin_to_bcd(t->min);
    buf[3] = bin_to_bcd(t->hour);
    buf[4] = bin_to_bcd(t->day);
    buf[5] = bin_to_bcd(t->date);
    buf[6] = bin_to_bcd(t->month);
    buf[7] = bin_to_bcd(t->year % 100);

    enum tlk_i2c_status status = tlk_i2c_master_write(dev, DS1307_ADDRESS, buf, sizeof(buf));

    if (status != TLK_I2C_OK)
    {
        TLK_LOG_DEBUG(ds1307, "I2C status: %s", status_to_string(status));

        return DS1307_ERROR;
    }

    return DS1307_OK;
}

enum ds1307_status ds1307_read_time(struct tlk_i2c_device* dev, struct ds1307_time* t)
{
    enum tlk_i2c_status status = TLK_I2C_OK;
    uint8_t             reg    = 0x00;
    uint8_t             raw[7];

    struct tlk_i2c_message messages[2] = {
        {
            .buffer  = &reg,
            .size    = 1,
            .is_read = 0,
        },
        {
            .buffer  = raw,
            .size    = 7,
            .is_read = 1,
        },
    };

    status = tlk_i2c_master_xfer(dev, DS1307_ADDRESS, messages, 2);

    if (status != TLK_I2C_OK)
    {
        TLK_LOG_DEBUG(ds1307, "I2C status: %s", status_to_string(status));

        return DS1307_ERROR;
    }

    t->sec   = bcd_to_bin(raw[0] & 0x7F);
    t->min   = bcd_to_bin(raw[1]);
    t->hour  = bcd_to_bin(raw[2] & 0x3F);
    t->day   = bcd_to_bin(raw[3]);
    t->date  = bcd_to_bin(raw[4]);
    t->month = bcd_to_bin(raw[5]);
    t->year  = 2000 + bcd_to_bin(raw[6]);

    return DS1307_OK;
}
