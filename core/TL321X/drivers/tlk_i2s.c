#include "core/include/tlk_i2s.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_power.h"
#include "registers/tlk_audio_reg.h"
#include "registers/tlk_soc.h"

struct tlk_i2s_device
{
    const uint8_t           number;
    enum tlk_i2s_data_width data_width;
};

// I2S0 and I2S1 are unavailable on this chip,
// so for compatibility, it is called I2S0 while being, in fact, I2S2.
struct tlk_i2s_device tlk_i2s0 = {
    .number = 2,
};

static struct tlk_dma_config tlk_i2s_dma_rx_config[3] = {
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
    {
        .dst_req_sel    = 0,
        .src_req_sel    = UNISDK_DMA_FUNC_NUM_AUDIO2_RX,
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

static struct tlk_dma_config tlk_i2s_dma_tx_config[3] = {
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
    {
        .dst_req_sel    = UNISDK_DMA_FUNC_NUM_AUDIO2_TX,
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
    tlk_power_set_dig_modules_state(TLK_POWER_AUDIO, true);
    TLK_BM_SET(reg_rst2, FLD_RST2_AUDIO);
    TLK_BM_SET(reg_clk_en2, FLD_CLK2_AUDIO_EN);

    reg_dmic_step = (1 & FLD_DMIC_STEP) | FLD_DMIC_SEL;
    reg_dmic_mod  = tlk_sys_clk.source_clk / 24;
}

static void tlk_i2s_set_i2s_clk(uint16_t div_numerator, uint16_t div_denominator)
{
    reg_i2s_step = (div_numerator & FLD_I2S_STEP) | FLD_I2S_CLK_EN_0;
    reg_i2s_mod  = div_denominator;
}

static void tlk_i2s_set_bclk(struct tlk_i2s_device* dev, uint16_t div)
{
    reg_i2s_pcm_clk_num(dev->number) = div;
}

static void tlk_i2s_set_lrclk(struct tlk_i2s_device* dev, uint16_t adc_div, uint16_t dac_div)
{
    reg_i2s_int_pcm_num(dev->number) = (adc_div - 1);
    reg_i2s_dec_pcm_num(dev->number) = (dac_div - 1);
}

static void tlk_i2s_enable_src_clk(void)
{
    reg_i2s_step |= FLD_I2S_CLK_EN_0;
}

static void tlk_i2s_set_clock(struct tlk_i2s_device* dev, struct tlk_i2s_sample_rate* clk_config)
{
    tlk_i2s_set_i2s_clk(clk_config->i2s_clk_div_numerator, clk_config->i2s_clk_div_denominator);
    tlk_i2s_set_bclk(dev, clk_config->bclk_div);
    tlk_i2s_set_lrclk(dev, clk_config->adc_div, clk_config->dac_div);
}

static void tlk_i2s_fifo_set_mono_channel(struct tlk_i2s_device* dev, enum tlk_i2s_fifo fifo,
                                          enum tlk_i2s_channel channel)
{
    const uint8_t dev_num = dev->number;

    switch (channel)
    {
    case TLK_I2S_CHANNEL_LEFT:
        reg_mono_rxfifo_sel(dev_num) =
            (reg_mono_rxfifo_sel(dev_num) & (~FLD_I2S2_MONOL_RXFIFO_SEL)) |
            TLK_MASK_VAL(FLD_I2S2_MONOL_RXFIFO_SEL, fifo);
        break;
    case TLK_I2S_CHANNEL_RIGHT:
        reg_mono_rxfifo_sel(dev_num) =
            (reg_mono_rxfifo_sel(dev_num) & (~FLD_I2S2_MONOR_RXFIFO_SEL)) |
            TLK_MASK_VAL(FLD_I2S2_MONOR_RXFIFO_SEL, fifo);
        break;
    default:
        break;
    }
}

static void tlk_i2s_rx_fifo_enable(enum tlk_i2s_fifo fifo)
{
    reg_rxfifo_en |= TLK_BIT(fifo);
}

static void tlk_i2s_rx_fifo_wptr_enable(enum tlk_i2s_fifo fifo)
{
    reg_rx_wptr_en |= TLK_BIT(fifo);
}

static void tlk_i2s_rx_fifo_set_mode(enum tlk_i2s_fifo fifo, enum tlk_i2s_channel channel,
                                     enum tlk_i2s_data_width data_width)
{
    uint8_t reg_value = 0b00;

    if (data_width != TLK_I2S_DATA_WIDTH_16_BIT)
    {
        reg_value |= 0b01;
    }

    if (channel == TLK_I2S_CHANNEL_STEREO)
    {
        reg_value |= 0b10;
    }

    switch (fifo)
    {
    case TLK_I2S_FIFO0:
        reg_i2s_ain_mode =
            (reg_i2s_ain_mode & (~FLD_I2S_AIN0_MODE)) | TLK_MASK_VAL(FLD_I2S_AIN0_MODE, reg_value);
        break;
    case TLK_I2S_FIFO1:
        reg_i2s_ain_mode =
            (reg_i2s_ain_mode & (~FLD_I2S_AIN1_MODE)) | TLK_MASK_VAL(FLD_I2S_AIN1_MODE, reg_value);
        break;
    case TLK_I2S_FIFO2:
        reg_i2s_ain_mode =
            (reg_i2s_ain_mode & (~FLD_I2S_AIN2_MODE)) | TLK_MASK_VAL(FLD_I2S_AIN2_MODE, reg_value);
        break;
    default:
        break;
    }
}

static void tlk_i2s_rx_fifo_select_path(struct tlk_i2s_device* dev, enum tlk_i2s_fifo fifo)
{
    const uint8_t dev_num = dev->number;

    if (TLK_I2S_FIFO0 == fifo)
    {
        reg_fifoin0_sel = (reg_fifoin0_sel & (~FLD_AIN0_SEL)) | dev_num;
    }
    else if (TLK_I2S_FIFO1 == fifo)
    {
        reg_fifoin12_sel = (reg_fifoin12_sel & (~FLD_AIN1_SEL)) | dev_num;
    }
    else
    {
        reg_fifoin12_sel = (reg_fifoin12_sel & (~FLD_AIN2_SEL)) | (dev_num << 4);
    }
}

static void tlk_i2s_tx_fifo_enable(enum tlk_i2s_fifo fifo)
{
    reg_fifo_outen |= TLK_BIT(fifo);
}

static void tlk_i2s_tx_fifo_wptr_enable(enum tlk_i2s_fifo fifo)
{
    reg_rx_wptr_en |= TLK_BIT(fifo + 4);
}

static void tlk_i2s_tx_fifo_set_mode(enum tlk_i2s_fifo fifo, enum tlk_i2s_channel channel,
                                     enum tlk_i2s_data_width data_width)
{
    uint8_t reg_value = 0b00;

    if (data_width != TLK_I2S_DATA_WIDTH_16_BIT)
    {
        reg_value |= 0b01;
    }

    if (channel == TLK_I2S_CHANNEL_STEREO)
    {
        reg_value |= 0b10;
    }

    switch (fifo)
    {
    case TLK_I2S_FIFO0:
        reg_i2s_aout_mode = (reg_i2s_aout_mode & (~FLD_I2S0_AOUT_MODE)) |
                            TLK_MASK_VAL(FLD_I2S0_AOUT_MODE, reg_value);
        break;
    case TLK_I2S_FIFO1:
        reg_i2s_aout_mode = (reg_i2s_aout_mode & (~FLD_I2S1_AOUT_MODE)) |
                            TLK_MASK_VAL(FLD_I2S1_AOUT_MODE, reg_value);
        break;
    case TLK_I2S_FIFO2:
        reg_i2s_aout_mode = (reg_i2s_aout_mode & (~FLD_I2S2_AOUT_MODE)) |
                            TLK_MASK_VAL(FLD_I2S2_AOUT_MODE, reg_value);
        break;
    default:
        break;
    }
}

/* I2S driver API */

enum tlk_i2s_status tlk_i2s_configure(struct tlk_i2s_device* dev, struct tlk_i2s_config* config)
{
    tlk_i2s_init();

    dev->data_width = config->data_width;

    if (config->role == TLK_I2S_MASTER)
    {
        tlk_i2s_set_clock(dev, &config->sample_rate);
    }
    else
    {
        tlk_i2s_enable_src_clk();
    }

    reg_i2s_cfg1(dev->number) =
        (reg_i2s_cfg1(dev->number) &
         (~(FLD_I2S_ADC_DCI_MS | FLD_I2S_DAC_DCI_MS | FLD_I2S_ADC_FRM_LOOP))) |
        TLK_MASK_VAL(FLD_I2S_ADC_DCI_MS, config->role, FLD_I2S_DAC_DCI_MS, config->role);

    reg_i2s_cfg2(dev->number) =
        (reg_i2s_cfg2(dev->number) &
         (~(FLD_I2S_WL | I2S_FORMAT | FLD_I2S_ADC_MBCLK_LOOP | FLD_I2S_DAC_MBCLK_LOOP))) |
        TLK_MASK_VAL(FLD_I2S_WL, config->data_width, I2S_FORMAT, config->mode);

    reg_i2s_cfg3(dev->number) =
        (reg_i2s_cfg3(dev->number) & (~(FLD_I2S_LRP | FLD_I2S_LRSWAP | FLD_I2S_DAC_FRM_LOOP))) |
        TLK_MASK_VAL(FLD_I2S_LRP, 0, FLD_I2S_LRSWAP, 0);

    return TLK_I2S_STATUS_OK;
}

enum tlk_i2s_status tlk_i2s_pinmux_configure(struct tlk_i2s_device* dev,
                                             struct tlk_i2s_pinmux* pinmux)
{
    // Only one I2S device is available.
    (void) dev;

    tlk_gpio_configure(pinmux->bclk.port, pinmux->bclk.pin, TLK_GPIO_INPUT_NO_PULL);
    tlk_gpio_set_mux(pinmux->bclk.port, pinmux->bclk.pin, TLK_GPIO_MUX_I2S_BCK);
    tlk_gpio_disable(pinmux->bclk.port, pinmux->bclk.pin);

    if (pinmux->adc_lr_clk.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(pinmux->adc_lr_clk.port, pinmux->adc_lr_clk.pin, TLK_GPIO_INPUT_NO_PULL);
        tlk_gpio_set_mux(pinmux->adc_lr_clk.port, pinmux->adc_lr_clk.pin, TLK_GPIO_MUX_I2S_LR0);
        tlk_gpio_disable(pinmux->adc_lr_clk.port, pinmux->adc_lr_clk.pin);
    }

    if (pinmux->dac_lr_clk.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(pinmux->dac_lr_clk.port, pinmux->dac_lr_clk.pin, TLK_GPIO_INPUT_NO_PULL);
        tlk_gpio_set_mux(pinmux->dac_lr_clk.port, pinmux->dac_lr_clk.pin, TLK_GPIO_MUX_I2S_LR1);
        tlk_gpio_disable(pinmux->dac_lr_clk.port, pinmux->dac_lr_clk.pin);
    }

    if (pinmux->adc_dat.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(pinmux->adc_dat.port, pinmux->adc_dat.pin, TLK_GPIO_INPUT_NO_PULL);
        tlk_gpio_set_mux(pinmux->adc_dat.port, pinmux->adc_dat.pin, TLK_GPIO_MUX_I2S_DAT0);
        tlk_gpio_disable(pinmux->adc_dat.port, pinmux->adc_dat.pin);
    }
    if (pinmux->dac_dat.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(pinmux->dac_dat.port, pinmux->dac_dat.pin, TLK_GPIO_INPUT_NO_PULL);
        tlk_gpio_set_mux(pinmux->dac_dat.port, pinmux->dac_dat.pin, TLK_GPIO_MUX_I2S_DAT1);
        tlk_gpio_disable(pinmux->dac_dat.port, pinmux->dac_dat.pin);
    }

    return TLK_I2S_STATUS_OK;
}

enum tlk_i2s_status tlk_i2s_input_configure(struct tlk_i2s_device*    dev,
                                            struct tlk_i2s_io_config* config)
{
    if (config->i2s_channel != TLK_I2S_CHANNEL_STEREO)
    {
        tlk_i2s_fifo_set_mono_channel(dev, config->fifo, config->i2s_channel);
    }
    tlk_i2s_rx_fifo_enable(config->fifo);
    tlk_i2s_rx_fifo_wptr_enable(config->fifo);
    tlk_i2s_rx_fifo_set_mode(config->fifo, config->i2s_channel, dev->data_width);
    tlk_i2s_rx_fifo_select_path(dev, config->fifo);

    tlk_dma_chn_configure(config->dma_channel, &tlk_i2s_dma_rx_config[config->fifo]);

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
    // TX FIFOs are hardwired to the corresponding I2S instances.
    (void) dev;
    config->fifo = TLK_I2S_FIFO2;

    tlk_i2s_tx_fifo_enable(config->fifo);
    tlk_i2s_tx_fifo_wptr_enable(config->fifo);
    tlk_i2s_tx_fifo_set_mode(config->fifo, config->i2s_channel, dev->data_width);

    tlk_dma_chn_configure(config->dma_channel, &tlk_i2s_dma_tx_config[config->fifo]);

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
    TLK_BM_SET(reg_i2s_cfg1(dev->number), FLD_I2S_CLK_EN);
}
