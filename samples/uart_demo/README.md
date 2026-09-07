# UART Driver Sample
## Overview
This example demonstrates the usage of the UART driver. The sample performs a simple "echo" function: it waits to receive data, and, after successful reception, transmits this data back.

This sample shows:
- if `CONFIG_TLK_UART_USED_AUTO_CONFIG` is disabled:
    * Manual UART configuration using `tlk_uart_configure` and `tlk_uart_pinmux_configure`
    * Manual UART flow configuration using `tlk_uart_flow_control_configure` and `tlk_uart_flow_control_pinmux_configure` if `CONFIG_TLK_UART1_FLOW_CONTROL` is enabled
- Callback-based data reception using `tlk_uart_receive_bytes`.
- Basic data transmission using `tlk_uart_send_bytes`.
- Power management and sleep prevention. (Optional)

## How It Works

### Initialization
1.  **Manual Configuration:** If `CONFIG_TLK_UART_USED_AUTO_CONFIG` is disabled, the sample configures `TLK_UART1` with set parameters.
2.  **Power Management Test:** If `CONFIG_TLK_UART_DEMO_SLEEP` is enabled, the application will first print "Before sleep", enter a system sleep state for 2 seconds, and then print "After sleep" after wake up. This verifies that the UART driver functions correctly works after sleep.
3.  **Interrupts:** Global interrupts are enabled by `tlk_core_interrupt_enable()` to allow the UART's asynchronous callbacks.

### Main Loop
Then application goes to `while` loop to perform the following steps:

1.  **Start receive:**
    It calls `tlk_uart_receive_bytes` to listen for incoming data.
    - It uses the global `data` buffer, which has a `BUFFER_SIZE`.
    - It registers the `rx_handler` function as the completion callback.
    - It sets a 5-second reception timeout (for active waiting mode).
2.  **Sleep Prevention Check (Optional):**
    If `CONFIG_TLK_UART_DEMO_PREVENT` is enabled, the application attempts to enter sleep mode using `tlk_pm_sleep` for 2 seconds while data reception is active.  
    In this case, a correct busy status causes `tlk_pm_sleep` to return `TLK_PM_SLEEP_DENIED`, and the application prints a confirmation message.
3.  **Wait for Callback:**
    After successfully starting the reception, application waits the `rx_done` flag.
4.  **Callback Execution (Interrupt Context):**
    When the UART reception is complete, the driver calls the `rx_handler` function.
    - `rx_handler` sets the `rx_done` flag to `true`.
    - It saves the number of received bytes into the `rx_count` variable.
5.  **Echo Data:**
    Application prints received data back, using the `tlk_uart_send_bytes` function
6.  **Reset and Repeat:**
    The `rx_done` and `rx_count` flags are reset, and the loop begins again.

## Configuration Options
This sample uses the following configuration settings:  
`CONFIG_TLK_UART_DEMO_SLEEP` Enable this to test if UART works correctly after the system wakes up from sleep.  
`CONFIG_TLK_UART_DEMO_PREVENT` Enable this to test if the system correctly blocks sleep while data is being received.  
