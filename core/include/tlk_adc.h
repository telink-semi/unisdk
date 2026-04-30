#ifndef TLK_INCLUDE_DRIVERS_ADC_H_
#define TLK_INCLUDE_DRIVERS_ADC_H_

#include "core/include/tlk_gpio.h"

#if IS_ENABLED(CONFIG_TLK_DMA)
    #include "core/include/tlk_dma.h"
#endif

enum tlk_adc_vref 
{
    TLK_ADC_VREF_0P6V       = 0x00,
    TLK_ADC_VREF_0P9V       = 0x01,
    TLK_ADC_VREF_1P2V       = 0x02,
    TLK_ADC_VREF_ANTI_AGING = 0x03,
};

enum tlk_adc_resolution
{
    TLK_ADC_RESOLUTION_8  = 0x00,
    TLK_ADC_RESOLUTION_10 = 0x01,
    TLK_ADC_RESOLUTION_12 = 0x02,
    TLK_ADC_RESOLUTION_14 = 0x03,
};

enum tlk_adc_prescale
{
    TLK_ADC_PRESCALE_1   = 0x00,
    TLK_ADC_PRESCALE_1F2 = 0x01,
    TLK_ADC_PRESCALE_1F4 = 0x02,
    TLK_ADC_PRESCALE_1F8 = 0x03,
};

enum tlk_adc_sample_freq
{
    TLK_ADC_SAMPLE_FREQ_23K,
    TLK_ADC_SAMPLE_FREQ_48K,
    TLK_ADC_SAMPLE_FREQ_96K,
    TLK_ADC_SAMPLE_FREQ_192K,
};

enum tlk_adc_channel
{
    TLK_ADC_CHANNEL_0,
};

struct tlk_adc_config 
{
    enum tlk_adc_channel channel;
    enum tlk_adc_vref vref;
    enum tlk_adc_prescale prescale;
    enum tlk_adc_sample_freq sample_freq;
    enum tlk_adc_resolution resolution;
    uint32_t timeout_us;
#if IS_ENABLED(CONFIG_TLK_DMA)
    enum tlk_dma_chn dma_channel;
    bool dma_en;
#endif
    bool is_differential;
    struct tlk_gpio_port_pin sampling_port_pin;
    struct tlk_gpio_port_pin differential_port_pin;

};

enum tlk_adc_status {
    TLK_ADC_OK,
    TLK_ADC_UNSUPPORTED,
    TLK_ADC_TIMEOUT,
};

extern struct tlk_adc_config tlk_adc_config;



/**
 * @brief Configure the ADC module.
 * 
 * @param cfg - Desired configuration.
 *
 * @return TLK_ADC_UNSUPPORTED if unsupported parameters are provided, otherwise TLK_ADC_OK.
 */
enum tlk_adc_status tlk_adc_configure(struct tlk_adc_config *cfg);

/**
 * @brief Read the data from ADC.
 * 
 * @param buffer - Buffer to store the read values.
 * @param size - Number of samples, which should not be greater than the size of a buffer.
 *
 * @return TLK_ADC_TIMEOUT if configured timeout exceeded, otherwise TLK_ADC_OK.
 */
enum tlk_adc_status tlk_adc_read(uint16_t *buffer, uint32_t size);

/**
 * @brief Disable the ADC module.
 * 
 * @return None.
 */
void tlk_adc_disable(void);

#endif