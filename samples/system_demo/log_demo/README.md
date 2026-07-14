# Task Planner Sample

## Overview

This example demonstrates the usage of the Task Planner on the Telink
RISC-V MCU platform.

It covers:

- Basic GPIO configuration and pin control
- Cooperative loop-based execution
- Task scheduler-based execution
- Optional software timer integration

The demo shows how GPIO can be used in different execution models
depending on the enabled configuration.

------------------------------------------------------------------------

## What This Sample Demonstrates

- GPIO output configuration using: `tlk_gpio_configure`
- GPIO pin control using: `tlk_gpio_pin_toggle`
- Cooperative loop execution model
- Scheduler-based task execution
- Periodic execution using software timers (when enabled)

------------------------------------------------------------------------

## How It Works

### 1. Initialization (`user_setup()`)

During system startup:

- **LED0**, **LED1**, and **LED2** are configured as GPIO outputs.
If `CONFIG_TLK_SOFTWARE_TIMERS` is enabled:
- A periodic software timer (`test_timer1`) is started.
- The timer toggles **LED2** every 1000 ms.

This function is executed once during platform initialization.

------------------------------------------------------------------------

## Execution Models

The behavior of this sample depends on the enabled task planner mode.

------------------------------------------------------------------------

### Mode 1: Loop Mode

Enabled with:

    CONFIG_TLK_TASK_PLANNER_MODE_LOOP=y

#### Loop Mode: Behavior

- `user_loop()` is executed repeatedly by the platform dispatcher:

    while (1) {
        tlk_loop();
    }

- `user_loop()` toggles **LED0** every 500 ms using
    `tlk_api_delay_us()`.
- An additional loop handler `loop1()` is registered using:

    TLK_REGISTER_LOOP(loop1);

- `loop1()` toggles **LED1** every 500 ms.

#### Loop Mode: Summary

- LED0 toggles periodically inside `user_loop()`
- LED1 toggles via a registered loop callback
- Execution resembles the Arduino `loop()` style model

------------------------------------------------------------------------

### Mode 2: Scheduler Mode

Enabled with:

    CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER=y

#### Scheduler Mode: Behavior

- `user_loop()` is executed from the default task.
- The execution period is defined by:

    CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK_LOOP_PERIOD_MS

- `loop1()` is defined as a task:

    TASK_SPAWN(test_task1, loop1);

- `loop1()` toggles **LED1** every 500 ms using:

    TASK_DELAY(t, TASK_MS(500));

#### Scheduler Mode: Summary

- LED0 toggles periodically via the default task
- LED1 toggles via a cooperative scheduled task
- Demonstrates task-based periodic execution

------------------------------------------------------------------------

### Optional: Software Timer

Enabled with:

    CONFIG_TLK_SOFTWARE_TIMERS=y

#### Software Timer: Behavior

- A periodic software timer (`test_timer1`) is statically registered.
- It executes `sw_timer_loop()` every 1000 ms.
- `sw_timer_loop()` toggles **LED2**.

#### Software Timer: Summary

- LED2 toggles every 1 second
- Demonstrates timer-based periodic execution
- Uses linker-based static timer registration

------------------------------------------------------------------------

## GPIO Behavior Summary

  |LED   |Controlled By            |Mode Dependency  |
  |------|-------------------------|-----------------|
  |LED0  |`user_loop()`            |Loop / Scheduler |
  |LED1  |`loop1()`                |Loop / Scheduler |
  |LED2  |Software Timer Callback  |SOFTWARE_TIMERS  |

------------------------------------------------------------------------

## Configuration Options

### `CONFIG_TLK_TASK_PLANNER_MODE_LOOP`

Enables cooperative loop-based execution.

### `CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER`

Enables task scheduler-based execution.

### `CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK`

Enables periodic execution of `user_loop()` in scheduler mode.

### `CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK_LOOP_PERIOD_MS`

Defines execution period for `user_loop()` in scheduler mode.

### `CONFIG_TLK_SOFTWARE_TIMERS`

Enables periodic software timer functionality (LED2 toggle).

------------------------------------------------------------------------

## Sample Summary

This sample demonstrates three different ways to implement periodic GPIO
control:

1. Cooperative loop execution
2. Task-based scheduler execution
3. Software timer-based execution

It serves as a reference for:

- Basic GPIO driver usage
- Task planner integration
- Timer subsystem usage
- Linker-based static registration mechanisms
