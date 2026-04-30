# API Documentation

## Overview
The API module provides a high-level abstraction layer for essential system utilities. It consolidates system timing, power management (sleep modes), and debug printing into a unified interface. The Print module acts as a lightweight, formatted output utility that directs debug messages to a configured UART instance.

**Key Features:**

* **System Timing:** Microsecond-precision timestamp generation.
* **Power Management:** Unified sleep function with configurable power modes (Suspend, Deep Sleep, Retention).
* **Debug Output:** `printf`-like formatting capability.
* **UART Integration:** Directs debug output to a specific, user-configurable UART hardware instance.


## API Reference
This section describes the functions available in the API and Print modules.

### 1. `tlk_api_micros`
Returns the current system up-time in microseconds. This function utilizes the system timer (`stimer`) to calculate the tick count.

**Prototype:**

```c
unsigned int tlk_api_micros(void);
```

**Parameters:**
None

**Return Value:**
`unsigned int`: The number of microseconds passed since the system started.


### 2. `tlk_api_sleep`
*This API is available only if `CONFIG_TLK_API_SLEEP` is enabled.*

Checks duration; if zero — returns.
If `CONFIG_TLK_PM` is enabled, it calls `tlk_pm_sleep` with `TLK_PM_SLEEP_MAX_LEVEL-1` parameter.
If `CONFIG_TLK_PM` is disabled, or `tlk_pm_sleep` returned err, it programs the timer for the requested delay, enables timer interrupt, and waits using WFI.

**Prototype:**

```c
void tlk_api_sleep(unsigned int duration_ms);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `duration_ms` | `unsigned int` | The duration to sleep in milliseconds. |

**Return Value:**
None


### 3. `tlk_api_delay`
Blocking delay function

**Prototype:**

```c
void tlk_api_delay(unsigned int duration_us);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `duration_us` | `unsigned int` | The duration to wait in microseconds. |

**Return Value:**
None


### 4. `tlk_print`
*This API is available only if `CONFIG_TLK_DEBUG_PRINT` is enabled.*

A formatted output function similar to the standard C `printf`. It formats the string and transmits it via the configured UART interface.

**Prototype:**

```c
int tlk_api_print(const char *restrict format, ...);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `format` | `const char *` | The format string (e.g., "Value: %d\n"). |
| `...` | `varargs` | Variable arguments corresponding to the format specifiers. |

**Return Value:**
`int`: The number of characters written to the output buffer.

**Note:**
This function uses an internal static buffer defined by `CONFIG_TLK_DEBUG_PRINT_BUFFER_SIZE`. Ensure your formatted message does not exceed this limit.



## API Configuration
This section describes the Kconfig options used to configure the API and Print modules.

### Configurations

```text
PM
│   Enables Power Management support. Required for `api_sleep`.
│
└── SLEEP_MODE
    │   Selects the default power-saving mode used by `api_sleep`.
    │
    ├── SLEEP_MODE_SUSPEND
    │       Standard suspend mode. Fast wake-up time.
    │
    ├── SLEEP_MODE_DEEP_SLEEP
    │       Deep sleep mode. Lower power consumption, longer wake-up time.
    │
    └── SLEEP_MODE_DEEP_RETENTION
            Deep retention mode. Lowest power consumption with state retention.

TLK_DEBUG_PRINT
│   Enables debug output for UART communication.
│
├── TLK_DEBUG_PRINT_UART
│       Selects which UART instance(s) will output debug messages.
│
└── TLK_DEBUG_PRINT_BUFFER_SIZE
        Sets the buffer size for debug messages (Default: 256 bytes).
```