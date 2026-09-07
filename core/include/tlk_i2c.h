#ifndef TLK_INCLUDE_DRIVERS_I2C_H_
#define TLK_INCLUDE_DRIVERS_I2C_H_

#include "core/include/tlk_gpio.h"
#include "properties/tlk_i2c.h"

enum tlk_i2c_role
{
    TLK_I2C_MASTER,
    TLK_I2C_SLAVE,
};

enum tlk_i2c_status
{
    TLK_I2C_OK,
    TLK_I2C_TIMEOUT,
    TLK_I2C_NACK,
    TLK_I2C_UNSUPPORTED,
};

enum tlk_i2c_event
{
    TLK_I2C_EVENT_WRITE_REQUEST,
    TLK_I2C_EVENT_READ_REQUEST,
    TLK_I2C_EVENT_STOP,
};

struct tlk_i2c_device;

/**
 * @brief 7-bit I2C address
 */
typedef uint8_t tlk_i2c_address_t;
typedef void (*tlk_i2c_slave_event_handler_t)(struct tlk_i2c_device* dev, enum tlk_i2c_event event);

struct tlk_i2c_master_config
{
    uint32_t clock_speed;
    bool     stretch_en;
};

struct tlk_i2c_slave_config
{
    tlk_i2c_address_t address;
    bool              stretch_en;
};
struct tlk_i2c_pinmux_config
{
    struct tlk_gpio_port_pin sda_port_pin;
    struct tlk_gpio_port_pin scl_port_pin;
};

struct tlk_i2c_message
{
    uint8_t* buffer;
    uint32_t size;
    struct
    {
        uint8_t is_read : 1;
    };
};

#define TLK_DECLARE_I2C_DEVICE(num) extern struct tlk_i2c_device tlk_i2c##num;

#define TLK_DECLARE_I2C_DEVICE_IF_ENABLED(num)                                                     \
    TLK_IF_ENABLED(CONFIG_TLK_I2C##num, (TLK_DECLARE_I2C_DEVICE(num)))

TLK_FOR_CALL(UNISDK_I2C_COUNT, TLK_DECLARE_I2C_DEVICE_IF_ENABLED)

/**
 * @brief Configure the I2C pin multiplexing.
 *
 * @param dev     Pointer to the I2C device.
 * @param config  Pointer to the pinmux configuration.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_pinmux_configure(struct tlk_i2c_device*        dev,
                                             struct tlk_i2c_pinmux_config* config);

/**
 * @brief Configure the I2C device as a master.
 *
 * @param dev     Pointer to the I2C device.
 * @param config  Pointer to the master configuration.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_master_configure(struct tlk_i2c_device*        dev,
                                             struct tlk_i2c_master_config* config);
/**
 * @brief Write data to an I2C slave device as master.
 *
 * @param dev     Pointer to the I2C device.
 * @param id      7-bit slave address.
 * @param buffer  Pointer to the data buffer to write.
 * @param size    Number of bytes to write.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_master_write(struct tlk_i2c_device* dev, tlk_i2c_address_t id,
                                         uint8_t* buffer, uint32_t size);
/**
 * @brief Read data from an I2C slave device as master.
 *
 * @param dev     Pointer to the I2C device.
 * @param id      7-bit slave address.
 * @param buffer  Pointer to the buffer to store read data.
 * @param size    Number of bytes to read.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_master_read(struct tlk_i2c_device* dev, tlk_i2c_address_t id,
                                        uint8_t* buffer, uint32_t size);
/**
 * @brief Perform a combined I2C transfer (write and/or read messages).
 *
 * @param dev      Pointer to the I2C device.
 * @param id       7-bit slave address.
 * @param messages Array of I2C messages to transfer.
 * @param count    Number of messages in the array.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_master_xfer(struct tlk_i2c_device* dev, tlk_i2c_address_t id,
                                        struct tlk_i2c_message* messages, uint8_t count);
/**
 * @brief Set the I2C master timeout.
 *
 * @param dev        Pointer to the I2C device.
 * @param timeout_us Timeout in microseconds.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_master_set_timeout(struct tlk_i2c_device* dev, uint32_t timeout_us);

/**
 * @brief Configure the I2C device as a slave.
 *
 * @param dev     Pointer to the I2C device.
 * @param config  Pointer to the slave configuration.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_slave_configure(struct tlk_i2c_device*       dev,
                                            struct tlk_i2c_slave_config* config);
/**
 * @brief Set the receive buffer for the I2C slave.
 *
 * @param dev     Pointer to the I2C device.
 * @param buffer  Pointer to the receive buffer.
 * @param size    Size of the receive buffer in bytes.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_slave_set_rx(struct tlk_i2c_device* dev, uint8_t* buffer,
                                         uint32_t size);
/**
 * @brief Set the transmit buffer for the I2C slave.
 *
 * @param dev     Pointer to the I2C device.
 * @param buffer  Pointer to the transmit buffer.
 * @param size    Size of the transmit buffer in bytes.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_slave_set_tx(struct tlk_i2c_device* dev, uint8_t* buffer,
                                         uint32_t size);
/**
 * @brief Set the event handler for the I2C slave.
 *
 * @param dev     Pointer to the I2C device.
 * @param handler Event handler callback function.
 *
 * @return TLK_I2C_OK on success, or an error code on failure.
 */
enum tlk_i2c_status tlk_i2c_slave_set_handler(struct tlk_i2c_device*        dev,
                                              tlk_i2c_slave_event_handler_t handler);

#endif
