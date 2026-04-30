#include "core/include/tlk_adc.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_sys.h"
#include "registers/tlk_adc_reg_new.h"
#include "registers/tlk_audio_reg.h"
#include "registers/tlk_chip.h"
#include "registers/tlk_dma.h"
#include "registers/tlk_gpio.h"
#include "properties/tlk_adc.h"
#include "properties/tlk_gpio.h"
#include "common/include/tlk_init.h"

#define TLK_ADC_MAX_STATE_NUM           0x02
#define TLK_ADC_DATA_READY              TLK_BIT(0)

#if IS_ENABLED(CONFIG_TLK_DMA)

#define TLK_SAR_ADC_IN_FIFO     3
#define TLK_DMA_REQ_AUDIO_RX    UNISDK_DMA_FUNC_NUM_AUDIO1_RX
#define TLK_AUDIO_FIFO          TLK_FIFO1

typedef enum
{
    TLK_FIFO0 = 0,
    TLK_FIFO1,
} tlk_audio_fifo_chn_e;

struct tlk_dma_config tlk_adc_rx_dma_config =
{
    .dst_req_sel    = 0,
    .src_req_sel    = TLK_DMA_REQ_AUDIO_RX, //adc use the audio1 interface
    .dst_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
    .src_addr_ctrl  = TLK_DMA_ADDR_FIX,
    .dstmode        = TLK_DMA_NORMAL_MODE,
    .srcmode        = TLK_DMA_HANDSHAKE_MODE,
    .dstwidth       = TLK_DMA_WORD_WIDTH, 
    .srcwidth       = TLK_DMA_WORD_WIDTH, 
    .src_burst_size = 0, //must 0
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0, //must 0
};

#endif

enum tlk_adc_sample_cycle
{
    TLK_ADC_SAMPLE_CYC_3,
    TLK_ADC_SAMPLE_CYC_6,
    TLK_ADC_SAMPLE_CYC_9,
    TLK_ADC_SAMPLE_CYC_12,
    TLK_ADC_SAMPLE_CYC_15,
    TLK_ADC_SAMPLE_CYC_18,
    TLK_ADC_SAMPLE_CYC_21,
    TLK_ADC_SAMPLE_CYC_24,
    TLK_ADC_SAMPLE_CYC_27,
    TLK_ADC_SAMPLE_CYC_30,
    TLK_ADC_SAMPLE_CYC_33,
    TLK_ADC_SAMPLE_CYC_36,
    TLK_ADC_SAMPLE_CYC_39,
    TLK_ADC_SAMPLE_CYC_42,
    TLK_ADC_SAMPLE_CYC_45,
    TLK_ADC_SAMPLE_CYC_48,
};

struct tlk_adc_config tlk_adc_config = {

};



/* Helper functions */

static uint8_t tlk_adc_get_port_pin_num(struct tlk_gpio_port_pin pp)
{
#if IS_ENABLED(CONFIG_TLK_GPIO_PORT_B_ENABLED)
    if (pp.port == TLK_GPIO_PORT_B) {
        return TLK_BIT_LOW_BIT(pp.pin) + 1;
    } else 
#endif
#if IS_ENABLED(CONFIG_TLK_GPIO_PORT_D_ENABLED)
    if (pp.port == TLK_GPIO_PORT_D) {
        if (pp.pin == TLK_GPIO_PIN_0) {
            return 9;
        } else if (pp.pin == TLK_GPIO_PIN_1) {
            return 10;
        } else {
            return 0;
        }
    } else 
#endif
    {
        return 0;
    }
}

static void tlk_adc_configure_pin(enum tlk_gpio_port port, enum tlk_gpio_pin pin) 
{
    TLK_BM_SET(tlk_gpio_reg.port_config[port].function, pin);

#if IS_ENABLED(CONFIG_TLK_GPIO_PORT_D_ENABLED)
    if (port == TLK_GPIO_PORT_D) {
        tlk_analog_write_reg8(UNISDK_GPIO_AREG_PD_IE, tlk_analog_read_reg8(UNISDK_GPIO_AREG_PD_IE) & (~pin));
    } else 
#endif
    {
        TLK_BM_CLR(tlk_gpio_reg.port_config[port].input_en, pin);
    }
    
    TLK_BM_SET(tlk_gpio_reg.port_config[port].output_en, pin);
    TLK_BM_CLR(tlk_gpio_reg.port_config[port].output, pin);
}

static inline void tlk_adc_clk_en(void)
{
    tlk_analog_write_reg8(AREG_ADC_CLK_SETTING, tlk_analog_read_reg8(AREG_ADC_CLK_SETTING) | FLD_CLK_24M_TO_SAR_EN);
}

static inline void tlk_adc_set_clk(void)
{
    tlk_analog_write_reg8(AREG_ADC_SAMPLE_CLK_DIV, UNISDK_ADC_CLK_DIV);
}

static inline void tlk_adc_reset(void)
{
    tlk_reset_reg.reset3_bit.saradc = 0;
    tlk_reset_reg.reset3_bit.saradc = 1;
}

static void tlk_adc_power_on(void)
{
    tlk_analog_write_reg8(AREG_ADC_PGA_CTRL, (tlk_analog_read_reg8(AREG_ADC_PGA_CTRL) & (~FLD_SAR_ADC_POWER_DOWN)));
}

static inline void tlk_adc_set_resolution(enum tlk_adc_resolution resolution)
{
    tlk_analog_write_reg8(AREG_ADC_RES_M, (tlk_analog_read_reg8(AREG_ADC_RES_M) & (~FLD_ADC_RES_M)) | resolution);
}

static inline void tlk_adc_set_diff_input(uint8_t p_ain, uint8_t n_ain)
{
    tlk_analog_write_reg8(AREG_ADC_RES_M, tlk_analog_read_reg8(AREG_ADC_RES_M) | FLD_ADC_EN_DIFF_CHN_M);
    tlk_analog_write_reg8(AREG_ADC_AIN_CHN_MISC, n_ain | (p_ain << 4));
}

static void tlk_adc_set_ref_voltage(enum tlk_adc_vref v_ref)
{
    tlk_analog_write_reg8(AREG_ADC_VREF, v_ref);

    if (v_ref == TLK_ADC_VREF_1P2V) {
        tlk_analog_write_reg8(AREG_AIN_SCALE, (tlk_analog_read_reg8(AREG_AIN_SCALE) & (0xC0)) | 0x3d);
    } else if (v_ref == TLK_ADC_VREF_0P9V) {
        tlk_analog_write_reg8(AREG_AIN_SCALE, (tlk_analog_read_reg8(AREG_AIN_SCALE) & (0xC0)) | 0x15);
    }
}

static inline void tlk_adc_set_scale_factor(enum tlk_adc_prescale pre_scale)
{
    tlk_analog_write_reg8(AREG_AIN_SCALE, (tlk_analog_read_reg8(AREG_AIN_SCALE) & (~FLD_SEL_AIN_SCALE)) | (pre_scale << 6));
}

static inline void tlk_adc_set_state_length(uint16_t r_max_mc, uint8_t r_max_s)
{
    tlk_analog_write_reg8(AREG_R_MAX_MC, r_max_mc);
    tlk_analog_write_reg8(AREG_R_MAX_S, ((r_max_mc >> 8) << 6) | (r_max_s & FLD_R_MAX_S));
}

static inline void tlk_adc_set_tsample_cycle(enum tlk_adc_sample_cycle sample_cycle)
{
    //ana_ee<7:4> is reserved, so no need care its value
    tlk_analog_write_reg8(AREG_ADC_TSMAPLE_M, sample_cycle);
}

static inline void tlk_adc_set_chn_en(void)
{
    tlk_analog_write_reg8(AREG_ADC_CHN_EN, FLD_ADC_CHN_EN_M | (TLK_ADC_MAX_STATE_NUM << 4));
}

static void tlk_adc_set_vbat_divider_off(void)
{
    tlk_analog_write_reg8(AREG_ADC_VREF_VBAT_DIV, (tlk_analog_read_reg8(AREG_ADC_VREF_VBAT_DIV) & (~FLD_ADC_VREF_VBAT_DIV)));
}

#if IS_ENABLED(CONFIG_TLK_DMA)

static void tlk_audio_data_fifo_input_path_sel(tlk_audio_fifo_chn_e fifo_chn, uint8_t ain_sel)
{
    reg_audio_sel(fifo_chn) = (reg_audio_sel(fifo_chn) & (~FLD_AUDIO_AIN_SEL)) | (ain_sel);
}

static void tlk_audio_power_on(void)
{
    /*The power-on sequence that must be followed
     * 1.power down audio power
     * 2.wait audio power stabilization
     * 3.switch on audio power switch
     * 4.power on audio power
     * 5.wait audio power stabilization
     * */
    tlk_analog_write_reg8(0x7d, tlk_analog_read_reg8(0x7d) | (TLK_BIT(2)));  //1.audio power down  ,     1:power down 0:power on
    tlk_sys_delay(6);                                                        //2.wait audio power stabilization
    tlk_analog_write_reg8(0x1e, tlk_analog_read_reg8(0x1e) & (~TLK_BIT(5))); //3.power switch default 1, 1:switch off 0:switch on
    tlk_analog_write_reg8(0x7d, tlk_analog_read_reg8(0x7d) & (~TLK_BIT(2))); //4.audio power on          1:power down 0:power on
    tlk_sys_delay(6);                                                        //5.wait audio power stabilization
}

static void tlk_adc_set_dma_config(enum tlk_dma_chn chn)
{
    tlk_audio_power_on();
    {
        tlk_audio_data_fifo_input_path_sel(TLK_AUDIO_FIFO, TLK_SAR_ADC_IN_FIFO); //connect DMA and ADC by audio input fifo1.
    }
    tlk_dma_configure(chn, &tlk_adc_rx_dma_config);
    
    tlk_dma_reg.chn[chn].control_bit.tc_irq_en = 1;  // tlk_dma_clr_irq_mask
    tlk_dma_reg.chn[chn].control_bit.err_irq_en = 1; // tlk_dma_clr_irq_mask
    tlk_dma_reg.chn[chn].control_bit.abt_irq_en = 1; // tlk_dma_clr_irq_mask

    tlk_dma_reg.chn[chn].control_bit.tc_irq_en = 0;  // tlk_dma_set_irq_mask
}

static inline void tlk_dma_chn_dis(enum tlk_dma_chn chn)
{
    tlk_dma_reg.chn[chn].control_bit.chn_en = 0;
}

static inline uint8_t tlk_dma_get_tc_irq_status(uint32_t tc_chn)
{
    return tlk_dma_reg.irq_status_bit.tc_irq & tc_chn;
}

static inline void tlk_dma_clr_tc_irq_status(uint32_t tc_chn)
{
    tlk_dma_reg.irq_status_bit.tc_irq = tc_chn;
}

static inline void tlk_audio_fifo_reset(tlk_audio_fifo_chn_e fifo_chn)
{
    reg_i2s0_cfg2 |= ((fifo_chn == TLK_FIFO0) ? FLD_AUDIO_FIFO0_RST : FLD_AUDIO_FIFO1_RST);
    reg_i2s0_cfg2 &= (~((fifo_chn == TLK_FIFO0) ? FLD_AUDIO_FIFO0_RST : FLD_AUDIO_FIFO1_RST));
}

static void tlk_adc_clr_sample_status_dma(void)
{
    tlk_audio_fifo_reset(TLK_AUDIO_FIFO);
    tlk_dma_chn_dis(tlk_adc_config.dma_channel);
    tlk_dma_clr_tc_irq_status(1 << tlk_adc_config.dma_channel);
}

static uint8_t tlk_adc_get_sample_status_dma(void)
{
    return (tlk_dma_get_tc_irq_status(1 << tlk_adc_config.dma_channel));
}

#endif



/* ADC driver API */

enum tlk_adc_status tlk_adc_configure(struct tlk_adc_config *cfg)
{
    tlk_adc_config = *cfg;

#if IS_ENABLED(CONFIG_TLK_DMA)
    if (cfg->dma_en) {
        tlk_adc_set_dma_config(cfg->dma_channel);
    }
#endif

    tlk_adc_disable();
    tlk_adc_reset();
    tlk_adc_clk_en();
    tlk_adc_set_clk();
    tlk_adc_set_ref_voltage(cfg->vref);
    tlk_adc_set_scale_factor(cfg->prescale);
    uint8_t  adc_set;
    uint8_t  adc_sample_cycle;
    uint16_t adc_capture;

    switch (cfg->sample_freq) {
    case TLK_ADC_SAMPLE_FREQ_23K:
        adc_set = 15;
        adc_sample_cycle = TLK_ADC_SAMPLE_CYC_24;
        adc_capture = 1023;
        break;
    case TLK_ADC_SAMPLE_FREQ_48K:
        adc_set = 10;
        adc_sample_cycle = TLK_ADC_SAMPLE_CYC_12;
        adc_capture = 490;
        break;
    case TLK_ADC_SAMPLE_FREQ_96K:
        adc_set = 10;
        adc_sample_cycle = TLK_ADC_SAMPLE_CYC_6;
        adc_capture = 240;
        break;
    case TLK_ADC_SAMPLE_FREQ_192K:
        adc_set = 10;
        adc_sample_cycle = TLK_ADC_SAMPLE_CYC_3;
        adc_capture = 115;
        break;
    default:
        return TLK_ADC_UNSUPPORTED;
    }
    
    tlk_adc_set_state_length(adc_capture, adc_set);
    tlk_adc_set_tsample_cycle(adc_sample_cycle); 

    tlk_adc_set_resolution(cfg->resolution);
    tlk_adc_set_chn_en();
    tlk_adc_set_vbat_divider_off();
    
    uint8_t sampling_pin_num = tlk_adc_get_port_pin_num(cfg->sampling_port_pin);
    if (sampling_pin_num == 0) {
        return TLK_ADC_UNSUPPORTED;
    }

    uint8_t differential_pin_num = UNISDK_ADC_GND_PIN_NUM;
    if (cfg->is_differential) {
        differential_pin_num = tlk_adc_get_port_pin_num(cfg->differential_port_pin);

        if (differential_pin_num == 0 || differential_pin_num == sampling_pin_num) {
            return TLK_ADC_UNSUPPORTED;
        }
    }

    tlk_adc_set_diff_input(sampling_pin_num, differential_pin_num);
    
    tlk_adc_configure_pin(cfg->sampling_port_pin.port, cfg->sampling_port_pin.pin);
    if (cfg->is_differential) {
        tlk_adc_configure_pin(cfg->differential_port_pin.port, cfg->differential_port_pin.pin);
    }

    tlk_adc_power_on();

    return TLK_ADC_OK;
}

enum tlk_adc_status tlk_adc_read(uint16_t *buffer, uint32_t size)
{
#if IS_ENABLED(CONFIG_TLK_DMA)
    if (tlk_adc_config.dma_en) {
        tlk_dma_start_transfer(
            tlk_adc_config.dma_channel, REG_AUDIO_FIFO_ADDR(TLK_AUDIO_FIFO), (uint32_t)buffer, 
            size * sizeof(uint16_t), TLK_DMA_WORD_WIDTH);

        TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
            (tlk_adc_get_sample_status_dma()), 
            (tlk_adc_config.timeout_us), 
            (return TLK_ADC_TIMEOUT;)
        )

        tlk_adc_clr_sample_status_dma(); 

        for (uint32_t i = 0; i < size; i++) {
            // 14 bit resolution, TLK_BIT(13) is sign bit, 1 means negative voltage in differential_mode
            if (buffer[i] & TLK_BIT(13)) { 
                buffer[i] = 0;
            } else {
                buffer[i] = (buffer[i] & 0x1fff);
            }
        }

        return TLK_ADC_OK;
    } else 
#endif
    {
        uint16_t adc_code = 0;

        for (uint32_t i = 0; i < size; i++) {
            TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
                (tlk_analog_read_reg8(AREG_ADC_DATA_STATUS) & TLK_ADC_DATA_READY), 
                (tlk_adc_config.timeout_us), 
                (return TLK_ADC_TIMEOUT;)
            )

            /* Lock ADC code in analog register */
            tlk_analog_write_reg8(AREG_ADC_DATA_SAMPLE_CONTROL, tlk_analog_read_reg8(AREG_ADC_DATA_SAMPLE_CONTROL) | FLD_NOT_SAMPLE_ADC_DATA);

            adc_code = tlk_analog_read_reg16(AREG_ADC_MISC_L);

            tlk_analog_write_reg8(AREG_ADC_DATA_SAMPLE_CONTROL, tlk_analog_read_reg8(AREG_ADC_DATA_SAMPLE_CONTROL) & (~FLD_NOT_SAMPLE_ADC_DATA));

            if (adc_code & TLK_BIT(13)) {
                adc_code = 0;
            } else {
                adc_code &= 0x1FFF;
            }

            buffer[i] = adc_code;
        }

        return TLK_ADC_OK;
    }
}

void tlk_adc_disable(void)
{
    tlk_analog_write_reg8(areg_adc_pga_ctrl, (tlk_analog_read_reg8(areg_adc_pga_ctrl) | FLD_SAR_ADC_POWER_DOWN));
}



#if IS_ENABLED(CONFIG_TLK_ADC_PM_DEVICE)

static void tlk_adc_restore(void)
{
    tlk_adc_configure(&tlk_adc_config);
}

TLK_REGISTER_AFTER_SLEEP(tlk_adc_restore, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#endif