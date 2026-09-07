#include "core/include/tlk_adc.h"
#include "common/include/tlk_init.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_gpio.h"
#include "properties/tlk_adc.h"
#include "properties/tlk_gpio.h"
#include "registers/tlk_adc.h"
#include "registers/tlk_adc_reg.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_chip.h"
#include "registers/tlk_clock.h"
#include "registers/tlk_dma.h"
#include "registers/tlk_gpio.h"

#if TLK_IS_ENABLED(CONFIG_TLK_DMA)

struct tlk_dma_config tlk_adc_rx_dma_config = {
    .dst_req_sel    = 0,
    .src_req_sel    = UNISDK_DMA_FUNC_NUM_SAR_ADC_RX,
    .dst_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
    .src_addr_ctrl  = TLK_DMA_ADDR_FIX,
    .dstmode        = TLK_DMA_NORMAL_MODE,
    .srcmode        = TLK_DMA_HANDSHAKE_MODE,
    .dstwidth       = TLK_DMA_WORD_WIDTH,
    .srcwidth       = TLK_DMA_WORD_WIDTH,
    .src_burst_size = 0, // must 0
    .read_num_en    = 0,
    .priority       = 0,
    .write_num_en   = 0,
    .auto_en        = 0, // must 0
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

uint8_t tlk_adc_rx_fifo_index = 0;

struct tlk_adc_config tlk_adc_config = {

};

/* Helper functions */

static uint8_t tlk_adc_get_port_pin_num(struct tlk_gpio_port_pin pp)
{
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_B_ENABLED)
    if (pp.port == TLK_GPIO_PORT_B)
    {
        return TLK_BIT_LOW_BIT(pp.pin) + 1;
    }
    else
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_D_ENABLED)
        if (pp.port == TLK_GPIO_PORT_D)
    {
        if (pp.pin == TLK_GPIO_PIN_0)
        {
            return 9;
        }
        else if (pp.pin == TLK_GPIO_PIN_1)
        {
            return 10;
        }
        else
        {
            return 0;
        }
    }
    else
#endif
    {
        return 0;
    }
}

static void tlk_adc_configure_pin(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    TLK_BM_SET(tlk_gpio_reg.port_config[port].function, pin);

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO_PORT_B_ENABLED)
    if ((port == TLK_GPIO_PORT_B) && (pin >= TLK_GPIO_PIN_4) && (pin <= TLK_GPIO_PIN_7))
    {
        TLK_ANALOG_MODIFY_RAW8(TLK_AREG_PB_IE, (value &= ~pin;));
    }
    else
#endif
    {
        TLK_BM_CLR(tlk_gpio_reg.port_config[port].input_en, pin);
    }

    TLK_BM_SET(tlk_gpio_reg.port_config[port].output_en, pin);
    tlk_gpio_reg.port_config[port].output_clear = pin;
}

static void tlk_adc_set_scan_chn_dis(void)
{
    // FLD_SCANT_MAX cannot be configured to 0, because 0 is the RNG channel, if RNG is not enabled
    // (default is not enabled), the state machine will not stop at the RNG channel after setting 0,
    // it will not stop.
    tlk_adc_reg.config0_bit.scant_max = 1;
}

static void tlk_adc_set_scan_chn_cnt(uint8_t chn_cnt)
{
    tlk_adc_reg.config0_bit.scant_max = chn_cnt * 2;
}

static void tlk_adc_clr_rx_index(void)
{
    tlk_adc_rx_fifo_index = 0;
}

static void tlk_adc_clr_rx_fifo_cnt(void)
{
    tlk_adc_reg.soft_control_bit.fifo_clr = 1;
    tlk_adc_clr_rx_index();
}

static void tlk_adc_all_chn_data_to_fifo_en(void)
{
    TLK_ANALOG_MODIFY(TLK_AREG_ADC_DATA_TRANSFER, (value.bit.r_auto_not_en = 0;));
}

static void tlk_adc_all_chn_data_to_fifo_dis(void)
{
    TLK_ANALOG_MODIFY(TLK_AREG_ADC_DATA_TRANSFER, (value.bit.r_auto_not_en = 1;));
}

static void tlk_adc_en_clk(void)
{
    tlk_clock_reg.clock_en3_bit.saradc = 1;
}

static void tlk_adc_set_clk(void)
{
    // tlk_adc_reg.config1_bit.clk_div = 1; // TODO: check this
    reg_adc_config1 = ((reg_adc_config1 & FLD_SAR_ADC_CLK_DIV) |
                       1); // div=1, adc digital clk = 24MHz/div.(crystal = 24MHz)

    // div=5, adc analog clk = 24MHz/(1+div) = 4M.
    union tlk_areg_adc_clk reg = {.bit.adc_div_mode = UNISDK_ADC_CLK_DIV};
    TLK_ANALOG_WRITE(TLK_AREG_ADC_CLK, reg);
}

static void tlk_adc_dig_clk_en(void)
{
    tlk_adc_reg.config2_bit.clk_en = 1;
}

static void tlk_adc_reset(void)
{
    tlk_reset_reg.reset3_bit.saradc = 0;
    tlk_reset_reg.reset3_bit.saradc = 1;
    tlk_adc_clr_rx_index();
}

static void tlk_adc_power_on(void)
{
    // tlk_adc_set_scan_chn_dis() must be called to stop the state machine
    // at the beginning of the M channel and not start sampling.
    tlk_adc_set_scan_chn_dis();

    TLK_ANALOG_MODIFY(TLK_AREG_ADC_PGA, (value.bit.adc_pd = 0;));
    tlk_adc_dig_clk_en();
}

static void tlk_adc_set_resolution(enum tlk_adc_resolution resolution)
{
    TLK_ANALOG_MODIFY(TLK_AREG_ADC_CONTROL, (value.bit.adc_resolution = resolution;));
}

static void tlk_adc_set_diff_input(enum tlk_adc_channel chn, uint8_t p_ain, uint8_t n_ain)
{
    TLK_ANALOG_MODIFY(TLK_AREG_ADC_CONTROL, (value.bit.adc_en_diffm = 1;));

    tlk_adc_reg.r_mux_bit[chn].p_input_pin = p_ain;
    tlk_adc_reg.r_mux_bit[chn].n_input_pin = n_ain;
}

static void tlk_adc_set_ref_voltage(enum tlk_adc_channel chn, enum tlk_adc_vref v_ref)
{
    tlk_adc_reg.channel_set_state_bit[chn].sel_vref = v_ref;

    TLK_ANALOG_MODIFY(TLK_AREG_ADC_AIN_SCALE, (
        if (v_ref == TLK_ADC_VREF_1P2V) {
            //Vref buffer bias current trimming:        150%
            //Comparator preamp bias current trimming:  100%
            value.bit.adc_itrim_preamp = 0b01;
            value.bit.adc_itrim_vrefbuf = 0b11;
            value.bit.adc_itrim_vcmbuf = 0b11;
        } else if (v_ref == TLK_ADC_VREF_0P9V) {
            //Vref buffer bias current trimming:        100%
            //Comparator preamp bias current trimming:  100%
            value.bit.adc_itrim_preamp = 0b01;
            value.bit.adc_itrim_vrefbuf = 0b01;
            value.bit.adc_itrim_vcmbuf = 0b01;
        }
    ));
}

static void tlk_adc_set_scale_factor(enum tlk_adc_channel chn, enum tlk_adc_prescale pre_scale)
{
    tlk_adc_reg.channel_set_state_bit[chn].sel_ai_scale = pre_scale;
}

static void tlk_adc_set_state_length(enum tlk_adc_channel chn, uint16_t r_max_mc, uint8_t r_max_s)
{
    tlk_adc_reg.capture_state_bit[chn].r_max_c     = r_max_mc;
    tlk_adc_reg.channel_set_state_bit[chn].r_max_s = r_max_s;
}

static void tlk_adc_set_tsample_cycle(enum tlk_adc_channel sample_cycle)
{
    tlk_adc_reg.tsamp_bit.m_tsamp = sample_cycle;
}

static void tlk_adc_set_chn_en(enum tlk_adc_channel chn)
{
    tlk_adc_reg.config2 |= TLK_BIT(chn);
}

static void tlk_adc_set_vbat_divider_off(enum tlk_adc_channel chn)
{
    uint8_t offset = chn * 2;
    tlk_adc_reg.vbat_div &= (~TLK_BIT_RNG(offset, offset + 1));
}

#if TLK_IS_ENABLED(CONFIG_TLK_DMA)

static void tlk_adc_set_dma_config(enum tlk_dma_chn chn)
{
    tlk_adc_reg.config2 = FLD_RX_DMA_ENABLE;
    tlk_dma_chn_configure(chn, &tlk_adc_rx_dma_config);
    tlk_dma_reg.chn[chn].ll_pointer = 0;

    tlk_adc_reg.rxfifo_trig_num_bit.rxfifo_trig_num = 1;
    tlk_adc_all_chn_data_to_fifo_en();
}

static void tlk_dma_dis_chn(enum tlk_dma_chn chn)
{
    tlk_dma_reg.chn[chn].control_bit.chn_en = 0;
}

static uint8_t tlk_dma_get_tc_irq_status(uint32_t tc_chn)
{
    return tlk_dma_reg.irq_status_bit.tc_irq & tc_chn;
}

static void tlk_dma_clr_tc_irq_status(uint32_t tc_chn)
{
    tlk_dma_reg.irq_status_bit.tc_irq = tc_chn;
}

static void tlk_adc_clr_irq_status_dma(void)
{
    // tlk_adc_set_scan_chn_dis() must be called when DMA is finished to stop the state machine
    // at the beginning of the M channel to prevent mis-ordering of multi-channel sampling data
    // when using multiple channels.
    tlk_adc_set_scan_chn_dis();
    tlk_dma_dis_chn(tlk_adc_config.dma_channel);
    tlk_dma_clr_tc_irq_status(1 << tlk_adc_config.dma_channel);
}

#endif

/* ADC driver API */

enum tlk_adc_status tlk_adc_configure(struct tlk_adc_config* cfg)
{
    if (cfg->resolution == TLK_ADC_RESOLUTION_14)
    {
        return TLK_ADC_UNSUPPORTED;
    }

    tlk_adc_config = *cfg;

#if TLK_IS_ENABLED(CONFIG_TLK_DMA)
    if (cfg->dma_en)
    {
        tlk_adc_set_dma_config(cfg->dma_channel);
        tlk_dma_reg.chn[cfg->dma_channel].control_bit.tc_irq_en = 1;
    }
#endif

    tlk_adc_disable();
    tlk_adc_reset();
    tlk_adc_en_clk();
    tlk_adc_set_clk();
    tlk_adc_set_resolution(cfg->resolution);

#if TLK_IS_ENABLED(CONFIG_TLK_DMA)
    if (!cfg->dma_en)
#endif
    {
        tlk_adc_all_chn_data_to_fifo_dis();
        tlk_adc_reg.config2_bit.rx_dma_en       = 0;
        tlk_adc_reg.config2_bit.rx_interrupt_en = 1;
    }

    /**
     * The set and capture of RNG channel are configured to 0 by default, and the actual state
     * machine scanning time of RNG channel is the maximum time(about 25us), and by configuring both
     * of them to 1 (the minimum scanning time), the state machine scanning time of RNG channel is
     * only (1+1)/24M=83ns, which enables the state machine to enter into the set state of the M
     * channel faster, and prevents the first code of M channel abnormality.
     */
    tlk_adc_reg.rng_set_state     = 0x01;
    tlk_adc_reg.rng_capture_state = 0x01;

    uint8_t sampling_pin_num = tlk_adc_get_port_pin_num(cfg->sampling_port_pin);
    if (sampling_pin_num == 0)
    {
        return TLK_ADC_UNSUPPORTED;
    }

    uint8_t differential_pin_num = UNISDK_ADC_GND_PIN_NUM;
    if (cfg->is_differential)
    {
        differential_pin_num = tlk_adc_get_port_pin_num(cfg->differential_port_pin);

        if (differential_pin_num == 0 || differential_pin_num == sampling_pin_num)
        {
            return TLK_ADC_UNSUPPORTED;
        }
    }

    tlk_adc_set_diff_input(cfg->channel, sampling_pin_num, differential_pin_num);

    tlk_adc_configure_pin(cfg->sampling_port_pin.port, cfg->sampling_port_pin.pin);
    if (cfg->is_differential)
    {
        tlk_adc_configure_pin(cfg->differential_port_pin.port, cfg->differential_port_pin.pin);
    }

    tlk_adc_set_vbat_divider_off(cfg->channel);
    tlk_adc_set_ref_voltage(cfg->channel, cfg->vref);
    tlk_adc_set_scale_factor(cfg->channel, cfg->prescale);

    uint8_t  adc_set;
    uint8_t  adc_sample_cycle;
    uint16_t adc_capture;

    switch (cfg->sample_freq)
    {
    case TLK_ADC_SAMPLE_FREQ_23K:
        adc_set          = 15;
        adc_sample_cycle = TLK_ADC_SAMPLE_CYC_48;
        adc_capture      = 1023;
        break;
    case TLK_ADC_SAMPLE_FREQ_48K:
        adc_set          = 10;
        adc_sample_cycle = TLK_ADC_SAMPLE_CYC_48;
        adc_capture      = 490;
        break;
    case TLK_ADC_SAMPLE_FREQ_96K:
        adc_set          = 10;
        adc_sample_cycle = TLK_ADC_SAMPLE_CYC_27;
        adc_capture      = 240;
        break;
    case TLK_ADC_SAMPLE_FREQ_192K:
        adc_set          = 10;
        adc_sample_cycle = TLK_ADC_SAMPLE_CYC_6;
        adc_capture      = 115;
        break;
    default:
        return TLK_ADC_UNSUPPORTED;
    }

    tlk_adc_set_state_length(cfg->channel, adc_capture, adc_set);
    tlk_adc_set_tsample_cycle(adc_sample_cycle);

    tlk_adc_set_chn_en(cfg->channel);

    tlk_adc_power_on();

    return TLK_ADC_OK;
}

enum tlk_adc_status tlk_adc_read(uint16_t* buffer, uint32_t size)
{
#if TLK_IS_ENABLED(CONFIG_TLK_DMA)
    if (tlk_adc_config.dma_en)
    {
        tlk_dma_chn_transfer_configure(
            tlk_adc_config.dma_channel, SAR_ADC_FIFO, (uint32_t) buffer, size * sizeof(uint16_t));

        tlk_dma_chn_transfer_start(tlk_adc_config.dma_channel);

        tlk_adc_clr_rx_fifo_cnt();
        tlk_adc_set_scan_chn_cnt(1);

        TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
            (tlk_dma_get_tc_irq_status(TLK_BIT(tlk_adc_config.dma_channel))),
            (tlk_adc_config.timeout_us),
            (return TLK_ADC_TIMEOUT;))

        tlk_adc_clr_irq_status_dma();

        for (uint32_t i = 0; i < size; i++)
        {
            // 12 bit resolution, TLK_BIT(11) is sign bit, 1 means negative voltage in
            // differential_mode
            if (buffer[i] & TLK_BIT(13))
            {
                buffer[i] = 0;
            }
            else
            {
                buffer[i] = (buffer[i] & 0x7ff);
            }
        }

        return TLK_ADC_OK;
    }
    else
#endif
    {
        uint16_t adc_code = 0;

        tlk_adc_clr_rx_fifo_cnt();
        tlk_adc_all_chn_data_to_fifo_en();
        tlk_adc_set_scan_chn_cnt(1);

        for (uint32_t i = 0; i < size; i++)
        {
            TLK_WAIT_FOR_TRUE_OR_TIMEOUT((tlk_adc_reg.rxfifo_trig_num_bit.buf_cnt),
                                         (tlk_adc_config.timeout_us),
                                         (return TLK_ADC_TIMEOUT;))

            adc_code              = tlk_adc_reg.rxfifo_data[tlk_adc_rx_fifo_index];
            tlk_adc_rx_fifo_index = !tlk_adc_rx_fifo_index;

            if (adc_code & TLK_BIT(13))
            {
                adc_code = 0;
            }
            else
            {
                adc_code &= 0x07FF;
            }

            buffer[i] = adc_code;
        }

        tlk_adc_set_scan_chn_dis();

        return TLK_ADC_OK;
    }
}

void tlk_adc_disable(void)
{
    TLK_ANALOG_MODIFY(TLK_AREG_ADC_PGA, (value.bit.adc_pd = 1;));
}

#if TLK_IS_ENABLED(CONFIG_TLK_ADC_PM_DEVICE)

static void tlk_adc_restore(void)
{
    tlk_adc_configure(&tlk_adc_config);
}

TLK_REGISTER_AFTER_SLEEP(tlk_adc_restore, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#endif
