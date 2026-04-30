#ifndef TLK_INCLUDE_DRIVERS_SPI_H_
#define TLK_INCLUDE_DRIVERS_SPI_H_

#include "core/include/tlk_gpio.h"

#define TLK_DECLARE_SPI_MODULE(num) TLK_SPI##num,

enum tlk_spi_module
{
    FOR_CALL(CONFIG_SPI_COUNT, TLK_DECLARE_SPI_MODULE)
};

enum tlk_spi_role
{
    TLK_SPI_MASTER,
    TLK_SPI_SLAVE,
};

enum tlk_spi_mode
{
    TLK_SPI_MODE0,
    TLK_SPI_MODE1,
    TLK_SPI_MODE2,
    TLK_SPI_MODE3,
};

enum tlk_spi_io_mode
{
    TLK_SPI_SINGLE_MODE,
    TLK_SPI_DUAL_MODE,
    TLK_SPI_QUAD_MODE,
    TLK_SPI_3_LINE_MODE,
};

struct tlk_spi_device
{
    enum tlk_spi_module module;

    volatile bool rx_done;
    unsigned char *rx_buf;
    volatile unsigned int rx_buf_size;
    volatile unsigned int rx_cnt;
};

struct tlk_spi_pins
{
    struct tlk_gpio_port_pin cs_port_pin;
    struct tlk_gpio_port_pin sck_port_pin;
    struct tlk_gpio_port_pin mosi_port_pin;
    struct tlk_gpio_port_pin miso_port_pin;
};

struct tlk_spi_config
{
    struct tlk_spi_pins pins;
    enum tlk_spi_role role;
    enum tlk_spi_mode mode;
    enum tlk_spi_io_mode io_mode;
    unsigned short div_clock;
};

#define TLK_DECLARE_SPI_DEVICE(num) extern struct tlk_spi_device tlk_spi##num;

FOR_CALL(CONFIG_SPI_COUNT, TLK_DECLARE_SPI_DEVICE)

void tlk_spi_configure(struct tlk_spi_device* device, struct tlk_spi_config* config);
bool tlk_spi_write(struct tlk_spi_device* device, const unsigned char *buf, unsigned int size);
void tlk_spi_receive(struct tlk_spi_device* device, unsigned char *buf, unsigned int size);


#endif