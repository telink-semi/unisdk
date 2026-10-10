#ifndef PM_DRIVER_HPP
#define PM_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_pm.h"
#ifdef __cplusplus
}
#endif

class PMDriver
{
  public:
    explicit PMDriver() = delete;

    static tlk_pm_wakeup_source getWakeupReason(void)
    {
        return tlk_pm_get_wakeup_reason();
    }

    static void clearWakeupSources(void)
    {
        tlk_pm_clear_wakeup_sources();
    }

    static tlk_pm_sleep_status sleep(tlk_pm_sleep_mode mode, uint32_t duration_ms)
    {
        return tlk_pm_sleep(mode, duration_ms);
    }

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO)
    static tlk_pm_sleep_status setGpioWakeup(tlk_gpio_port port, tlk_gpio_pin pin,
                                             tlk_pm_gpio_wakeup_level polarity)
    {
        return tlk_pm_set_gpio_wakeup(port, pin, polarity);
    }
#endif
};

#endif // PM_DRIVER_HPP
