#ifndef TLK_INCLUDE_DRIVERS_I2S_H_
#define TLK_INCLUDE_DRIVERS_I2S_H_

#include "core/include/tlk_dma.h"
#include "core/include/tlk_gpio.h"
#include "properties/tlk_i2s.h"

enum tlk_i2s_role
{
    TLK_I2S_SLAVE,
    TLK_I2S_MASTER,
};

enum tlk_i2s_channel
{
    TLK_I2S_CHANNEL_LEFT   = TLK_BIT(0),
    TLK_I2S_CHANNEL_RIGHT  = TLK_BIT(1),
    TLK_I2S_CHANNEL_STEREO = TLK_I2S_CHANNEL_LEFT | TLK_I2S_CHANNEL_RIGHT,
};

enum tlk_i2s_fifo
{
    TLK_I2S_FIFO0,
    TLK_I2S_FIFO1,
    TLK_I2S_FIFO2,
};

enum tlk_i2s_mode
{
    TLK_I2S_MODE_RJ,
    TLK_I2S_MODE_LJ,
    TLK_I2S_MODE_I2S,
    TLK_I2S_MODE_DSP,
    TLK_I2S_MODE_TDM,
};

enum tlk_i2s_data_width
{
    TLK_I2S_DATA_WIDTH_16_BIT,
    TLK_I2S_DATA_WIDTH_20_BIT,
    TLK_I2S_DATA_WIDTH_24_BIT,
};

enum tlk_i2s_status
{
    TLK_I2S_STATUS_OK,
    TLK_I2S_STATUS_UNSUPPORTED,
};

struct tlk_i2s_device;

struct tlk_i2s_pinmux
{
    struct tlk_gpio_port_pin bclk;
    struct tlk_gpio_port_pin adc_lr_clk;
    struct tlk_gpio_port_pin dac_lr_clk;
    struct tlk_gpio_port_pin adc_dat;
    struct tlk_gpio_port_pin dac_dat;
};

struct tlk_i2s_sample_rate
{
    uint16_t i2s_clk_div_numerator;
    uint16_t i2s_clk_div_denominator;
    uint16_t bclk_div;
    uint16_t adc_div;
    uint16_t dac_div;
};

struct tlk_i2s_config
{
    struct tlk_i2s_sample_rate sample_rate;
    enum tlk_i2s_data_width    data_width;
    enum tlk_i2s_mode          mode;
    enum tlk_i2s_role          role;
};

struct tlk_i2s_io_config
{
    enum tlk_i2s_channel    i2s_channel;
    enum tlk_dma_chn        dma_channel;
    enum tlk_i2s_fifo       fifo;
    struct tlk_dma_ll_node* ll_head;
};

#define TLK_DECLARE_I2S_DEVICE(num) extern struct tlk_i2s_device tlk_i2s##num;

#define TLK_DECLARE_I2S_DEVICE_IF_ENABLED(num)                                                     \
    TLK_IF_ENABLED(CONFIG_TLK_I2S##num, (TLK_DECLARE_I2S_DEVICE(num)))

TLK_FOR_CALL(UNISDK_I2S_COUNT, TLK_DECLARE_I2S_DEVICE_IF_ENABLED)

/**
 * @brief Configure the I2S device.
 *
 * @param dev     Pointer to the I2S device.
 * @param config  Pointer to the device configuration.
 *
 * @return TLK_I2S_STATUS_OK on success, or an error code on failure.
 */
enum tlk_i2s_status tlk_i2s_configure(struct tlk_i2s_device* dev, struct tlk_i2s_config* config);

/**
 * @brief Configure the I2S pin multiplexing.
 *
 * @param dev     Pointer to the I2S device.
 * @param config  Pointer to the pinmux configuration.
 *
 * @return TLK_I2S_STATUS_OK on success, or an error code on failure.
 */
enum tlk_i2s_status tlk_i2s_pinmux_configure(struct tlk_i2s_device* dev,
                                             struct tlk_i2s_pinmux* pinmux);

/**
 * @brief Configure the I2S input.
 *
 * @param dev     Pointer to the I2S device.
 * @param config  Pointer to the input configuration.
 *
 * @return TLK_I2S_STATUS_OK on success, or an error code on failure.
 */
enum tlk_i2s_status tlk_i2s_input_configure(struct tlk_i2s_device*    dev,
                                            struct tlk_i2s_io_config* config);

/**
 * @brief Configure the I2S output.
 *
 * @param dev     Pointer to the I2S device.
 * @param config  Pointer to the output configuration.
 *
 * @return TLK_I2S_STATUS_OK on success, or an error code on failure.
 */
enum tlk_i2s_status tlk_i2s_configure_output(struct tlk_i2s_device*    dev,
                                             struct tlk_i2s_io_config* config);

/**
 * @brief Start the I2S tx/rx.
 *
 * @param dev     Pointer to the I2S device.
 *
 * @return None.
 */
void tlk_i2s_start(struct tlk_i2s_device* dev);

#endif
