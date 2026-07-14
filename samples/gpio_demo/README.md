# GPIO Driver Sample
## Overview
This example demonstrates the usage of the GPIO driver.  
It covers basic GPIO input/output operations, interrupt handling, and sleep API.

This sample shows:
- Basic GPIO configuration and pin control (`tlk_gpio_configure`, `tlk_gpio_pin_write`, `tlk_gpio_pin_toggle`).
- GPIO interrupt usage with callback registration (when `CONFIG_TLK_GPIO_DEMO_INTERRUPT` is enabled).

## How It Works

### Initialization
1. **LED0 Configuration**  
    LED0 is configured as an output.
    The sample toggles it with a 1-second delay inside the main loop to verify GPIO functionality.

2. **Optional Interrupt Configuration (`CONFIG_TLK_GPIO_DEMO_INTERRUPT`)**  
    When enabled:
    - LED1 is configured as output (toggled inside the interrupt callback).
    - KEY2 is configured as **output** and set to high level.
    - KEY0 is configured as **input with pull-down**.
    - A GPIO interrupt callback (`gpio_cb`) is registered.
    - The interrupt is set to trigger on the **rising edge** of KEY0.
    - `tlk_core_interrupt_enable()` enables global interrupts.

    When KEY0 triggers the interrupt, LED1 toggles inside the callback.

### Main Loop
The application enters an infinite `while` loop and performs:

- Periodic toggling of **LED0**
- `tlk_api_delay(TLK_SEC_TO_MS(1000))` followed by `tlk_api_sleep(1000)`
  demonstrating delay and sleep API usage in combination with GPIO output.

## Configuration Options
This sample uses the following configuration settings:

`CONFIG_TLK_GPIO_DEMO_INTERRUPT`  
Enables interrupt-based GPIO demonstration:
- Rising-edge interrupt on KEY0.
- LED1 toggles in the registered callback.
