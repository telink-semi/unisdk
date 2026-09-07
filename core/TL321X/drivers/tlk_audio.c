#include "core/include/tlk_audio.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_i2s.h"
#include "core/include/tlk_power.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_audio_reg.h"
#include "registers/tlk_soc.h"

typedef enum
{
    ASCL0 = 0x02,
    ASCL2 = 0x01,
} tlk_audio_ascl_select_e;

static struct tlk_dma_config tlk_audio_dma_rx_config[3] = {
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

static struct tlk_dma_config tlk_audio_dma_tx_config[3] = {
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

static uint8_t tlk_audio_sample_rate_to_reg_table[] = {
    0x06,
    /*8k*/ // 12Mhz/1500=8K
    0x17,
    /*8p0214k*/ // 12Mhz/1496=8.0214K
    0x19,
    /*11.0259k*/ // 12Mhz/1088=11.0259K
    0x08,
    /*12k*/ // 12Mhz/1000=12K
    0x0a,
    /*16k*/ // 12Mhz/750=16K
    0x1b,
    /*22.0588k*/ // 12Mhz/544=22.0588K
    0x1c,
    /*24k*/ // 12Mhz/500=24K
    0x0c,
    /*32k*/ // 12Mhz/375=32K
    0x11,
    /*44.118k_clk6mhz*/ // 12Mhz/272=44.118K
    0x00,
    /*48k_clk6mhz*/ // 12Mhz/250=48K
    0x0d,           // 32k_clk2mhz
    0x1f,           // 44.118k_clk3mhz
    0x1e,           // 48k_clk3mhz
};

static uint32_t tlk_audio_sample_rate_to_stereo_step_reg_table[] = {
    0x00831001, /*8k*/
    0x00837001, /*8p0214k*/
    0x00b4a001, /*11.0259k*/
    0x00c4a001, /*12k*/
    0x01062001, /*16k*/
    0x01696001, /*22.0588k*/
    0x01893001, /*24k*/
    0x020c5001, /*32k*/
    0x02d28001, /*44.1k*/
    0x03127001, /*48k*/
};

static uint32_t tlk_audio_sample_rate_to_mono_step_reg_table[] = {
    0x01062001, /*8k*/
    0x0106D001, /*8p0214k*/
    0x01694001, /*11.0259k*/
    0x01894001, /*12k*/
    0x020c5001, /*16k*/
    0x02d2c001, /*22.0588k*/
    0x03126001, /*24k*/
    0x0418a801, /*32k*/
    0x05a50001, /*44.1k*/
    0x0624e801, /*48k*/
};

/* Helper functions */

static void tlk_audio_adc_set_clock(void)
{
    reg_codec_cfg1 |= FLD_R_CK_SEL; // b1'0: adc data clk to adc core clk ratio 6:1, b1'1: adc data
                                    // clk to adc core clk ratio 1:1;
}

static void tlk_audio_adc_power_on(void)
{
    TLK_ANALOG_MODIFY(TLK_AREG_PGA_CTRL2, (value.bit.l_lp = 0;));
    TLK_ANALOG_MODIFY(TLK_AREG_PGA_CTRL1, (value.bit.l_pd_vmid = 0;));
    TLK_ANALOG_MODIFY(TLK_AREG_XO_ADC_PGA_CTRL, (value.bit.l_dem_en = 1;));
    TLK_ANALOG_MODIFY(TLK_AREG_PGA_CTRL1, (
        value.bit.l_filtcap_ctl = 0;
        value.bit.l_pd_pga = 0;
        value.bit.l_pd_bias = 0;
        value.bit.l_pd_adc = 0;
    ));
}

static void tlk_audio_input_set_mode(enum tlk_audio_input_source src)
{
    reg_codec_vol =
        (reg_codec_vol & (~FLD_MIC_SEL)) | TLK_MASK_VAL(FLD_MIC_SEL, src == TLK_AUDIO_INPUT_DMIC);
}

static void tlk_audio_set_path(enum tlk_i2s_channel channel)
{
    reg_codec_cfg = (reg_codec_cfg & (~(FLD_L_CH_EN | FLD_R_CH_EN))) |
                    TLK_MASK_VAL(FLD_L_CH_EN,
                                 (channel & TLK_I2S_CHANNEL_LEFT) ? (0x01) : (0x00),
                                 FLD_R_CH_EN,
                                 (channel & TLK_I2S_CHANNEL_RIGHT) ? (0x01) : (0x00));
}

static void tlk_audio_set_sample_rate(enum tlk_audio_input_source source,
                                      enum tlk_audio_sample_rate  rate)
{
    if (source == TLK_AUDIO_INPUT_DMIC)
    {
        /* When the sampling rate is greater than or equal to 32K there is a data channel reversal
         * problem, so reg_codec_cfg bit[5] need to be configured to adjust the direction, when
         * sampling rate is less than 32K there is no reversal problem, so clear the bit.
         */
        if (rate >= TLK_AUDIO_32K)
        {
            rate += 3;
            reg_codec_cfg |= FLD_R_NEG;
        }
        else
        {
            reg_codec_cfg &= ~FLD_R_NEG;
        }
    }

    reg_codec_clkcfg =
        (reg_codec_clkcfg & (~FLD_CLK_SR)) | (tlk_audio_sample_rate_to_reg_table[rate] << 0x01);
}

static void tlk_audio_set_adc_pga_gain(uint8_t pga_gain)
{
    TLK_ANALOG_MODIFY(TLK_AREG_PGA_CTRL0, (value.bit.l_vol = pga_gain;));
}

static void tlk_audio_set_dig_gain(uint8_t d_gain)
{
    reg_codec_vol = ((reg_codec_vol & (~FLD_DEC_VOL)) | d_gain);
}

static void tlk_audio_rx_fifo_enable(enum tlk_i2s_fifo fifo)
{
    reg_rxfifo_en |= TLK_BIT(fifo);
}

static void tlk_audio_rx_fifo_wptr_enable(enum tlk_i2s_fifo fifo)
{
    reg_rx_wptr_en |= TLK_BIT(fifo);
}

static void tlk_audio_rx_fifo_set_mode(enum tlk_i2s_fifo fifo, enum tlk_i2s_channel channel,
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
        reg_dec_ain_mode =
            (reg_dec_ain_mode & (~FLD_I2S_AIN0_MODE)) | TLK_MASK_VAL(FLD_I2S_AIN0_MODE, reg_value);
        break;
    case TLK_I2S_FIFO1:
        reg_dec_ain_mode =
            (reg_dec_ain_mode & (~FLD_I2S_AIN1_MODE)) | TLK_MASK_VAL(FLD_I2S_AIN1_MODE, reg_value);
        break;
    case TLK_I2S_FIFO2:
        reg_dec_ain_mode =
            (reg_dec_ain_mode & (~FLD_I2S_AIN2_MODE)) | TLK_MASK_VAL(FLD_I2S_AIN2_MODE, reg_value);
        break;
    default:
        break;
    }
}

static void tlk_audio_fifo_set_mono_channel(enum tlk_i2s_fifo fifo, enum tlk_i2s_channel channel)
{
    const uint8_t dev_num = 0;

    switch (channel)
    {
    case TLK_I2S_CHANNEL_LEFT:
        reg_mono_rxfifo_sel(dev_num) = (reg_mono_rxfifo_sel(dev_num) & (~FLD_DECL_RXFIFO_SEL)) |
                                       TLK_MASK_VAL(FLD_DECL_RXFIFO_SEL, fifo);
        break;
    case TLK_I2S_CHANNEL_RIGHT:
        reg_mono_rxfifo_sel(dev_num) = (reg_mono_rxfifo_sel(dev_num) & (~FLD_DECR_RXFIFO_SEL)) |
                                       TLK_MASK_VAL(FLD_DECR_RXFIFO_SEL, fifo);
        break;
    default:
        break;
    }
}

static void tlk_audio_set_sdm_clk(uint16_t div_numerator, uint16_t div_denominator)
{
    reg_sdm_step = (div_numerator & FLD_SDM_STEP) | FLD_SDM_CLK_EN_0;
    reg_sdm_mod  = div_denominator;
}

static void tlk_audio_ascl_lerp_en(tlk_audio_ascl_select_e ascl_select)
{
    if (ascl_select != ASCL0)
    {
        TLK_BM_SET(reg_line(ascl_select), FLD_LINE_1);
    }
    else
    {
        TLK_BM_SET(reg_aud_ctrl, FLD_LINE_0);
    }
}

static void tlk_audio_set_ascl_channel(tlk_audio_ascl_select_e ascl_select, bool is_stereo)
{
    if (ascl_select != ASCL0)
    {
        if (!is_stereo)
        {
            TLK_BM_SET(reg_ascl_config(ascl_select), FLD_MONO_1);
        }
        else
        {
            TLK_BM_CLR(reg_ascl_config(ascl_select), FLD_MONO_1);
        }
    }
    else
    {
        if (!is_stereo)
        {
            TLK_BM_SET(reg_aud_en, FLD_MONO);
        }
        else
        {
            TLK_BM_CLR(reg_aud_en, FLD_MONO);
        }
    }
}

static void tlk_audio_set_ascl_tune_step(tlk_audio_ascl_select_e ascl_select, uint32_t value)
{
    if (ascl_select != ASCL0)
    {
        reg_step_tune_1(ascl_select) = value;
    }
    else
    {
        reg_step_tune_0 = value;
    }
}

static void tlk_audio_set_output_sample_rate(enum tlk_i2s_channel       channel,
                                             enum tlk_audio_sample_rate rate)
{
    const uint32_t* const step_index[2] = {
        tlk_audio_sample_rate_to_mono_step_reg_table,
        tlk_audio_sample_rate_to_stereo_step_reg_table,
    };
    const bool is_stereo = channel == TLK_I2S_CHANNEL_STEREO;

    tlk_audio_set_ascl_channel(ASCL0, is_stereo);
    tlk_audio_set_ascl_tune_step(ASCL0, step_index[is_stereo][rate]);
}

static void tlk_audio_set_ascl_gain(tlk_audio_ascl_select_e ascl_select, uint8_t gain)
{
    if (ascl_select != ASCL0)
    {
        reg_ascl_vol(ascl_select) =
            ((reg_ascl_vol(ascl_select) & (~FLD_VOL_1)) | TLK_MASK_VAL(FLD_VOL_1, gain));
    }
    else
    {
        reg_aud_vol = ((reg_aud_vol & (~FLD_VOL_0)) | TLK_MASK_VAL(FLD_VOL_0, gain));
    }
}

static void tlk_audio_tx_fifo_enable(enum tlk_i2s_fifo fifo)
{
    reg_fifo_outen |= TLK_BIT(fifo);
}

static void tlk_audio_tx_fifo_wptr_enable(enum tlk_i2s_fifo fifo)
{
    reg_rx_wptr_en |= TLK_BIT(fifo + 4);
}

static void tlk_audio_set_sdm_path(enum tlk_i2s_channel chn)
{
    reg_aud_en = (reg_aud_en & (~FLD_MONO)) | TLK_MASK_VAL(FLD_MONO, chn != TLK_I2S_CHANNEL_STEREO);
}

static void tlk_audio_tx_fifo_set_mode(enum tlk_i2s_fifo fifo, enum tlk_i2s_channel channel,
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

static void tlk_audio_rx_fifo_select_path(enum tlk_i2s_fifo fifo)
{
    const uint8_t dev_num = 3; // codec

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

/* Audio driver API */

void tlk_audio_init(void)
{
    tlk_power_set_dig_modules_state(TLK_POWER_AUDIO, true);
    TLK_BM_SET(reg_rst2, FLD_RST2_AUDIO);
    TLK_BM_SET(reg_clk_en2, FLD_CLK2_AUDIO_EN);

    reg_dmic_step = (1 & FLD_DMIC_STEP) | FLD_DMIC_SEL;
    reg_dmic_mod  = tlk_sys_clk.source_clk / 24;
}

void tlk_audio_amic_pinmux_configure(struct tlk_audio_amic_pinmux* mux)
{
    tlk_gpio_configure(mux->bias.port, mux->bias.pin, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(mux->bias.port, mux->bias.pin, 1);
}

void tlk_audio_dmic_pinmux_configure(enum tlk_audio_input_stream   stream,
                                     struct tlk_audio_dmic_pinmux* mux)
{
    (void) stream;

    tlk_gpio_configure(mux->data.port, mux->data.pin, TLK_GPIO_INPUT_NO_PULL);
    tlk_gpio_set_mux(mux->data.port, mux->data.pin, TLK_GPIO_MUX_DMIC_DAT);
    tlk_gpio_disable(mux->data.port, mux->data.pin);

    tlk_gpio_set_mux(mux->clk1.port, mux->clk1.pin, TLK_GPIO_MUX_DMIC_CLK);
    tlk_gpio_disable(mux->clk1.port, mux->clk1.pin);

    if (mux->clk2.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_set_mux(mux->clk2.port, mux->clk2.pin, TLK_GPIO_MUX_DMIC_CLK);
        tlk_gpio_disable(mux->clk2.port, mux->clk2.pin);
    }
}

void tlk_audio_sdm_pinmux_configure(struct tlk_audio_sdm_pinmux* mux)
{
    if (mux->sdm0_p.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_set_mux(mux->sdm0_p.port, mux->sdm0_p.pin, TLK_GPIO_MUX_SDM0_P);
        tlk_gpio_disable(mux->sdm0_p.port, mux->sdm0_p.pin);
    }

    if (mux->sdm0_n.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_set_mux(mux->sdm0_n.port, mux->sdm0_n.pin, TLK_GPIO_MUX_SDM0_N);
        tlk_gpio_disable(mux->sdm0_n.port, mux->sdm0_n.pin);
    }

    if (mux->sdm1_p.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_set_mux(mux->sdm1_p.port, mux->sdm1_p.pin, TLK_GPIO_MUX_SDM1_P);
        tlk_gpio_disable(mux->sdm1_p.port, mux->sdm1_p.pin);
    }

    if (mux->sdm1_n.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_set_mux(mux->sdm1_n.port, mux->sdm1_n.pin, TLK_GPIO_MUX_SDM1_N);
        tlk_gpio_disable(mux->sdm1_n.port, mux->sdm1_n.pin);
    }
}

void tlk_audio_input_configure(struct tlk_audio_input* cfg)
{
    reg_codec_clkcfg |= TLK_MASK_VAL(FLD_CLK_USB, 0x01, FLD_CLK_DIV2, 0x01); // audio_codec_init

    if (cfg->source != TLK_AUDIO_INPUT_DMIC)
    {
        tlk_audio_adc_set_clock();
        tlk_audio_adc_power_on();
    }

    tlk_audio_input_set_mode(cfg->source);
    tlk_audio_set_path(cfg->audio_channel);
    tlk_audio_set_sample_rate(cfg->source, cfg->sample_rate);

    tlk_audio_set_adc_pga_gain(15); // 0dB
    tlk_audio_set_dig_gain(16);     //-24dB

    tlk_audio_rx_fifo_enable(cfg->fifo);
    tlk_audio_rx_fifo_wptr_enable(cfg->fifo);
    if (cfg->audio_channel != TLK_I2S_CHANNEL_STEREO)
    {
        tlk_audio_fifo_set_mono_channel(cfg->fifo, cfg->audio_channel);
    }
    tlk_audio_rx_fifo_set_mode(cfg->fifo, cfg->audio_channel, cfg->data_width);

    tlk_dma_chn_configure(cfg->dma_channel, &tlk_audio_dma_rx_config[cfg->fifo]);

    for (struct tlk_dma_ll_node* current = cfg->ll_head; current != NULL; current = current->next)
    {
        current->src_addr = (void*) REG_AUDIO_FIFO_ADDR(cfg->fifo);

        if (current->next == cfg->ll_head)
        {
            break;
        }
    }

    tlk_dma_chn_transfer_configure_ll(cfg->dma_channel, cfg->ll_head);
    tlk_dma_chn_transfer_start(cfg->dma_channel);
}

void tlk_audio_output_configure(struct tlk_audio_output* cfg)
{
    tlk_audio_set_sdm_clk(1, tlk_sys_clk.source_clk / 2);
    tlk_audio_ascl_lerp_en(ASCL0);
    tlk_audio_set_output_sample_rate(cfg->audio_channel, cfg->sample_rate);
    tlk_audio_set_ascl_gain(ASCL0, 0x40); // 0dB

    cfg->fifo = TLK_I2S_FIFO0;

    tlk_audio_tx_fifo_enable(cfg->fifo);
    tlk_audio_tx_fifo_wptr_enable(cfg->fifo);
    tlk_audio_set_sdm_path(cfg->audio_channel);
    tlk_audio_tx_fifo_set_mode(cfg->fifo, cfg->audio_channel, cfg->data_width);

    tlk_audio_rx_fifo_select_path(cfg->fifo); // TODO: check

    tlk_dma_chn_configure(cfg->dma_channel, &tlk_audio_dma_tx_config[cfg->fifo]);

    for (struct tlk_dma_ll_node* current = cfg->ll_head; current != NULL; current = current->next)
    {
        current->dst_addr = (void*) REG_AUDIO_FIFO_ADDR(cfg->fifo);

        if (current->next == cfg->ll_head)
        {
            break;
        }
    }

    tlk_dma_chn_transfer_configure_ll(cfg->dma_channel, cfg->ll_head);
    tlk_dma_chn_transfer_start(cfg->dma_channel);
}

void tlk_audio_input_enable(void)
{
    TLK_BM_SET(reg_codec_rst, FLD_EN_DEC);
    TLK_BM_SET(reg_codec_clkcfg, FLD_CODEC_CLK_EN);
}

void tlk_audio_output_enable(void)
{
    TLK_BM_SET(reg_aud_en, FLD_SDMPEN);
}

void tlk_audio_input_disable(void)
{
    TLK_BM_CLR(reg_codec_rst, FLD_EN_DEC);
    TLK_BM_CLR(reg_codec_clkcfg, FLD_CODEC_CLK_EN);
}

void tlk_audio_output_disable(void)
{
    TLK_BM_CLR(reg_aud_en, FLD_SDMPEN);
}
