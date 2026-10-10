#ifndef I2C_DRIVER_HPP
#define I2C_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_i2c.h"
#ifdef __cplusplus
}
#endif

class I2CDriver
{
  public:
    explicit I2CDriver(tlk_i2c_device* dev) : dev_(dev) {}

    tlk_i2c_status configurePinmux(tlk_i2c_pinmux_config* config)
    {
        return tlk_i2c_configure_pinmux(dev_, config);
    }

    tlk_i2c_status masterConfigure(tlk_i2c_master_config* config)
    {
        return tlk_i2c_master_configure(dev_, config);
    }

    tlk_i2c_status masterWrite(tlk_i2c_address_t id, uint8_t* buffer, uint32_t size)
    {
        return tlk_i2c_master_write(dev_, id, buffer, size);
    }

    tlk_i2c_status masterRead(tlk_i2c_address_t id, uint8_t* buffer, uint32_t size)
    {
        return tlk_i2c_master_read(dev_, id, buffer, size);
    }

    tlk_i2c_status masterXfer(tlk_i2c_address_t id, tlk_i2c_message* messages, uint8_t count)
    {
        return tlk_i2c_master_xfer(dev_, id, messages, count);
    }

    tlk_i2c_status masterSetTimeout(uint32_t timeout_us)
    {
        return tlk_i2c_master_set_timeout(dev_, timeout_us);
    }

    tlk_i2c_status slaveConfigure(tlk_i2c_slave_config* config)
    {
        return tlk_i2c_slave_configure(dev_, config);
    }

    tlk_i2c_status slaveSetRx(uint8_t* buffer, uint32_t size)
    {
        return tlk_i2c_slave_set_rx(dev_, buffer, size);
    }

    tlk_i2c_status slaveSetTx(uint8_t* buffer, uint32_t size)
    {
        return tlk_i2c_slave_set_tx(dev_, buffer, size);
    }

    tlk_i2c_status slaveSetHandler(tlk_i2c_slave_event_handler_t handler)
    {
        return tlk_i2c_slave_set_handler(dev_, handler);
    }

  private:
    tlk_i2c_device* dev_;
};

#endif
