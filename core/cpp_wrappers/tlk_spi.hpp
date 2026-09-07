#ifndef SPI_DRIVER_HPP
#define SPI_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_spi.h"
#ifdef __cplusplus
}
#endif

class SpiDriver
{
  public:
    explicit SpiDriver(tlk_spi_device* dev) : dev_(dev) {}

    tlk_spi_status configurePinmux(tlk_spi_pinmux* mux)
    {
        return tlk_spi_configure_pinmux(dev_, mux);
    }

    tlk_spi_status masterConfigure(tlk_spi_config* cfg)
    {
        return tlk_spi_master_configure(dev_, cfg);
    }

    tlk_spi_status masterXfer(tlk_spi_xfer* xfer)
    {
        return tlk_spi_master_xfer(dev_, xfer);
    }

    tlk_spi_status slaveConfigure(tlk_spi_config* cfg)
    {
        return tlk_spi_slave_configure(dev_, cfg);
    }

    tlk_spi_status slaveSetHandler(tlk_spi_slave_event_handler_t handler)
    {
        return tlk_spi_slave_set_handler(dev_, handler);
    }

    tlk_spi_status slaveWrite(uint8_t* buffer, uint32_t size)
    {
        return tlk_spi_slave_write(dev_, buffer, size);
    }

    tlk_spi_status slaveRead(uint8_t* buffer, uint32_t size)
    {
        return tlk_spi_slave_read(dev_, buffer, size);
    }

  private:
    tlk_spi_device* dev_;
};

#endif
