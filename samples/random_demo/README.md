# Random Driver Sample


## Overview
A sample application demonstrating the usage of the TLK Random API. It generates a configurable sequence of random numbers and logs them to the console in a loop.


## How It Works
On every loop iteration the sample:

1. Generates `CONFIG_TLK_RANDOM_DEMO_NUMBERS_COUNT` random values in the range `[0, CONFIG_TLK_RANDOM_DEMO_MAX_NUMBER]` using `tlk_random()` and logs each one.
2. *(Optional)* If `USE_TERO_MODULE` is set to `1`, repeats the same sequence using `tlk_random_tero()` - a hardware TERO-based source available only on **TL721X**.
3. Waits before the next iteration:
   - Without `CONFIG_TLK_RANDOM_DEMO_SLEEP` - delay via `tlk_api_delay(TLK_USEC(4))`.
   - With `CONFIG_TLK_RANDOM_DEMO_SLEEP` - sleep via `tlk_api_sleep(TLK_MSEC(4))`.


## Configuration
All options are exposed via **Kconfig** under *Random Demo Configuration*.

| Symbol | Type | Default | Description |
|---|---|---|---|
| `TLK_RANDOM_DEMO_NUMBERS_COUNT` | `int` | `10` | How many numbers to generate per iteration. Range: `1–100`. |
| `TLK_RANDOM_DEMO_MAX_NUMBER` | `int` | `10` | Upper bound (inclusive) of the generated values. |
| `TLK_RANDOM_DEMO_SLEEP` | `bool` | `n` | Use sleep instead of delay between iterations. Automatically selects `TLK_API_SLEEP` and implies `TLK_RANDOM_PM`. |

> **Note:** `TLK_RANDOM_DEMO` auto-selects `TLK_RANDOM` and `TLK_API_TIME` - no manual selection required.


## TERO Module (TL721X only)
The TERO-based random source is **disabled by default**. To enable it, edit `main.c` and set:

```c
#define USE_TERO_MODULE 1
```
