# MCUboot Bootloader Sample
## Overview
This example builds MCUboot as the bootloader stage: it validates and jumps to the signed application in Slot 0, and can optionally expose a UART serial recovery (DFU) console for uploading new images without a debugger.

This sample shows:
- Bootloader startup and handoff to the application.
- Board-specific DFU-entry detection and recovery UART setup.
- UART serial recovery transport configuration for `boot_serial`.

## How It Works

### Initialization
1. **Global interrupts** are enabled via `tlk_core_interrupt_enable()`.
2. **Debug UART0** is configured if `CONFIG_TLK_DEBUG_LOG_INTERFACE_UART` is enabled, so bootloader log messages are visible.
3. `tlk_mcuboot_run()` is called, handing control to the bootloader core.

### Boot Sequence (`tlk_mcuboot_run`)
1. Logs startup and warns if the image is verified against the SDK's development signing key (see `subsystem/mcuboot/port/keys.c`).
2. Initializes the crypto backend used for signature verification.
3. Calls `tlk_mcuboot_recovery_requested()`. If it returns `true`, the bootloader enters serial recovery instead of booting:
   - `tlk_mcuboot_prepare_recovery()` configures the recovery UART and DFU indicator LED.
   - `boot_serial_start()` runs the `boot_serial` console until reset.
4. Otherwise, `boot_go()` locates and validates a bootable image in Slot 0/Slot 1 and the sample jumps to it.
5. If no valid image is found, the bootloader logs an error and halts.

### DFU-Entry Detection
`tlk_mcuboot_recovery_requested()` in `main.c` is checked **once, at boot, before `boot_go()` runs** — the button must be held through power-up/reset, not pressed after the app has already booted - a 30 ms debounce confirms the press.

When recovery is entered, `tlk_mcuboot_prepare_recovery()` turns on `LED1` as a visual indicator and configures the recovery UART (`RECOVERY_UART_TX_PORT`/`PIN` = PC4, `RECOVERY_UART_RX_PORT`/`PIN` = PC5) at 115200 baud on the `tlk_uart_module` selected by `CONFIG_TLK_MCUBOOT_SERIAL_UART`.

## Configuration Options
This sample uses the following configuration settings:

`CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY`
Compiles in `boot_serial` and enables the DFU-entry button check. Disabled by default — without it, the sample only boots the application.

`CONFIG_TLK_MCUBOOT_SERIAL_UART`
Selects which `tlk_uart_module` carries the recovery console (default `TLK_UART0`). The corresponding `TLK_UARTx` must be enabled.

`CONFIG_TLK_MCUBOOT_PRODUCTION_KEY`
Silences the development-key warning once the public key in `subsystem/mcuboot/port/keys.c` has been replaced with a real release key.

`CONFIG_TLK_MCUBOOT_BOOTLOADER_SIZE`, `CONFIG_TLK_MCUBOOT_HEADER_SIZE`, `CONFIG_TLK_MCUBOOT_SLOT_SIZE`, `CONFIG_TLK_MCUBOOT_SCRATCH_SIZE`
Flash partition layout; must match the values used when signing the application image (see below).

## Signing an Application Image
An application built with `CONFIG_TLK_MCUBOOT_MODE_APP` is linked with space reserved for the MCUboot header, but still needs to be signed with `imgtool` before the bootloader will accept it:

```
imgtool sign \
    --header-size   <CONFIG_TLK_MCUBOOT_HEADER_SIZE> \
    --pad-header \
    --slot-size     <CONFIG_TLK_MCUBOOT_SLOT_SIZE> \
    --version       1.0.0 \
    --key           <path-to-signing-key>.pem \
    app.bin app_signed.bin
```

Use the corresponding development signing key for local testing, or generate a real key pair and set `CONFIG_TLK_MCUBOOT_PRODUCTION_KEY`.

## Testing Serial Recovery (DFU)
1. Build and flash this sample with `CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY=y`.
2. Hold `KEY_0` down **through power-up or reset** — `LED1` turns on once recovery mode is entered.
3. Connect to the recovery UART pins at 115200 baud and use an `mcumgr`-compatible client to interact with `boot_serial`, e.g.:
   - `image list` — list images currently in the slots.
   - `image upload` — upload a signed image. By default this targets the **primary** slot; pass the client's image-index flag (e.g. `-n 2`) to target the secondary slot for a normal test/confirm upgrade flow.
   - `image test` / `reset` / `image confirm` — mark the uploaded image for one-time boot or permanent confirmation.
