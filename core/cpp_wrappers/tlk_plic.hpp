#ifndef PLIC_DRIVER_HPP
#define PLIC_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_plic.h"
#ifdef __cplusplus
}
#endif

class PlicDriver
{
  public:
    PlicDriver() = delete;

    static void enable(uint32_t src)
    {
        tlk_plic_interrupt_enable(src);
    }

    static void disable(uint32_t src)
    {
        tlk_plic_interrupt_disable(src);
    }

    static void setPriority(uint32_t src, tlk_irq_priority priority)
    {
        tlk_plic_set_priority(src, priority);
    }

    static void setThreshold(tlk_irq_priority threshold)
    {
        tlk_plic_set_threshold(threshold);
    }

    static void setPending(uint32_t src)
    {
        tlk_plic_set_pending(src);
    }

    static uint32_t claim()
    {
        return tlk_plic_interrupt_claim();
    }

    static void complete(uint32_t src)
    {
        tlk_plic_interrupt_complete(src);
    }

    static int32_t clearAllRequests()
    {
        return tlk_plic_clr_all_request();
    }

    static void preprocessForWfi(uint8_t flag, tlk_mie_e mie)
    {
        tlk_plic_irqs_preprocess_for_wfi(flag, mie);
    }

    static void postprocessForWfi()
    {
        tlk_plic_irqs_postprocess_for_wfi();
    }

    static bool isInIsr()
    {
        return tlk_plic_is_in_isr();
    }

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC_NEST_PLIC)
    static void preemptEnable()
    {
        tlk_plic_preempt_enable();
    }

    static void preemptDisable()
    {
        tlk_plic_preempt_disable();
    }
#endif
};

#endif
