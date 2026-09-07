# U-mode Driver Sample


## Overview

A sample application demonstrating the TLK U-mode API. It switches the core from Machine
mode (M-mode) to User mode (U-mode), executes a task under reduced privilege, optionally
dispatches a service request back to M-mode via `ecall`, and optionally returns to M-mode
permanently through `tlk_core_enable_mmode`.


## How It Works

On startup the sample:

1. Registers `ecall_handler` as the ecall exception callback via
   `tlk_trap_register_ecall_callback`.
2. Configures PMP entry 7 to grant full RWX access to the entire 4 GB address space —
   this is the baseline permission for U-mode.
3. *(Optional)* If `CONFIG_TLK_UMODE_DEMO_ENABLE_UART` is **not** set, configures PMP
   entry 0 to deny all access to the UART peripheral region. Because PMP entries are
   checked from lowest index first, entry 0 overrides entry 7 for that range, blocking
   any UART access from U-mode.
4. Calls `tlk_core_enable_umode(user_task, task_handler)` to switch to U-mode:
   - **`user_task`** executes in U-mode and attempts to send `"User task.\n"` over
     UART0. Succeeds only if `CONFIG_TLK_UMODE_DEMO_ENABLE_UART` is set; otherwise a
     PMP violation trap fires on the first UART register access. If
     `CONFIG_TLK_UMODE_DEMO_ENABLE_SLEEP` is set, `user_task` also sets
     `USER_REQUEST_SLEEP` before returning.
   - After `user_task` returns, **`task_handler`** executes in U-mode. This function calls the `ecall`.
   - The `ecall` causes an environment-call trap which transfers control to M-mode
     where **`ecall_handler`** runs:
     - If `USER_REQUEST_SLEEP` is set, it logs `"Ecall. Sleep 1s.\n"`, sleeps for
       1 second via `tlk_api_sleep`, then clears the request.
     - If `CONFIG_TLK_UMODE_DEMO_ENABLE_MMODE` is set, it calls `tlk_core_enable_mmode()`
       to permanently restore M-mode privilege before returning.
5. After `tlk_core_enable_umode` returns, `main` performs a privilege check: it attempts to
   read the `mstatus` CSR. This read succeeds in M-mode and raises an
   illegal-instruction exception in U-mode. If `CONFIG_TLK_UMODE_DEMO_ENABLE_MMODE` is
   not set, `ecall_handler` never calls `tlk_core_enable_mmode()`, so execution returns to
   `main` still in U-mode and the read traps.


## Configuration

All options are exposed via **Kconfig** under *U-mode Demo Configuration*.

| Symbol | Type | Default | Description |
|---|---|---|---|
| `CONFIG_TLK_UMODE_DEMO_ENABLE_UART` | `bool` | `n` | Allow U-mode access to the UART peripheral. When disabled, PMP entry 0 is configured to deny all access to the UART region, causing a PMP violation trap on any UART access attempted from U-mode. |
| `CONFIG_TLK_UMODE_DEMO_ENABLE_SLEEP` | `bool` | `n` | Enable the sleep service request. When set, `user_task` issues a `USER_REQUEST_SLEEP` request before returning, causing `ecall_handler` to perform a 1-second sleep in M-mode. |
| `CONFIG_TLK_UMODE_DEMO_ENABLE_MMODE` | `bool` | `n` | Return to M-mode after the ecall. When set, `ecall_handler` calls `tlk_core_enable_mmode()` so that execution continues in `main` with full M-mode privilege. When disabled, `main` remains in U-mode after `tlk_core_enable_umode` returns, and the subsequent `mstatus` CSR read raises an illegal-instruction exception. |

> **Note:** With all options disabled the sample demonstrates two protections in
> sequence: a PMP violation trap when `user_task` touches the blocked UART region, and
> an illegal-instruction trap when `main` attempts to read a privileged CSR from U-mode.
