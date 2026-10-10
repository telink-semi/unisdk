#ifndef ADC_DRIVER_HPP
#define ADC_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_adc.h"
#ifdef __cplusplus
}
#endif

class ADCDriver
{
  public:
    explicit ADCDriver(const struct tlk_adc_config& config) : config_(config) {}

    ~ADCDriver()
    {
        disable();
    }

    tlk_adc_status configure()
    {
        return tlk_adc_configure(&this->config_);
    }

    tlk_adc_status read(uint16_t* buffer, uint32_t size)
    {
        return tlk_adc_read(buffer, size);
    }
    void disable()
    {
        tlk_adc_disable();
    }

  private:
    struct tlk_adc_config config_;
};

#endif // ADC_DRIVER_HPP
