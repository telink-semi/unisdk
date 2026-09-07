#include "core/include/tlk_i2s.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_sys.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_audio_reg.h"

struct tlk_i2s_device
{
    const uint8_t number;
    struct
    {
        const enum tlk_gpio_mux bck;
        const enum tlk_gpio_mux lr0;
        const enum tlk_gpio_mux lr1;
        const enum tlk_gpio_mux dat0;
        const enum tlk_gpio_mux dat1;
    } const mux;
    enum tlk_i2s_data_width data_width;
};

#define TLK_DEFINE_I2S_DEVICE(num)                                                                 \
    struct tlk_i2s_device tlk_i2s##num = {                                                         \
        .number = num,                                                                             \
        .mux =                                                                                     \
            {                                                                                      \
                .bck  = TLK_GPIO_MUX_I2S##num##_BCK,                                               \
                .lr0  = TLK_GPIO_MUX_I2S##num##_LR0,                                               \
                .lr1  = TLK_GPIO_MUX_I2S##num##_LR1,                                               \
                .dat0 = TLK_GPIO_MUX_I2S##num##_DAT0,                                              \
                .dat1 = TLK_GPIO_MUX_I2S##num##_DAT1,                                              \
            },                                                                                     \
    };

TLK_FOR_CALL(UNISDK_I2S_COUNT, TLK_DEFINE_I2S_DEVICE)

struct tlk_dma_config audio_dma_rx_config[2] = {
    {
        .dst_req_sel    = 0,
        .src_req_sel    = UNISDK_DMA_FUNC_NUM_AUDIO0_RX,
        .dst_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
        .src_addr_ctrl  = TLK_DMA_ADDR_FIX,
        .dstmode        = TLK_DMA_NORMAL_MODE,
        .srcmode        = TLK_DMA_HANDSHAKE_MODE,
        .dstwidth       = TLK_DMA_WORD_WIDTH,
        .srcwidth       = TLK_DMA_WORD_WIDTH,
        .src_burst_size = 0,
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0,
    },
    {
        .dst_req_sel    = 0,
        .src_req_sel    = UNISDK_DMA_FUNC_NUM_AUDIO1_RX,
        .dst_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
        .src_addr_ctrl  = TLK_DMA_ADDR_FIX,
        .dstmode        = TLK_DMA_NORMAL_MODE,
        .srcmode        = TLK_DMA_HANDSHAKE_MODE,
        .dstwidth       = TLK_DMA_WORD_WIDTH,
        .srcwidth       = TLK_DMA_WORD_WIDTH,
        .src_burst_size = 0,
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0,
    },
};

struct tlk_dma_config audio_dma_tx_config[2] = {
    {
        .dst_req_sel    = UNISDK_DMA_FUNC_NUM_AUDIO0_TX,
        .src_req_sel    = 0,
        .dst_addr_ctrl  = TLK_DMA_ADDR_FIX,
        .src_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
        .dstmode        = TLK_DMA_HANDSHAKE_MODE,
        .srcmode        = TLK_DMA_NORMAL_MODE,
        .dstwidth       = TLK_DMA_WORD_WIDTH,
        .srcwidth       = TLK_DMA_WORD_WIDTH,
        .src_burst_size = 0,
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0,
    },
    {
        .dst_req_sel    = UNISDK_DMA_FUNC_NUM_AUDIO1_TX,
        .src_req_sel    = 0,
        .dst_addr_ctrl  = TLK_DMA_ADDR_FIX,
        .src_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
        .dstmode        = TLK_DMA_HANDSHAKE_MODE,
        .srcmode        = TLK_DMA_NORMAL_MODE,
        .dstwidth       = TLK_DMA_WORD_WIDTH,
        .srcwidth       = TLK_DMA_WORD_WIDTH,
        .src_burst_size = 0,
        .read_num_en    = 0,
        .priority       = 0,
        .write_num_en   = 0,
        .auto_en        = 0,
    },
};

/* Helper functions */

static void tlk_i2s_init(void)
{
    // TODO: do it once

    // The power-on sequence that must be followed
    // 1.power down audio power
    // 2.wait audio power stabilization
    // 3.switch on audio power switch
    // 4.power on audio power
    // 5.wait audio power stabilization
    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN2, (value.bit.pd_audio = 1;));
    tlk_sys_delay(6);
    TLK_ANALOG_MODIFY(TLK_AREG_POWER12, (value.bit.pd_vdd_codec_1p0v = 0;));
    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN2, (value.bit.pd_audio = 0;));
    tlk_sys_delay(6);
}

static void tlk_i2s_set_i2s_clk(struct tlk_i2s_device* dev, uint16_t div_numerator,
                                uint16_t div_denominator)
{
    reg_i2s_step(dev->number) = (div_numerator & FLD_I2S_STEP) | FLD_I2S_CLK_EN;
    reg_i2s_mod(dev->number)  = div_denominator;
}

static void tlk_i2s_set_bclk(struct tlk_i2s_device* dev, uint16_t div)
{
    reg_i2s_pcm_clk_num(dev->number) = div & 0x0f;
}

static void tlk_i2s_set_lrclk(struct tlk_i2s_device* dev, uint16_t adc_div, uint16_t dac_div)
{
    reg_i2s_int_pcm_num(dev->number) = (adc_div - 1);
    reg_i2s_dec_pcm_num(dev->number) = (dac_div - 1);
}

static void tlk_i2s_set_clock(struct tlk_i2s_device* dev, struct tlk_i2s_sample_rate* clk_config)
{
    tlk_i2s_set_i2s_clk(
        dev, clk_config->i2s_clk_div_numerator, clk_config->i2s_clk_div_denominator);
    tlk_i2s_set_bclk(dev, clk_config->bclk_div);
    tlk_i2s_set_lrclk(dev, clk_config->adc_div, clk_config->dac_div);
}

static void tlk_i2s_rx_fifo_set_mode(struct tlk_i2s_device* dev, enum tlk_i2s_fifo fifo,
                                     enum tlk_i2s_channel    channel,
                                     enum tlk_i2s_data_width data_width)
{
    const uint8_t dev_num   = dev->number;
    uint8_t       reg_value = 0b00;

    if (data_width != TLK_I2S_DATA_WIDTH_16_BIT)
    {
        reg_value |= 0b01;
    }

    if (channel == TLK_I2S_CHANNEL_STEREO)
    {
        reg_value |= 0b10;
    }

    reg_audio_i2s_tune(dev_num) =
        (fifo == TLK_I2S_FIFO0)
            ? ((reg_audio_i2s_tune(dev_num) & (~FLD_AUDIO_I2S_AIN0_COME)) | (reg_value))
            : ((reg_audio_i2s_tune(dev_num) & (~FLD_AUDIO_I2S_AIN1_COME)) | (reg_value << 2));
}

static void tlk_i2s_rx_fifo_select_path(struct tlk_i2s_device* dev, enum tlk_i2s_fifo fifo)
{
    reg_audio_sel(fifo) = (reg_audio_sel(fifo) & (~FLD_AUDIO_AIN_SEL)) | (dev->number);
}

static void tlk_i2s_tx_fifo_set_mode(struct tlk_i2s_device* dev, enum tlk_i2s_fifo fifo,
                                     enum tlk_i2s_channel    channel,
                                     enum tlk_i2s_data_width data_width)
{
    const uint8_t dev_num   = dev->number;
    uint8_t       reg_value = 0b00;

    if (data_width != TLK_I2S_DATA_WIDTH_16_BIT)
    {
        reg_value |= 0b01;
    }

    if (channel == TLK_I2S_CHANNEL_STEREO)
    {
        reg_value |= 0b10;
    }

    if (fifo == TLK_I2S_FIFO1)
    {
        reg_value |= 0b100;

        if (channel == TLK_I2S_CHANNEL_STEREO)
        {
            reg_value += 2;
            reg_value ^= 0b01;
        }
    }

    reg_audio_i2s_tune(dev_num) =
        (reg_audio_i2s_tune(dev_num) & (~FLD_AUDIO_I2S_AOUT_COME)) | (reg_value << 4);
}

static void tlk_i2s_tx_fifo_select_path(struct tlk_i2s_device* dev, enum tlk_i2s_fifo fifo)
{
    uint8_t reg_value   = dev->number == 0 ? 0 : 3;
    reg_audio_sel(fifo) = (reg_audio_sel(fifo) & (~FLD_AUDIO_AOUT_SEL)) | (reg_value << 3);
}

/* I2S driver API */

enum tlk_i2s_status tlk_i2s_configure(struct tlk_i2s_device* dev, struct tlk_i2s_config* config)
{
    if (config->mode == TLK_I2S_MODE_TDM)
    {
        return TLK_I2S_STATUS_UNSUPPORTED;
    }

    tlk_i2s_init();

    dev->data_width = config->data_width;

    if (config->role == TLK_I2S_MASTER)
    {
        tlk_i2s_set_clock(dev, &config->sample_rate);
    }

    reg_i2s_cfg1(dev->number) = TLK_MASK_VAL(FLD_AUDIO_I2S_FORMAT,
                                             config->mode,
                                             FLD_AUDIO_I2S_WL,
                                             config->data_width,
                                             FLD_AUDIO_I2S_LRP,
                                             0,
                                             FLD_AUDIO_I2S_LRSWAP,
                                             0,
                                             FLD_AUDIO_I2S_ADC_DCI_MS,
                                             config->role,
                                             FLD_AUDIO_I2S_DAC_DCI_MS,
                                             config->role);

    return TLK_I2S_STATUS_OK;
}

enum tlk_i2s_status tlk_i2s_pinmux_configure(struct tlk_i2s_device* dev,
                                             struct tlk_i2s_pinmux* pinmux)
{
    tlk_gpio_configure(pinmux->bclk.port, pinmux->bclk.pin, TLK_GPIO_INPUT_NO_PULL);
    tlk_gpio_set_mux(pinmux->bclk.port, pinmux->bclk.pin, dev->mux.bck);
    tlk_gpio_disable(pinmux->bclk.port, pinmux->bclk.pin);

    if (pinmux->adc_lr_clk.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(pinmux->adc_lr_clk.port, pinmux->adc_lr_clk.pin, TLK_GPIO_INPUT_NO_PULL);
        tlk_gpio_set_mux(pinmux->adc_lr_clk.port, pinmux->adc_lr_clk.pin, dev->mux.lr0);
        tlk_gpio_disable(pinmux->adc_lr_clk.port, pinmux->adc_lr_clk.pin);
    }

    if (pinmux->dac_lr_clk.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(pinmux->dac_lr_clk.port, pinmux->dac_lr_clk.pin, TLK_GPIO_INPUT_NO_PULL);
        tlk_gpio_set_mux(pinmux->dac_lr_clk.port, pinmux->dac_lr_clk.pin, dev->mux.lr1);
        tlk_gpio_disable(pinmux->dac_lr_clk.port, pinmux->dac_lr_clk.pin);
    }

    if (pinmux->adc_dat.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(pinmux->adc_dat.port, pinmux->adc_dat.pin, TLK_GPIO_INPUT_NO_PULL);
        tlk_gpio_set_mux(pinmux->adc_dat.port, pinmux->adc_dat.pin, dev->mux.dat0);
        tlk_gpio_disable(pinmux->adc_dat.port, pinmux->adc_dat.pin);
    }
    if (pinmux->dac_dat.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(pinmux->dac_dat.port, pinmux->dac_dat.pin, TLK_GPIO_INPUT_NO_PULL);
        tlk_gpio_set_mux(pinmux->dac_dat.port, pinmux->dac_dat.pin, dev->mux.dat1);
        tlk_gpio_disable(pinmux->dac_dat.port, pinmux->dac_dat.pin);
    }

    return TLK_I2S_STATUS_OK;
}

enum tlk_i2s_status tlk_i2s_input_configure(struct tlk_i2s_device*    dev,
                                            struct tlk_i2s_io_config* config)
{
    tlk_i2s_rx_fifo_set_mode(dev, config->fifo, config->i2s_channel, dev->data_width);
    tlk_i2s_rx_fifo_select_path(dev, config->fifo);

    tlk_dma_chn_configure(config->dma_channel, &audio_dma_rx_config[config->fifo]);

    for (struct tlk_dma_ll_node* current = config->ll_head; current != NULL;
         current                         = current->next)
    {
        current->src_addr = (void*) REG_AUDIO_FIFO_ADDR(config->fifo);

        if (current->next == config->ll_head)
        {
            break;
        }
    }

    tlk_dma_chn_transfer_configure_ll(config->dma_channel, config->ll_head);
    tlk_dma_chn_transfer_start(config->dma_channel);

    return TLK_I2S_STATUS_OK;
}

enum tlk_i2s_status tlk_i2s_configure_output(struct tlk_i2s_device*    dev,
                                             struct tlk_i2s_io_config* config)
{
    tlk_i2s_tx_fifo_set_mode(dev, config->fifo, config->i2s_channel, dev->data_width);
    tlk_i2s_tx_fifo_select_path(dev, config->fifo);

    tlk_dma_chn_configure(config->dma_channel, &audio_dma_tx_config[config->fifo]);

    for (struct tlk_dma_ll_node* current = config->ll_head; current != NULL;
         current                         = current->next)
    {
        current->dst_addr = (void*) REG_AUDIO_FIFO_ADDR(config->fifo);

        if (current->next == config->ll_head)
        {
            break;
        }
    }

    tlk_dma_chn_transfer_configure_ll(config->dma_channel, config->ll_head);
    tlk_dma_chn_transfer_start(config->dma_channel);

    return TLK_I2S_STATUS_OK;
}

void tlk_i2s_start(struct tlk_i2s_device* dev)
{
    TLK_BM_SET(reg_i2s_en(dev->number), FLD_AUDIO_I2S_CLK_EN);
}
