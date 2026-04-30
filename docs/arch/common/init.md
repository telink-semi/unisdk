# Init hooks documentation

## Overview

This document describes the init and power-state hook mechanism provided by the `common/include/tlk_init.h` and `common/tlk_init.c` pair.

The system exposes several hook types that components can register. Hooks are collected into dedicated linker sections by registration macros and invoked by the runtime using the functions implemented in `tlk_init.c`.

**Primary goals:**
- Provide ordered initialization hooks (pre-init) for system, driver and application code.
- Provide lifecycle hooks for suspend/sleep transitions (before/after).
- Allow components to prevent suspend/sleep via boolean callbacks.

## Concepts

- **Registration by linker sections:** Registration macros place function pointers into named linker sections (for example, `.pre_init_array_<level>_<priority>`). The build/linker is expected to place these sections in an order that respects level and priority so the runtime sees callbacks in the intended sequence.
- **Invocation:** Each public `tlk_*` function in `tlk_init.c` iterates from a linker-provided `__*_start` to `__*_end` range and calls every non-NULL function pointer it finds.
- **Ordering convention:** Lower numeric values indicate earlier/higher priority. The headers define named level and priority constants (e.g. `TLK_INIT_LEVEL_SYSTEM`/`TLK_INIT_PRIORITY_HIGH`). The final ordering depends on the linker script that arranges the sections, but the numeric convention should be used when registering callbacks.

## Levels & Priorities

- **Levels (examples):**
	- `TLK_INIT_LEVEL_SYSTEM` = 2
	- `TLK_INIT_LEVEL_DRIVER` = 5
	- `TLK_INIT_LEVEL_APPLICATION` = 8
- **Priorities (examples):**
	- `TLK_INIT_PRIORITY_HIGH` = 2
	- `TLK_INIT_PRIORITY_NORMAL` = 5
	- `TLK_INIT_PRIORITY_LOW` = 8

Use the level to group callbacks by broad stage (system/driver/application) and use priority to fine-tune ordering inside that stage. Smaller numbers mean higher precedence. Additional levels or priorities are expected to be added in the future.

## API reference

### Initialization hooks (pre-init)

- **Type:** `typedef void (*tlk_pre_init_func_t)(void)`
- **Registration macro:** `TLK_REGISTER_PRE_INIT(fn, level, priority)` — places the function pointer in `.pre_init_array_<level>_<priority>` and marks it `used` so the linker doesn't discard it.
- **Invoker:** `void tlk_pre_init(void)` — iterates from `__pre_init_start` to `__pre_init_end` and calls each registered function.

Usage example:

```c
void my_driver_init(void) {
		/* driver init work */
}

TLK_REGISTER_PRE_INIT(my_driver_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_HIGH);
```

### Suspend hooks (before / after)

- **Types:**
	- `typedef void (*tlk_before_suspend_func_t)(void)`
	- `typedef void (*tlk_after_suspend_func_t)(void)`
- **Registration macros:**
	- `TLK_REGISTER_BEFORE_SUSPEND(fn, level, priority)` → section `.before_suspend_array_<level>_<priority>`
	- `TLK_REGISTER_AFTER_SUSPEND(fn, level, priority)` → section `.after_suspend_array_<level>_<priority>`
- **Invokers:**
	- `void tlk_before_suspend(void)` — calls all registered before-suspend callbacks.
	- `void tlk_after_suspend(void)` — calls all registered after-suspend callbacks.

Usage example:

```c
void save_peripherals(void) { /* ... */ }

TLK_REGISTER_BEFORE_SUSPEND(save_peripherals, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL);
```

### Sleep hooks (before / after)

- **Types:**
	- `typedef void (*tlk_before_sleep_func_t)(void)`
	- `typedef void (*tlk_after_sleep_func_t)(void)`
- **Registration macros:**
	- `TLK_REGISTER_BEFORE_SLEEP(fn, level, priority)` → section `.before_sleep_array_<level>_<priority>`
	- `TLK_REGISTER_AFTER_SLEEP(fn, level, priority)` → section `.after_sleep_array_<level>_<priority>`
- **Invokers:**
	- `void tlk_before_sleep(void)`
	- `void tlk_after_sleep(void)`

Usage example:

```c
void board_prepare_sleep(void) { /* disable sensors, clocks, ... */ }

TLK_REGISTER_BEFORE_SLEEP(board_prepare_sleep, TLK_INIT_LEVEL_SYSTEM, TLK_INIT_PRIORITY_HIGH);
```

### Prevent-suspend / prevent-sleep callbacks

- **Types:**
	- `typedef bool (*tlk_prevent_suspend_func_t)(void)`
	- `typedef bool (*tlk_prevent_sleep_func_t)(void)`
- **Registration macros:**
	- `TLK_REGISTER_PREVENT_SUSPEND(fn)` → section `.prevent_suspend_array`
	- `TLK_REGISTER_PREVENT_SLEEP(fn)` → section `.prevent_sleep_array`
- **Invokers and semantics:**
	- `bool tlk_prevent_suspend(void)` — returns `true` as soon as any registered `tlk_prevent_suspend_func_t` returns `true`; otherwise returns `false`.
	- `bool tlk_prevent_sleep(void)` — same behaviour for sleep.

Usage example:

```c
bool uart_busy(void) {
		/* return true if UART has pending transmissions */
}

TLK_REGISTER_PREVENT_SUSPEND(uart_busy);
```

## Additional notes
The functions provided by this API **should not** be invoked by the user. 
They are called automatically on the corresponding lifecycle stages.