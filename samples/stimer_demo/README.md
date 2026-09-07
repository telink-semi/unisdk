# STIMER Driver Sample

## Overview
This example demonstrates the usage of the **System Timer (STIMER)** driver on Telink RISC-V MCUs.
It showcases timer initialization, interrupt configuration, periodic callbacks, and integration with GPIO and sleep APIs.

This sample shows:
- System timer configuration and start (`tlk_stimer_enable`, `tlk_stimer_set_tick`, `tlk_stimer_set_auto_mode`).
- Timer interrupt setup and callback registration (`tlk_stimer_register_callback`).
- Periodic interrupt generation using capture compare.
- GPIO control inside both main loop and interrupt context.
- Low-power sleep usage in conjunction with timer-driven events.

---

## How It Works

### Initialization

1. **GPIO Configuration**
   - **LED0** is configured as output and toggled in the main loop.
   - **LED1** is configured as output and toggled inside the timer interrupt callback.

2. **System Timer Setup**
   - The system timer is enabled using `tlk_stimer_enable()`.
   - The timer tick counter is reset to `0` using `tlk_stimer_set_tick(0)`.
   - Auto mode is enabled (`tlk_stimer_set_auto_mode(true)`), allowing continuous operation.

3. **Interrupt Configuration (`CONFIG_TLK_STIMER_INTERRUPT`)**
   When enabled:
   - A callback function (`timer_cb`) is registered using `tlk_stimer_register_callback()`.
   - The first interrupt is scheduled using:
     ```c
     tlk_stimer_irq_set_capture(tlk_stimer_get_tick() + TLK_SYSTEM_TIMER_TICK_1MS * 500);
     ```
   - System timer interrupt mask is enabled via:
     ```c
     tlk_stimer_irq_set_mask(FLD_SYSTEM_IRQ);
     ```
   - PLIC interrupt for STIMER is enabled:
     ```c
     tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_STIMER);
     ```
   - Global interrupts are enabled using:
     ```c
     tlk_core_interrupt_enable();
     ```

---

### Timer Callback

The `timer_cb()` function executes on every timer interrupt:

- Schedules the next interrupt (every 500 ms).
- Clears the interrupt status.
- Toggles **LED1**.

```c
void timer_cb(void)
{
    tlk_stimer_irq_set_capture(tlk_stimer_get_tick() + TLK_SYSTEM_TIMER_TICK_1MS * 500);
    tlk_stimer_irq_clr_status(FLD_SYSTEM_IRQ);
    tlk_gpio_pin_toggle(UNISDK_ALIAS_LED_1_PORT, UNISDK_ALIAS_LED_1_PIN);
}
