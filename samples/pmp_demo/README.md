```markdown
# PMP Driver Sample


## Overview
A sample application demonstrating the usage of the TLK PMP API. It protects the upper half
of a 16-element array against all access, then attempts to read or write the full array,
triggering a PMP violation trap on the protected region.


## How It Works
On startup the sample:

1. *(Optional)* Configures PMP protection on `demo_array[8..15]` using the selected address
   matching scheme. All permissions are disabled and the entry is locked (`l = 1`).
2. *(Optional)* If `CONFIG_TLK_PMP_DEMO_SLEEP` is set, enters sleep for 500 ms and wakes up,
   demonstrating that PMP state is correctly saved and restored across the sleep.
3. Iterates over all 16 elements of `demo_array`:
   - With `CONFIG_TLK_PMP_DEMO_READ` — reads and logs each value.
   - With `CONFIG_TLK_PMP_DEMO_WRITE` — writes and logs each value.

Elements `[0..7]` complete successfully. On the first access to `demo_array[8]` a PMP
violation trap is raised.


## Configuration
All options are exposed via **Kconfig** under *PMP Demo Configuration*.

| Symbol | Type | Default | Description |
|---|---|---|---|
| `TLK_PMP_DEMO_TOR` | `bool` | `n` | Protect `demo_array[8..15]` using TOR address matching. Entry 0 holds the lower bound; Entry 1 holds the upper bound. |
| `TLK_PMP_DEMO_NAPOT` | `bool` | `n` | Protect `demo_array[8..15]` using NAPOT address matching. |
| `TLK_PMP_DEMO_READ` | `bool` | `n` | Access the array by reading. |
| `TLK_PMP_DEMO_WRITE` | `bool` | `n` | Access the array by writing. |
| `TLK_PMP_DEMO_SLEEP` | `bool` | `n` | Enter the sleep between PMP configuration and array access. Demonstrates PMP state retention across sleep. |

> **Note:** If neither `TLK_PMP_DEMO_TOR` nor `TLK_PMP_DEMO_NAPOT` is set, no protection
> is configured and all 16 elements are accessible without a fault.
