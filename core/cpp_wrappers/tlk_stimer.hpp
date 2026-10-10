#ifndef STIMER_DRIVER_HPP
#define STIMER_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_stimer.h"
#ifdef __cplusplus
}
#endif

class STimerDriver
{
  public:
    STimerDriver() = delete;

    static void enable()
    {
        tlk_stimer_enable();
    }

    static void disable()
    {
        tlk_stimer_disable();
    }

    static void setIrqMask(tlk_stimer_irq_mask_e mask)
    {
        tlk_stimer_set_irq_mask(mask);
    }

    static void clrIrqMask(tlk_stimer_irq_mask_e mask)
    {
        tlk_stimer_clr_irq_mask(mask);
    }

    static void clrIrqStatus(tlk_stimer_irq_status_e status)
    {
        tlk_stimer_clr_irq_status(status);
    }

    static void setIrqCapture(uint32_t tick)
    {
        tlk_stimer_set_irq_capture(tick);
    }

    static void setTick(uint32_t tick)
    {
        tlk_stimer_set_tick(tick);
    }

    static void setAutoMode(bool enable)
    {
        tlk_stimer_set_auto_mode(enable);
    }

    static void set32kWrite(bool enable)
    {
        tlk_stimer_set_32k_write(enable);
    }

    static tlk_stimer_irq_status_e getIrqStatus(tlk_stimer_irq_status_e status)
    {
        return tlk_stimer_get_irq_status(status);
    }

    static uint32_t getTick()
    {
        return tlk_stimer_get_tick();
    }

    static void set32kClockTracking(bool enable)
    {
        tlk_stimer_set_32k_clock_tracking(enable);
    }

    static tlk_stimer_state_e getState()
    {
        return tlk_stimer_get_state();
    }

    static void setState(tlk_stimer_state_e state)
    {
        tlk_stimer_set_state(state);
    }

    static void registerCallback(void (*cb)(void))
    {
        tlk_stimer_register_callback(cb);
    }
};

#endif
