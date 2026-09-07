#include "core/cpp_wrappers/tlk_adc.hpp"
#include "core/cpp_wrappers/tlk_gpio.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "system/debug/log/tlk_log.h"
#include <string.h>

#if TLK_IS_ENABLED(CONFIG_TLK_ADC_DEMO_PM)
#include "api/include/tlk_sleep.h"
#endif

#ifdef __cplusplus
}
#endif

#define ADC_BUFFER_SIZE 8

TLK_LOG_CREATE(adc_demo, "ADC_DEMO");

uint16_t adc_values[ADC_BUFFER_SIZE] __attribute__((aligned(4)));

int main(void)
{
    struct tlk_adc_config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.channel                    = TLK_ADC_CHANNEL_0;
    cfg.prescale                   = TLK_ADC_PRESCALE_1F4;
    cfg.resolution                 = TLK_ADC_RESOLUTION_12;
    cfg.sample_freq                = TLK_ADC_SAMPLE_FREQ_96K;
    cfg.vref                       = TLK_ADC_VREF_1P2V;
    cfg.sampling_port_pin.port     = TLK_GPIO_PORT_B;
    cfg.sampling_port_pin.pin      = TLK_GPIO_PIN_2;
    cfg.is_differential            = false;
    cfg.differential_port_pin.port = TLK_GPIO_PORT_D;
    cfg.differential_port_pin.pin  = TLK_GPIO_PIN_0;
    cfg.timeout_us                 = 10000;
#if TLK_IS_ENABLED(CONFIG_TLK_DMA)
    cfg.dma_en      = false;
    cfg.dma_channel = tlk_dma_request_chn();
#endif

    ADCDriver adc(cfg);
    adc.configure();

    tlk_core_interrupt_enable();

    while (1)
    {
        enum tlk_adc_status status = adc.read(adc_values, ADC_BUFFER_SIZE);

        if (status == TLK_ADC_OK)
        {
            for (uint32_t i = 0; i < ADC_BUFFER_SIZE; i++)
            {
                TLK_LOG_INFO(adc_demo, "%u", adc_values[i]);
            }
        }
        else
        {
            TLK_LOG_ERROR(adc_demo, "Timeout");
        }
#if TLK_IS_ENABLED(CONFIG_TLK_ADC_DEMO_PM)
        tlk_api_sleep(TLK_SEC_TO_MS(1));
#else
        tlk_api_delay(TLK_SEC_TO_US(1));
#endif
    }
    return 0;
}
