#ifndef TLK_DS1307_H_
#define TLK_DS1307_H_

#include "core/include/tlk_i2c.h"

#define DS1307_ADDRESS 0x68

enum ds1307_status
{
    DS1307_OK    = 0,
    DS1307_ERROR = 1,
};

struct ds1307_time
{
    uint8_t  sec;
    uint8_t  min;
    uint8_t  hour;
    uint8_t  day;
    uint8_t  date;
    uint8_t  month;
    uint16_t year;
};

enum ds1307_status ds1307_set_time(struct tlk_i2c_device* dev, const struct ds1307_time* t);

enum ds1307_status ds1307_read_time(struct tlk_i2c_device* dev, struct ds1307_time* t);

#endif
