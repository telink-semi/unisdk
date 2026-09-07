#include "core/include/tlk_audio.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_i2s.h"
#include "core/include/tlk_sys.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_audio_reg.h"
#include "registers/tlk_soc.h"

typedef enum
{
    MICBIAS_NORMAL_1V6_MODE,
    MICBIAS_NORMAL_1V8_MODE,
    MICBIAS_AVD_3V3_MODE,
} tlk_audio_micbias_work_mode_e;

typedef enum
{
    CURR_5P0_UA  = TLK_BIT(2) | 3, // 5.0uA
    CURR_3P8_UA  = TLK_BIT(2) | 0, // 3.8uA
    CURR_3P07_UA = 3,              // 3.07uA
} codec_lbias_curr_sel_e;

typedef enum
{
    HP_MODE,  // high performance
    NML_MODE, // normal performance
    LP_MODE,  // low performance
} codec_dac_mode_e;

uint8_t tlk_audio_sample_rate_to_reg_table[] = {
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
};

struct tlk_dma_config tlk_audio_dma_rx_config[2] = {
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

struct tlk_dma_config tlk_audio_dma_tx_config[2] = {
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

static void tlk_audio_power_on(void)
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

static void tlk_audio_set_clk(uint16_t div_numerator, uint16_t div_denominator)
{
    reg_dmic_step = (div_numerator & FLD_DMIC_STEP) | FLD_DMIC_SEL;
    reg_dmic_mod  = div_denominator;
}

static void tlk_audio_set_adc_clock(void)
{
    reg_codec_adc_1m_clk_ctr |=
        FLD_AUDIO_CODEC_CLK_1M_SEL | FLD_AUDIO_CODEC_CLK_1M_EN; // set 1M clk  for adc
}

static void tlk_audio_set_micbias(bool en, tlk_audio_micbias_work_mode_e micbias_mode)
{
    if (en)
    {
        if (micbias_mode == MICBIAS_AVD_3V3_MODE)
        {
            tlk_analog_write_reg8(
                0x26,
                (tlk_analog_read_reg8(0x26) & (~((TLK_BIT(0) | TLK_BIT(1) | TLK_BIT(4))))) |
                    TLK_MASK_VAL(TLK_BIT(0), 1, TLK_BIT(1), 0, TLK_BIT(4), 1));
        }
        else if (micbias_mode == MICBIAS_NORMAL_1V6_MODE)
        {
            tlk_analog_write_reg8(
                0x26,
                (tlk_analog_read_reg8(0x26) & (~((TLK_BIT(0) | TLK_BIT(1) | TLK_BIT(4))))) |
                    TLK_MASK_VAL(TLK_BIT(0), 0, TLK_BIT(1), 0, TLK_BIT(4), 0));
        }
        else if (micbias_mode == MICBIAS_NORMAL_1V8_MODE)
        {
            tlk_analog_write_reg8(
                0x26,
                (tlk_analog_read_reg8(0x26) & (~((TLK_BIT(0) | TLK_BIT(1) | TLK_BIT(4))))) |
                    TLK_MASK_VAL(TLK_BIT(0), 1, TLK_BIT(1), 0, TLK_BIT(4), 0));
        }
    }
    else
    {
        tlk_analog_write_reg8(0x26, tlk_analog_read_reg8(0x26) | TLK_BIT(1));
    }
    /*0x26<0>:mic bias voltage select,                     default:1, 0:micbias voltage=1.6V,
       1:micbias voltage=1.8V <1>:pd_micbias                                   default:1, 0:micbias
       up                  1:micbias down <4>:bypass micbias, micbias out= IO supply 3.3V,
       default:0, 0:micbias normal working mode 1:micbias bypass(VAD mode)
    */
}

static void tlk_audio_adc_power_on(enum tlk_i2s_channel channel, bool micbias_power_en)
{
    tlk_audio_set_micbias(micbias_power_en, MICBIAS_NORMAL_1V6_MODE);
    tlk_analog_write_reg8(0x1a, tlk_analog_read_reg8(0x1a) & ~TLK_BIT(0)); // pd_LDO_adc=0(power on)

    bool reg_val_left  = !(channel & TLK_I2S_CHANNEL_LEFT);
    bool reg_val_right = !(channel & TLK_I2S_CHANNEL_RIGHT);

    reg_codec_ana_reg1 = (reg_codec_ana_reg1 &
                          (~(FLD_AUDIO_PD_PGABUF | FLD_AUDIO_PD_PGABOOST | FLD_AUDIO_PD_INPPGA))) |
                         TLK_MASK_VAL(FLD_AUDIO_PD_PGABUF,
                                      reg_val_left,
                                      FLD_AUDIO_PD_PGABOOST,
                                      reg_val_left,
                                      FLD_AUDIO_PD_INPPGA,
                                      reg_val_left);
    reg_codec_ana_reg2 = (reg_codec_ana_reg2 & (~(FLD_AUDIO_PD_ASDM))) |
                         TLK_MASK_VAL(FLD_AUDIO_PD_ASDM, reg_val_left);

    reg_codec_ana_reg12 = (reg_codec_ana_reg12 &
                           (~(FLD_AUDIO_PD_PGABUF | FLD_AUDIO_PD_PGABOOST | FLD_AUDIO_PD_INPPGA))) |
                          TLK_MASK_VAL(FLD_AUDIO_PD_PGABUF,
                                       reg_val_right,
                                       FLD_AUDIO_PD_PGABOOST,
                                       reg_val_right,
                                       FLD_AUDIO_PD_INPPGA,
                                       reg_val_right);
    reg_codec_ana_reg13 = (reg_codec_ana_reg13 & (~(FLD_AUDIO_PD_ASDM))) |
                          TLK_MASK_VAL(FLD_AUDIO_PD_ASDM, reg_val_right);
}

static void tlk_audio_set_input_mode(enum tlk_audio_input_source src)
{
    /**
     * Enabling FLD_AUDIO_CODEC_R_NEG bit is valid for both stream0 and stream1.
     * If this bit is 0 and the DMIC sampling rate is 44.1k and 48k,the left and right channel data
     * will be exchanged.
     **/
    reg_codec_mic_ctr = (reg_codec_mic_ctr & (~FLD_AUDIO_CODEC_MIC_SEL)) | FLD_AUDIO_CODEC_R_NEG |
                        (((src == TLK_AUDIO_INPUT_DMIC) ? 1 : 0) << 3);
}

static void tlk_audio_set_path(enum tlk_audio_input_stream stream, enum tlk_audio_input_source src,
                               enum tlk_i2s_channel channel, enum tlk_audio_sample_rate rate)

{
    /* at 44.1k, 48k sampling rate, the DMCI channel will be switched (amic, line_in does not
     * affect), and the switch here restores the original channel correspondence
     */

    if (src == TLK_AUDIO_INPUT_DMIC && (rate == TLK_AUDIO_44P118K || rate == TLK_AUDIO_48K))
    {
        if (channel == TLK_I2S_CHANNEL_LEFT)
        {
            channel = TLK_I2S_CHANNEL_RIGHT;
        }
        else if (channel == TLK_I2S_CHANNEL_RIGHT)
        {
            channel = TLK_I2S_CHANNEL_LEFT;
        }
    }

    if (stream == TLK_AUDIO_INPUT_STREAM0)
    {
        reg_codec_mic_ctr =
            (reg_codec_mic_ctr & (~(FLD_AUDIO_CODEC_DEC0_CH0_EN | FLD_AUDIO_CODEC_DEC0_CH1_EN))) |
            TLK_MASK_VAL(FLD_AUDIO_CODEC_DEC0_CH0_EN,
                         (channel & TLK_I2S_CHANNEL_LEFT) ? 1 : 0,
                         FLD_AUDIO_CODEC_DEC0_CH1_EN,
                         (channel & TLK_I2S_CHANNEL_RIGHT) ? 1 : 0);
    }
    else
    {
        reg_codec_dec1_ctr0 =
            (reg_codec_dec1_ctr0 & (~(FLD_AUDIO_CODEC_DEC1_CH0_EN | FLD_AUDIO_CODEC_DEC1_CH1_EN))) |
            TLK_MASK_VAL(FLD_AUDIO_CODEC_CLK1_EN,
                         1,
                         FLD_AUDIO_CODEC_DEC1_CH0_EN,
                         (channel & TLK_I2S_CHANNEL_LEFT) ? 1 : 0,
                         FLD_AUDIO_CODEC_DEC1_CH1_EN,
                         (channel & TLK_I2S_CHANNEL_RIGHT) ? 1 : 0);
    }
}

static void tlk_audio_set_sample_rate(enum tlk_audio_input_stream stream,
                                      enum tlk_audio_sample_rate  rate)
{
    if (stream == TLK_AUDIO_INPUT_STREAM0)
    {
        reg_codec_clk_ctr1 = (reg_codec_clk_ctr1 & (~FLD_AUDIO_CODEC_DEC0_CLK_SR)) |
                             (tlk_audio_sample_rate_to_reg_table[rate] << 1);
    }
    else
    {
        reg_codec_dec1_ctr1 = tlk_audio_sample_rate_to_reg_table[rate];
    }
}

static void tlk_audio_set_adc_pga_l_gain(uint8_t pga_gain)
{
    reg_codec_ana_reg0 = pga_gain;
}

static void tlk_audio_set_adc_pga_r_gain(uint8_t pga_gain)
{
    reg_codec_ana_reg11 = (reg_codec_ana_reg11 & (~FLD_AUDIO_PGA_VOL_IN_R)) | pga_gain;
}

static void tlk_audio_set_stream0_dig_gain0(uint8_t d_gain)
{
    reg_codec_alc_ctr5 = (reg_codec_alc_ctr5 & (~FLD_AUDIO_CODEC_DEC_GAIN)) | (d_gain << 4);
}

static void tlk_audio_set_stream0_dig_gain1(uint8_t d_gain)
{
    reg_codec_dec0_vol0 = d_gain;
}

static void tlk_audio_set_stream1_dig_gain(uint8_t d_gain)
{
    reg_codec_dec1_vol = d_gain;
}

static void tlk_audio_swap_data(enum tlk_audio_input_stream stream, bool en)
{
    if (stream == TLK_AUDIO_INPUT_STREAM0)
    {
        if (en)
        {
            reg_codec_dsm_ctr1 = reg_codec_dsm_ctr1 | FLD_AUDIO_CODEC_DEC0_SWAP;
        }
        else
        {
            reg_codec_dsm_ctr1 = reg_codec_dsm_ctr1 & (~FLD_AUDIO_CODEC_DEC0_SWAP);
        }
    }
    else
    {
        if (en)
        {
            reg_codec_dec1_ctr0 = reg_codec_dec1_ctr0 | FLD_AUDIO_CODEC_DEC1_SWAP;
        }
        else
        {
            reg_codec_dec1_ctr0 = reg_codec_dec1_ctr0 & (~FLD_AUDIO_CODEC_DEC1_SWAP);
        }
    }
}

static void tlk_audio_swap_data_errata(enum tlk_audio_input_stream stream, enum tlk_i2s_fifo fifo,
                                       enum tlk_audio_input_source src,
                                       enum tlk_i2s_channel        channel,
                                       enum tlk_audio_sample_rate  rate)
{
    /*
     * at the sampling rate of 44.1k and 48k, two dmic data exchanges occur in the mono and stereo
     * modes so at the sampling rate of 44.1k and 48k, software is required to exchange the two dmic
     *  data so as to restore the original data of the two dmics.
     *
     * sample rate = AUDIO_44P1K or AUDIO_48K,fifo = fifo0
     * (1) audio_swap_stream0_data(DATA_INVERT_EN)
     * (2) mono:fifo0->ch1_r on
     *          fifo1->ch0_l on
     * sample rate = AUDIO_44P1K or AUDIO_48K,fifo = fifo1
     * (1) audio_swap_stream0_data(DATA_INVERT_DIS)
     * (2) mono:fifo0->ch0_l on
     *          fifo1->ch1_r on
     */

    bool should_invert =
        (src == TLK_AUDIO_INPUT_DMIC) && (rate == TLK_AUDIO_44P118K || rate == TLK_AUDIO_48K);
    bool need_invert = false;

    switch (channel)
    {
    case TLK_I2S_CHANNEL_LEFT:
        need_invert =
            (should_invert && fifo == TLK_I2S_FIFO0) || (!should_invert && fifo == TLK_I2S_FIFO1);
        break;
    case TLK_I2S_CHANNEL_RIGHT:
        need_invert =
            (should_invert && fifo == TLK_I2S_FIFO1) || (!should_invert && fifo == TLK_I2S_FIFO0);
        break;
    case TLK_I2S_CHANNEL_STEREO:
        need_invert = should_invert;
        break;
    default:
        break;
    }

    tlk_audio_swap_data(stream, need_invert);
}

static void tlk_audio_rx_fifo_set_mode(enum tlk_audio_input_stream stream, enum tlk_i2s_fifo fifo,
                                       enum tlk_i2s_channel    channel,
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

    if (stream == TLK_AUDIO_INPUT_STREAM0)
    {
        if (channel == TLK_I2S_CHANNEL_STEREO)
        {
            reg_audio_dec0_sel =
                (reg_audio_dec0_sel & (~(FLD_AUDIO_DEC0_AINL_COME | FLD_AUDIO_DEC0_AINR_COME))) |
                TLK_MASK_VAL(
                    FLD_AUDIO_DEC0_AINL_COME, reg_value, FLD_AUDIO_DEC0_AINR_COME, reg_value);
        }
        else
        {
            reg_audio_dec0_sel =
                (fifo == TLK_I2S_FIFO0)
                    ? ((reg_audio_dec0_sel & (~FLD_AUDIO_DEC0_AINL_COME)) | (reg_value))
                    : ((reg_audio_dec0_sel & (~FLD_AUDIO_DEC0_AINR_COME)) | (reg_value << 2));
        }
    }
    else
    {
        if (channel == TLK_I2S_CHANNEL_STEREO)
        {
            reg_audio_dec1_sel =
                (reg_audio_dec1_sel & (~(FLD_AUDIO_DEC1_AINL_COME | FLD_AUDIO_DEC1_AINR_COME))) |
                TLK_MASK_VAL(
                    FLD_AUDIO_DEC1_AINL_COME, reg_value, FLD_AUDIO_DEC1_AINR_COME, reg_value);
        }
        else
        {
            reg_audio_dec1_sel =
                (fifo == TLK_I2S_FIFO0)
                    ? ((reg_audio_dec1_sel & (~FLD_AUDIO_DEC1_AINL_COME)) | (reg_value))
                    : (reg_audio_dec1_sel & (~FLD_AUDIO_DEC1_AINR_COME)) | (reg_value << 4);
        }
    }
}

static void tlk_audio_input_set_path(enum tlk_i2s_fifo fifo, uint8_t ain_sel)
{
    reg_audio_sel(fifo) = (reg_audio_sel(fifo) & (~FLD_AUDIO_AIN_SEL)) | (ain_sel);
}

static void tlk_audio_dac_power_on(enum tlk_i2s_channel channel)
{
    tlk_analog_write_reg8(0x1a,
                          tlk_analog_read_reg8(0x1a) & (~TLK_BIT(1))); // pd_LDO_dac=0(power on)
    tlk_analog_write_reg8(0x1a, tlk_analog_read_reg8(0x1a) & (~TLK_BIT(2))); // pd_LDO_pa =0(on))

    bool reg_val_left  = !(channel & TLK_I2S_CHANNEL_LEFT);
    bool reg_val_right = !(channel & TLK_I2S_CHANNEL_RIGHT);

    reg_codec_ana_reg3 =
        (reg_codec_ana_reg3 &
         (~(FLD_AUDIO_PD_PA_L | FLD_AUDIO_PD_PA_R | FLD_AUDIO_PD_DAC_L | FLD_AUDIO_PD_DAC_R))) |
        TLK_MASK_VAL(FLD_AUDIO_PD_PA_L,
                     reg_val_left,
                     FLD_AUDIO_PD_PA_R,
                     reg_val_right,
                     FLD_AUDIO_PD_DAC_L,
                     reg_val_left,
                     FLD_AUDIO_PD_DAC_R,
                     reg_val_right);
    /*FLD_AUDIO_FRC_ON_DRL must be 1*/
    reg_codec_ana_reg7 =
        (reg_codec_ana_reg7 & (~(FLD_AUDIO_FRC_ON_DRVL | FLD_AUDIO_FRC_ON_DRVR))) |
        TLK_MASK_VAL(FLD_AUDIO_FRC_ON_DRVL, !reg_val_left, FLD_AUDIO_FRC_ON_DRVR, !reg_val_right);
}

static void tlk_audio_set_output_lbias_curr(codec_lbias_curr_sel_e lbias_curr)
{
    reg_codec_ana_reg4 = (lbias_curr & 0x03) << 5;
    reg_codec_ana_reg5 = ((lbias_curr & TLK_BIT(2)) == TLK_BIT(2)) ? 1 : 0;
}

static void tlk_audio_dac_init(codec_dac_mode_e work_mode)
{
    /*CIC compensation +0.6dB, default 0*/
    reg_codec_pga_l_2 |= FLD_AUDIO_CIC_COMP_EN;
    /*BIT[4]dsm sigma-delta modulator g coefficient select , BIT[5] sigma-delta modulator quantizer
     * select  default 1*/
    reg_codec_pga_l_2 &= ~(TLK_BIT_RNG(4, 5));
    if (HP_MODE == work_mode)
    {
        tlk_audio_set_output_lbias_curr(CURR_5P0_UA);
    }
    else if (NML_MODE == work_mode)
    {
        tlk_audio_set_output_lbias_curr(CURR_3P8_UA);
    }
    else if (LP_MODE == work_mode)
    {
        tlk_audio_set_output_lbias_curr(CURR_3P07_UA);
    }

    /* FLD_AUDIO_INT_VREF must be 1(Bypass the Vref_buf LPF filter)*/
    reg_codec_ana_reg6 = TLK_MASK_VAL(FLD_AUDIO_HP_VREF_BUF,
                                      1,
                                      FLD_AUDIO_INT_VREF,
                                      1,
                                      FLD_AUDIO_LP_VREF_BUF,
                                      0,
                                      FLD_AUDIO_LP_L_DAC,
                                      1,
                                      FLD_AUDIO_LP_R_DAC,
                                      1,
                                      FLD_AUDIO_LP_L_DRV,
                                      0,
                                      FLD_AUDIO_LP_R_DRV,
                                      0);

    reg_codec_ana_reg7 =
        (reg_codec_ana_reg7 & (~(FLD_AUDIO_HP_CTRL_DRV_L | FLD_AUDIO_HP_CTRL_DRV_R))) |
        TLK_MASK_VAL(FLD_AUDIO_HP_CTRL_DRV_L, 1, FLD_AUDIO_HP_CTRL_DRV_R, 1);
}

static void tlk_audio_dac_set_sample_rate(enum tlk_audio_sample_rate rate)
{
    reg_codec_clk_ctr2 &= (~FLD_AUDIO_CODEC_INT_EN);
    reg_codec_clk_ctr2 = (reg_codec_clk_ctr2 & (~FLD_AUDIO_CODEC_INT_CLK_SR)) |
                         ((tlk_audio_sample_rate_to_reg_table[rate]) << 3);
}

static void tlk_audio_set_dac_pga_l_gain(uint16_t pga_gain)
{
    reg_codec_pga_r_1 = pga_gain & 0xff;
    reg_codec_pga_r_2 = (reg_codec_pga_r_2 & (~FLD_AUDIO_PGA_GAIN_R_H)) | ((pga_gain >> 8) & 0x07);
}

static void tlk_audio_set_dac_pga_r_gain(uint16_t pga_gain)
{
    reg_codec_pga_l_1 = pga_gain & 0xff;
    reg_codec_pga_l_2 = (reg_codec_pga_l_2 & (~FLD_AUDIO_PGA_GAIN_L_H)) | ((pga_gain >> 8) & 0x07);
}

static void tlk_audio_set_dac_l_gain(uint16_t d_gain)
{
    reg_codec_int_vol_l_2 = d_gain & 0xff;
    reg_codec_int_vol_l_1 =
        (reg_codec_int_vol_l_1 & (~FLD_AUDIO_CODEC_INT_VOL_L_L)) | ((d_gain >> 8) & 0x0f);
}

static void tlk_audio_set_dac_r_gain(uint16_t d_gain)
{
    reg_codec_int_vol_r_2 = d_gain & 0xff;
    reg_codec_int_vol_r_1 =
        (reg_codec_int_vol_r_1 & (~FLD_AUDIO_CODEC_INT_VOL_R_L)) | ((d_gain >> 8) & 0x0f);
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

    if (fifo == TLK_I2S_FIFO1)
    {
        reg_value |= 0b100;

        if (channel == TLK_I2S_CHANNEL_STEREO)
        {
            reg_value += 2;
            reg_value ^= 0b01;
        }
    }

    reg_audio_dec0_sel = (reg_audio_dec0_sel & (~FLD_AUDIO_INT_AOUT_COME)) | (reg_value << 4);
}

static void tlk_audio_set_output_path(enum tlk_i2s_fifo fifo_chn, uint8_t aout_sel)
{
    reg_audio_sel(fifo_chn) = (reg_audio_sel(fifo_chn) & (~FLD_AUDIO_AOUT_SEL)) | (aout_sel << 3);
}

static void tlk_audio_set_dig_gain1(uint8_t d_gain)
{
    reg_codec_dec0_vol0 = d_gain;
}

/* Audio driver API */

void tlk_audio_init(void)
{
    tlk_audio_power_on();
    tlk_audio_set_clk(1, 16); // mclk 192/16=12M for internal codec
    reg_codec_clk_ctr1 |=
        FLD_AUDIO_CODEC_CLK_USB | FLD_AUDIO_CODEC_CLK0_EN; // usb mode and codec clk enable
}

void tlk_audio_amic_pinmux_configure(struct tlk_audio_amic_pinmux* mux)
{
    (void) mux;
}

void tlk_audio_dmic_pinmux_configure(enum tlk_audio_input_stream   stream,
                                     struct tlk_audio_dmic_pinmux* mux)
{
    const enum tlk_gpio_mux dmic_dat = TLK_GPIO_MUX_DMIC0_DAT + stream * 2;
    const enum tlk_gpio_mux dmic_clk = TLK_GPIO_MUX_DMIC0_CLK + stream * 2;

    tlk_gpio_configure(mux->data.port, mux->data.pin, TLK_GPIO_INPUT_NO_PULL);
    tlk_gpio_set_mux(mux->data.port, mux->data.pin, dmic_dat);
    tlk_gpio_disable(mux->data.port, mux->data.pin);

    tlk_gpio_set_mux(mux->clk1.port, mux->clk1.pin, dmic_clk);
    tlk_gpio_disable(mux->clk1.port, mux->clk1.pin);

    if (mux->clk2.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_set_mux(mux->clk2.port, mux->clk2.pin, dmic_clk);
        tlk_gpio_disable(mux->clk2.port, mux->clk2.pin);
    }
}

void tlk_audio_sdm_pinmux_configure(struct tlk_audio_sdm_pinmux* mux)
{
    (void) mux;
}

void tlk_audio_input_configure(struct tlk_audio_input* cfg)
{
    if (cfg->stream == TLK_AUDIO_INPUT_STREAM0)
    {
        if (cfg->source != TLK_AUDIO_INPUT_DMIC)
        {
            tlk_audio_set_adc_clock();
            tlk_audio_adc_power_on(cfg->audio_channel, cfg->source == TLK_AUDIO_INPUT_AMIC);
        }

        tlk_audio_set_input_mode(cfg->source);
        tlk_audio_set_path(cfg->stream, cfg->source, cfg->audio_channel, cfg->sample_rate);
        tlk_audio_set_sample_rate(cfg->stream, cfg->sample_rate);

        // Initialization sets all input stream gain to 0
        // 1.because some gains are not 0 default
        // 2.for performance test
        tlk_audio_set_adc_pga_l_gain(15);    // 0dB
        tlk_audio_set_adc_pga_r_gain(15);    // 0dB
        tlk_audio_set_stream0_dig_gain0(00); // 0dB
        tlk_audio_set_stream0_dig_gain1(32); // 0dB

        tlk_audio_swap_data_errata(
            cfg->stream, cfg->fifo, cfg->source, cfg->audio_channel, cfg->sample_rate);

        tlk_audio_rx_fifo_set_mode(cfg->stream, cfg->fifo, cfg->audio_channel, cfg->data_width);
        tlk_audio_input_set_path(cfg->fifo, 4);

        if (cfg->source == TLK_AUDIO_INPUT_AMIC)
        {
            tlk_audio_set_dig_gain1(44); // 18dB
        }
        else if (cfg->source == TLK_AUDIO_INPUT_DMIC)
        {
            tlk_audio_set_dig_gain1(52); // 30dB
        }
    }
    else
    {
        tlk_audio_set_path(cfg->stream, cfg->source, cfg->audio_channel, cfg->sample_rate);
        tlk_audio_set_sample_rate(cfg->stream, cfg->sample_rate);

        tlk_audio_swap_data_errata(
            cfg->stream, cfg->fifo, cfg->source, cfg->audio_channel, cfg->sample_rate);

        tlk_audio_rx_fifo_set_mode(cfg->stream, cfg->fifo, cfg->audio_channel, cfg->data_width);
        tlk_audio_input_set_path(cfg->fifo, 5);

        tlk_audio_set_stream1_dig_gain(44); // 18dB
    }

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
    tlk_audio_dac_power_on(cfg->audio_channel);
    tlk_audio_dac_init(NML_MODE);
    tlk_audio_dac_set_sample_rate(cfg->sample_rate);

    tlk_audio_set_dac_pga_l_gain(0x400); // 0dB
    tlk_audio_set_dac_pga_r_gain(0x400); // 0dB
    tlk_audio_set_dac_l_gain(0xfff);     // 0dB
    tlk_audio_set_dac_r_gain(0xfff);     // 0dB

    tlk_audio_tx_fifo_set_mode(cfg->fifo, cfg->audio_channel, cfg->data_width);
    tlk_audio_set_output_path(cfg->fifo, 2);

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
    reg_codec_clk_ctr2 |= FLD_AUDIO_CODEC_DEC0_EN;
    reg_codec_dec1_ctr0 |= FLD_AUDIO_CODEC_DEC1_EN;
}

void tlk_audio_output_enable(void)
{
    reg_codec_clk_ctr2 |= FLD_AUDIO_CODEC_INT_EN;
}

void tlk_audio_input_disable(void)
{
    reg_codec_clk_ctr2 &= ~FLD_AUDIO_CODEC_DEC0_EN;
    reg_codec_dec1_ctr0 &= ~FLD_AUDIO_CODEC_DEC1_EN;
}

void tlk_audio_output_disable(void)
{
    reg_codec_clk_ctr2 &= ~FLD_AUDIO_CODEC_INT_EN;
}
