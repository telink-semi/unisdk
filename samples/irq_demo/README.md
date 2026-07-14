# IRQ Nesting & Preemption Sample

## Overview
A sample application demonstrating interrupts management, prioritization, and nesting using the PLIC (Platform-Level Interrupt Controller), Machine Software Interrupts (PLIC SW) (MSI), and Machine Timer Interrupts (MTIMER) (MTI).

## How It Works
On startup the sample:
1. **MSI Configuration:** Enables the software-triggered interrupt subsystem (PLIC SW) and links `tlk_sample_msi_handler` as the execution callback via `tlk_plic_sw_register_callback`.
2. **MTI Configuration:** Registers `tlk_sample_mti_handler` as the hardware timer-driven exception callback via `tlk_mtimer_register_callback`.
3. **MEI Configuration:** Sets up specific PLIC priorities using three distinct levels:
   - Low-Priority Source (`TLK_SAMPLE_PLIC_SRC_LOW` / Vector 12) set to priority tier 1 (`TLK_IRQ_PRI_LEV1`).
   - High-Priority Source (`TLK_SAMPLE_PLIC_SRC_HIGH` / Vector 13) set to priority tier 2 (`TLK_IRQ_PRI_LEV2`).
   - UART0 peripheral (`UNISDK_PLIC_IRQ_NUM_UART0`) set to priority tier 3 (`TLK_IRQ_PRI_LEV3`) for debugging outputs.
4. **Preemption & M-Mode Activation:** If both `CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC` and `CONFIG_TLK_IRQ_DEMO_PLIC_PREEMPT` are set, the hardware module enables nested interrupt execution via `tlk_plic_preempt_enable()`. The core enabkes interrupt flags (MSIE, MTIE, MEIE) and activates global interrupts via `tlk_core_interrupt_enable()`.
5. **Cascading Nesting Cycle:** Based on the configured Kconfig entry-point option, `main` triggers a pending state for one of the primary vectors (MSI, MTI, or PLIC Low) to jumpstart the pipeline:
   - Once executing inside a handler, it purposely cross-triggers subsequent interrupts by setting pending registers (`tlk_plic_set_pending`, `tlk_plic_sw_set_pending`, or modifying `mtimecmp`).
   - To track the cascade visual path clearly, each handler increments an execution tracking variable (`tlk_log_depth`), indents debug prints dynamically, captures current local CSR `mie` configurations via `tlk_sample_log_mie()`, uses localized static execution variables (`processed`) to avoid infinite recursion loops, and decrements the trace depth upon exit.

## Configuration
All options are exposed via **Kconfig** under *IRQ Demo Configuration*.

| Symbol | Type | Default | Description |
|---|---|---|---|
| `CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC_SW` | `bool` | `n` | Use Machine Software Interrupt (MSI) as the base trigger of the nesting execution. When enabled, `main()` kicks off the sequence by forcing an MSI event state. |
| `CONFIG_TLK_IRQ_DEMO_IRQ_BASE_MTIMER` | `bool` | `n` | Use Machine Timer Interrupt (MTI) as the base trigger or enable nested timer-cascading configurations. When enabled, `main()` kickstarts execution by writing to the timer comparator register. |
| `CONFIG_TLK_IRQ_DEMO_IRQ_BASE_PLIC` | `bool` | `n` | Enable External PLIC interrupt cascading and internal handler tracking. Governs whether high and low external channels trigger nested interrupt evaluations. |
| `CONFIG_TLK_IRQ_DEMO_PLIC_PREEMPT` | `bool` | `n` | Enable real-time preemption capabilities on the PLIC controller level. When disabled, interrupts will be queued linearly instead of nesting dynamically. |
