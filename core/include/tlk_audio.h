#ifndef TLK_INCLUDE_DRIVERS_AUDIO_H_
#define TLK_INCLUDE_DRIVERS_AUDIO_H_

#include "core/include/tlk_dma.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_i2s.h"
#include "properties/tlk_audio.h"

#define TLK_AUDIO_DECLARE_INPUT_STREAM(num) TLK_AUDIO_INPUT_STREAM##num,

enum tlk_audio_input_stream
{
    TLK_FOR_CALL(UNISDK_AUDIO_INPUT_STREAM_COUNT, TLK_AUDIO_DECLARE_INPUT_STREAM)
};

enum tlk_audio_input_source
{
    TLK_AUDIO_INPUT_LINE,
    TLK_AUDIO_INPUT_AMIC,
    TLK_AUDIO_INPUT_DMIC,
};

enum tlk_audio_sample_rate
{
    TLK_AUDIO_8K,      // 8K=12Mhz/1500
    TLK_AUDIO_8P021K,  // 8.021K=12Mhz/1496
    TLK_AUDIO_11P025K, // 11.025K=12Mhz/1088
    TLK_AUDIO_12K,     // 12K=12Mhz/1000
    TLK_AUDIO_16K,     // 16K=12Mhz/750
    TLK_AUDIO_22P058K, // 22.058K=12Mhz/544
    TLK_AUDIO_24K,     // 24K=12Mhz/500
    TLK_AUDIO_32K,     // 32K=12Mhz/375
    TLK_AUDIO_44P118K, // 44.118K=12Mhz/272
    TLK_AUDIO_48K,     // 48k=12Mhz/250
};

struct tlk_audio_sdm_pinmux
{
    struct tlk_gpio_port_pin sdm0_p;
    struct tlk_gpio_port_pin sdm0_n;
    struct tlk_gpio_port_pin sdm1_p;
    struct tlk_gpio_port_pin sdm1_n;
};

struct tlk_audio_amic_pinmux
{
    struct tlk_gpio_port_pin bias;
};

struct tlk_audio_dmic_pinmux
{
    struct tlk_gpio_port_pin data;
    struct tlk_gpio_port_pin clk1;
    struct tlk_gpio_port_pin clk2;
};

struct tlk_audio_input
{
    enum tlk_audio_input_stream stream;
    enum tlk_audio_input_source source;
    enum tlk_audio_sample_rate  sample_rate;
    enum tlk_i2s_channel        audio_channel;
    enum tlk_i2s_data_width     data_width;
    enum tlk_dma_chn            dma_channel;
    enum tlk_i2s_fifo           fifo;
    struct tlk_dma_ll_node*     ll_head;
};

struct tlk_audio_output
{
    enum tlk_audio_sample_rate sample_rate;
    enum tlk_i2s_channel       audio_channel;
    enum tlk_i2s_data_width    data_width;
    enum tlk_dma_chn           dma_channel;
    enum tlk_i2s_fifo          fifo;
    struct tlk_dma_ll_node*    ll_head;
};

/**
 * @brief Initialize the audio driver.
 *
 * @return None.
 */
void tlk_audio_init(void);

/**
 * @brief Configure pin multiplexing for analog microphone.
 *
 * @param mux - pointer to the pin multiplexing struct.
 *
 * @return None.
 */
void tlk_audio_amic_pinmux_configure(struct tlk_audio_amic_pinmux* mux);

/**
 * @brief Configure pin multiplexing for digital microphone.
 *
 * @param stream - audio stream used for the sampling.
 * @param mux - pointer to the pin multiplexing struct.
 *
 * @return None.
 */
void tlk_audio_dmic_pinmux_configure(enum tlk_audio_input_stream   stream,
                                     struct tlk_audio_dmic_pinmux* mux);

/**
 * @brief Configure pin multiplexing for sdm output.
 *
 * @param mux - pointer to the pin multiplexing struct.
 *
 * @return None.
 */
void tlk_audio_sdm_pinmux_configure(struct tlk_audio_sdm_pinmux* mux);

/**
 * @brief Configure audio input.
 *
 * @param cfg - pointer to the input configuration struct.
 *
 * @return None.
 */
void tlk_audio_input_configure(struct tlk_audio_input* cfg);

/**
 * @brief Configure audio output.
 *
 * @param cfg - pointer to the output configuration struct.
 *
 * @return None.
 */
void tlk_audio_output_configure(struct tlk_audio_output* cfg);

/**
 * @brief Enable audio input.
 *
 * @return None.
 */
void tlk_audio_input_enable(void);

/**
 * @brief Enable audio output.
 *
 * @return None.
 */
void tlk_audio_output_enable(void);

/**
 * @brief Disable audio input.
 *
 * @return None.
 */
void tlk_audio_input_disable(void);

/**
 * @brief Disable audio output.
 *
 * @return None.
 */
void tlk_audio_output_disable(void);

#endif
