# Power Management (PM) Sample
## Overview
This example demonstrates usage of the Power Management (PM) subsystem.  
It shows how to enter into deep/suspend sleep, configure GPIO wake-up, and preserve application state across low-power transitions.

This sample shows:
- entering into deep/suspend sleep using `tlk_api_sleep`
- GPIO-based wake-up configuration (optional)
- using **before/after sleep hooks**
- retention of variables in deep sleep retention mode

## How It Works

### Initialization
1. **LED State Initialization (optional)**  
   When `CONFIG_TLK_PM_DEMO_ENABLE_LED` is enabled:
   - Two boolean variables (`led0_state`, `led1_state`) are stored in **retention section**
   - Initial states are set:
     - LED0 – ON  
     - LED1 – OFF
   - `init_led()` configures both LEDs as outputs and applies the retained state.

2. **Hook Registration**  
   Several callbacks are registered:
   - `flip_led()` registered as:
     - `TLK_REGISTER_BEFORE_SUSPEND`
     - `TLK_REGISTER_BEFORE_SLEEP`
   - `init_led()` registered as:
     - `TLK_REGISTER_AFTER_SUSPEND`
     - `TLK_REGISTER_AFTER_SLEEP`
     - (also before sleep/suspend)

   Effect:
   - before sleep/suspend → LED states are toggled
   - after wake-up → LED GPIOs are reinitialized and restored

### Main Loop
The application runs an infinite loop that:

1. waits for 1 second using `sys_delay`
2. optionally configures GPIO wake-up
3. optionally disables LEDs before sleep
4. enters low-power mode for 1 second

### GPIO Wakeup (optional)
When `CONFIG_TLK_PM_DEMO_ENABLE_GPIO_INPUT` is enabled:

- GPIO C4 is configured as:
  - wake-up source
  - input with pull-down
- The wake-up level is **HIGH**
- If GPIO wake is enabled:
  - the device enters **suspend**
  - without GPIO wake it may enter **deep sleep**

> Note: when GPIO wakeup is enabled, deep sleep is not entered; the system uses suspend mode instead.

### LED Handling (optional)
When `CONFIG_TLK_PM_DEMO_ENABLE_LED_BEFORE_SLEEP` is enabled:

- before entering sleep:
  - LED0 is turned OFF
  - LED1 is turned OFF


## Configuration Options
This sample uses the following configuration settings:

`CONFIG_TLK_PM_DEMO_ENABLE_LED`  
Enables LED demonstration:
- LED state stored in retention section
- LEDs reinitialized after waking up
- LED state toggled before sleep/suspend

`CONFIG_TLK_PM_DEMO_ENABLE_GPIO_INPUT`  
Enables GPIO wake-up demonstration:
- GPIO C4 configured as wake-up source
- Triggers resume from suspend on HIGH level

`CONFIG_TLK_PM_DEMO_DISABLE_LED_BEFORE_SLEEP`  
Optional behavior:
- LEDs are turned off right before entering sleep
- Useful for visualizing sleep entry
