Here is the updated **PLIC Driver Documentation**.
# PLIC Driver Documentation

## Overview
The PLIC (Platform-Level Interrupt Controller) driver provides an interface for managing system interrupts. It allows developers to configure interrupt priorities, enable or disable specific interrupt sources, and manage interrupt preemption (nesting).
The interrupt handling mode (General vs. Vector) is determined at build time via the system configuration (`.config`).

**Key Features:**
- Build-time selection of global interrupt mode (General vs. Vector).
- Management of interrupt priorities and thresholds.
- Enabling and disabling of specific interrupt sources.
- Support for interrupt preemption (nested interrupts).
- Optional Software Interrupt support (Trigger, Enable, Disable, Claim, Complete).

## Data Types and Enumerations
The following section describes the enumerations and data types used to configure and work with the PLIC driver.

### 1. Interrupt Priorities
**Enum:** `tlk_irq_priority`

Defines the priority level for an interrupt source. Higher numbers indicate higher priority.

| Items | Description |
| --- | --- |
| `TLK_IRQ_PRI_LEV0` | Priority 0. Indicates that no interrupt is generated (effectively disabled). |
| `TLK_IRQ_PRI_LEV1` | Priority Level 1 (Lowest). |
| `TLK_IRQ_PRI_LEV2` | Priority Level 2. |
| `TLK_IRQ_PRI_LEV3` | Priority Level 3 (Highest). |



## API Reference
The following section describes the functions available in the PLIC driver.

### 1. `TLK_PLIC_ISR_REGISTER` (Macro)
A helper macro used to define and register an Interrupt Service Routine (ISR) for a specific interrupt number. It automatically generates the correct function attributes and entry code based on whether Vector Mode or General Mode is enabled in Kconfig.

**Usage:**

```c
TLK_PLIC_ISR_REGISTER(my_uart_handler, (IRQ_UART0))
```

**Parameters:**

| Parameter | Description |
| --- | --- |
| `isr` | The name of the C function to be called as the handler. |
| `irq_num` | The interrupt number (e.g., `IRQ_UART0`) associated with this handler. |


### 2. `tlk_plic_interrupt_enable`
Enables a specific interrupt source.

**Prototype:**

```c
void tlk_plic_interrupt_enable(uint32_t src);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `src` | `uint32_t` | The interrupt source ID. |

**Return Value:**
None


### 3. `tlk_plic_interrupt_disable`
Disables a specific interrupt source.

**Prototype:**

```c
void tlk_plic_interrupt_disable(uint32_t src);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `src` | `uint32_t` | The interrupt source ID. |

**Return Value:**
None


### 4. `tlk_plic_set_priority`
Sets the priority level for a specific interrupt source.

**Prototype:**

```c
void tlk_plic_set_priority(uint32_t src, enum tlk_irq_priority priority);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `src` | `uint32_t` | The interrupt source ID. |
| `priority` | `enum tlk_irq_priority` | The priority level. |

**Return Value:**
None


### 5. `tlk_plic_set_threshold`
Sets the global interrupt threshold. Only interrupts with a priority strictly higher than this threshold will be serviced.

**Prototype:**

```c
void tlk_plic_set_threshold(enum tlk_irq_priority threshold);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `threshold` | `enum tlk_irq_priority` | The minimum priority threshold. |

**Return Value:**
None


### 6. `tlk_plic_set_pending`
Manually sets an interrupt source as pending.

**Prototype:**

```c
void tlk_plic_set_pending(uint32_t src);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `src` | `uint32_t` | The interrupt source ID. |

**Return Value:**
None


### 7. `tlk_plic_clr_all_request`
Clears all pending interrupt requests.

**Prototype:**

```c
int32_t tlk_plic_clr_all_request(void);
```

**Parameters:**
None

**Return Value:**
`int32_t`: Status code (1 for success).


### 8. `tlk_plic_preempt_enable`
Enables interrupt preemption (nested interrupts) for priorities higher than the specified level.

**Prototype:**

```c
void tlk_plic_preempt_enable(tlk_core_preempt_pri_e preempt_pri);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `preempt_pri` | `tlk_core_preempt_pri_e` | The base priority level for preemption. |

**Return Value:**
None


### 9. `tlk_plic_preempt_disable`
Disables interrupt preemption.

**Prototype:**

```c
void tlk_plic_preempt_disable(void);
```

**Parameters:**
None

**Return Value:**
None


### 10. `tlk_plic_isr`
A low-level wrapper function used internally by the vector mode to execute the ISR. Users typically do not call this directly; it is handled by the `TLK_PLIC_ISR_REGISTER` macro.

**Prototype:**

```c
void tlk_plic_isr(tlk_func_isr_t func, uint32_t src);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `func` | `tlk_func_isr_t` | The ISR function to be called. |
| `src` | `uint32_t` | The interrupt number. |

**Return Value:**
None


### 11. `tlk_plic_irqs_preprocess_for_wfi`
Prepares the PLIC controller and interrupt state before the system enters a "Wait For Interrupt" (WFI) or sleep state.

**Prototype:**

```c
void tlk_plic_irqs_preprocess_for_wfi(uint8_t flag, tlk_mie_e mie);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `flag` | `uint8_t` | Specific flag for wakeup configuration. |
| `mie` | `tlk_mie_e` | Machine Interrupt Enable state to preserve. |

**Return Value:**
None


### 12. `tlk_plic_irqs_postprocess_for_wfi`
Restores the PLIC state and interrupt configuration after the system wakes up from WFI or sleep.

**Prototype:**

```c
void tlk_plic_irqs_postprocess_for_wfi(void);
```

**Parameters:**
None

**Return Value:**
None


### 13. `tlk_plic_sw_set_pending`
*This API is available only if `CONFIG_TLK_PLIC_SW` is enabled.*

Manually triggers a software interrupt.

**Prototype:**

```c
void tlk_plic_sw_set_pending(void);
```

**Parameters:**
None

**Return Value:**
None


### 14. `tlk_plic_sw_interrupt_enable`
*This API is available only if `CONFIG_TLK_PLIC_SW` is enabled.*

Enables the software interrupt source.

**Prototype:**

```c
void tlk_plic_sw_interrupt_enable(void);
```

**Parameters:**
None

**Return Value:**
None


### 15. `tlk_plic_sw_interrupt_disable`
*This API is available only if `CONFIG_TLK_PLIC_SW` is enabled.*

Disables the software interrupt source.

**Prototype:**

```c
void tlk_plic_sw_interrupt_disable(void);
```

**Parameters:**
None

**Return Value:**
None


### 16. `tlk_plic_sw_interrupt_claim`
*This API is available only if `CONFIG_TLK_PLIC_SW` is enabled.*

Claims the software interrupt source. This is typically used inside the ISR.

**Prototype:**

```c
uint32_t tlk_plic_sw_interrupt_claim(void);
```

**Parameters:**
None

**Return Value:**
`uint32_t`: The software interrupt source ID.


### 17. `tlk_plic_sw_interrupt_complete`
*This API is available only if `CONFIG_TLK_PLIC_SW` is enabled.*

Sends a completion message for the software interrupt, allowing new requests to be processed.

**Prototype:**

```c
void tlk_plic_sw_interrupt_complete(uint32_t src);
```

**Parameters:**

| Parameter | Type | Description |
| --- | --- | --- |
| `src` | `uint32_t` | The software interrupt source ID. |

**Return Value:**
None



## PLIC Configuration
This section describes the PLIC configuration options available in the Kconfig.

### Configurations

```text
TLK_PLIC_VECTOR_MODE
│   Selects the global interrupt handling mode at build time.
│   If enabled (`y`), Vector Mode is used (interrupts jump directly to their vector address).
│   If disabled (`n`), General Mode is used (all interrupts share a common handler).
│   Default: `y`
│
PLIC_SW
│   Enables support for Software Interrupts.
│   If enabled, APIs like `tlk_plic_sw_set_pending` become available.
│   Default: `n`

```
