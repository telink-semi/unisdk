# Interrupt Nesting Demo Application

## Overview
This sample demonstrates interrupt nesting configurations using different interrupt sources.

## Hardware Setup
> **Important:** For the GPIO interrupt example, you must short the **LED2** and **KEY1** pins **after reset**.

> **Important:** Make sure to disable nesting
for other sources, except currently used.
For example, if use nesting in external interrupt nesting for
machine timer and software interrupts should be disabled.


To view the logs, connect a UART converter to **TLK_UART1**.

## Interrupt Sources
The main core handles three distinct interrupt sources:
- External Interrupt (PLIC)
- Software Interrupt (PLIC_SW)
- Machine Timer Interrupt

Each source is handled by software and supports nesting, with the exception of external interrupts when the PLIC is configured in **Vectored Mode**.

### External Interrupts (PLIC)
The PLIC (Platform-Level Interrupt Controller) manages all peripheral interrupts. It evaluates priority and threshold levels **internally**.

From a software perspective, you only need to enable the necessary sources and set the threshold level (if required). The PLIC automatically handles arbitration: if multiple interrupts trigger simultaneously, it selects the one with the highest priority above the threshold. The PLIC then sends an external interrupt signal to the core. The subsequent behavior depends on the configured mode:

#### Preemptive Mode (Default)
The core enters a `trap` and reads the `mcause` register to identify the source. Since it is an external interrupt, the core jumps to the general external interrupt handler (`tlk_plic_handler`).
1. The handler queries the PLIC for the specific Interrupt ID.
2. Before jumping to the specific peripheral driver, the handler saves the current context.
3. Global interrupts are re-enabled (they are automatically disabled when entering a trap).
4. **Result:** Other interrupts can now preempt (nest into) the current ISR.

#### Vectored Mode
In this mode, the core bypasses the general handler and jumps directly to the specific peripheral handler.
- **Advantage:** Reduces interrupt handling latency.
- **Limitation:** Because there is no general handler to save context and re-enable global interrupts safely, **nesting is impossible** inside an external interrupt in vectored mode.

### Software Interrupts (PLIC_SW)
Software interrupts are handled by a separate PLIC instance with a single source. This mechanism is simpler than external interrupts; the core always jumps to a general software interrupt handler, which supports context saving and nesting.
- **Trigger:** Set by software using `tlk_plic_sw_set_pending`.

### Machine Timer Interrupts
Functionally similar to software interrupts, these always utilize a general handler that supports nesting.
- **Trigger:** Occurs when the `mtime` counter exceeds the `mtime_compare` value.

## Configuration
### Host Source Configuration
This configuration determines which interrupt source will act as the "host" allowing others to nest inside it.
- **Example:** If `TLK_NEST_INTO_EXT` is enabled, the system allows other interrupts to trigger **while an external interrupt is currently being handled**.

### Nesting Source Configuration
The options `TLK_NEST_SW`, `TLK_NEST_EXT`, and `TLK_NEST_MTIMER` enable the specific sources allowed to interrupt the host.
- **Example:** If the Host is set to External (`TLK_NEST_INTO_EXT`) and `TLK_NEST_SW` + `TLK_NEST_MTIMER` are enabled, the core will pause the external interrupt handler to process a software or timer interrupt, then resume the external handler.
