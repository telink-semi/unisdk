#ifndef TLK_INCLUDE_DRIVERS_SPI_H_
#define TLK_INCLUDE_DRIVERS_SPI_H_

#include "core/include/tlk_gpio.h"
#include "properties/tlk_spi.h"

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

enum tlk_spi_xfer_type
{
    TLK_SPI_TRANSCEIVE,
    TLK_SPI_TX,
    TLK_SPI_RX,
    TLK_SPI_TX_THEN_RX,
    TLK_SPI_RX_THEN_TX,
    TLK_SPI_TX_DUMMY_RX,
    TLK_SPI_RX_DUMMY_TX,
    TLK_SPI_NO_DATA,
    TLK_SPI_DUMMY_TX,
    TLK_SPI_DUMMY_RX,
};

enum tlk_spi_status
{
    TLK_SPI_OK,
    TLK_SPI_TIMEOUT,
};

struct tlk_spi_device;

typedef uint8_t tlk_spi_cmd_t;
typedef void (*tlk_spi_slave_event_handler_t)(struct tlk_spi_device* dev, tlk_spi_cmd_t cmd);

struct tlk_spi_pinmux
{
    struct tlk_gpio_port_pin cs;
    struct tlk_gpio_port_pin sck;
    struct tlk_gpio_port_pin mosi;
    struct tlk_gpio_port_pin miso;
    struct tlk_gpio_port_pin io2;
    struct tlk_gpio_port_pin io3;
};

struct tlk_spi_config
{
    enum tlk_spi_mode    mode;
    enum tlk_spi_io_mode io_mode;
    uint32_t             frequency;
};

struct tlk_spi_xfer
{
    enum tlk_spi_xfer_type type;
    uint8_t*               tx_buffer;
    uint32_t               tx_size;
    uint8_t*               rx_buffer;
    uint32_t               rx_size;
    tlk_spi_cmd_t          cmd;
    uint8_t                use_cmd : 1;
};

#define TLK_DECLARE_SPI_DEVICE(num) extern struct tlk_spi_device tlk_spi##num;

TLK_FOR_CALL(UNISDK_SPI_COUNT, TLK_DECLARE_SPI_DEVICE)

/**
 * @brief Configure the SPI pin multiplexing.
 *
 * @param dev     Pointer to the SPI device.
 * @param mux     Pointer to the pinmux configuration.
 *
 * @return TLK_SPI_OK on success, or an error code on failure.
 */
enum tlk_spi_status tlk_spi_pinmux_configure(struct tlk_spi_device* dev,
                                             struct tlk_spi_pinmux* mux);

/**
 * @brief Configure the SPI device as a master.
 *
 * @param dev     Pointer to the SPI device.
 * @param cfg     Pointer to the master configuration.
 *
 * @return TLK_SPI_OK on success, or an error code on failure.
 */
enum tlk_spi_status tlk_spi_master_configure(struct tlk_spi_device* dev,
                                             struct tlk_spi_config* cfg);

/**
 * @brief Perform a combined SPI transfer (write and/or read messages).
 *
 * @param dev      Pointer to the SPI device.
 * @param xfer     Pointer to the transfer description.
 *
 * @return TLK_SPI_OK on success, or an error code on failure.
 */
enum tlk_spi_status tlk_spi_master_xfer(struct tlk_spi_device* dev, struct tlk_spi_xfer* xfer);

/**
 * @brief Configure the SPI device as a slave.
 *
 * @param dev     Pointer to the SPI device.
 * @param cfg     Pointer to the slave configuration.
 *
 * @return TLK_SPI_OK on success, or an error code on failure.
 */
enum tlk_spi_status tlk_spi_slave_configure(struct tlk_spi_device* dev, struct tlk_spi_config* cfg);

/**
 * @brief Set the event handler for the SPI slave.
 *
 * @param dev     Pointer to the SPI device.
 * @param handler Event handler callback function.
 *
 * @return TLK_SPI_OK on success, or an error code on failure.
 */
enum tlk_spi_status tlk_spi_slave_set_handler(struct tlk_spi_device*        dev,
                                              tlk_spi_slave_event_handler_t handler);

/**
 * @brief Feed the data into the tx fifo.
 *
 * @param dev     Pointer to the SPI slave device.
 * @param buffer  Pointer to the transmit buffer.
 * @param size    Size of the transmit buffer in bytes.
 *
 * @return TLK_SPI_OK on success, or an error code on failure.
 */
enum tlk_spi_status tlk_spi_slave_write(struct tlk_spi_device* dev, uint8_t* buffer, uint32_t size);

/**
 * @brief Read the data from the rx fifo.
 *
 * @param dev     Pointer to the SPI slave device.
 * @param buffer  Pointer to the receive buffer.
 * @param size    Size of the receive buffer in bytes.
 *
 * @return TLK_SPI_OK on success, or an error code on failure.
 */
enum tlk_spi_status tlk_spi_slave_read(struct tlk_spi_device* dev, uint8_t* buffer, uint32_t size);

#endif
