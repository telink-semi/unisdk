# MCUboot Bootloader Sample
## Overview
This example builds MCUboot as the bootloader stage: it validates and jumps to the signed application in Slot 0, and can optionally expose a serial recovery (DFU) console — over UART or USB CDC ACM — for uploading new images without a debugger.

This sample shows:
- Bootloader startup and handoff to the application.
- Board-specific DFU-entry detection and recovery transport setup.
- UART or USB CDC ACM serial recovery transport configuration for `boot_serial`.

## How It Works

### Initialization
1. **Global interrupts** are enabled via `tlk_core_interrupt_enable()`.
2. **Debug UART0** is configured if `CONFIG_TLK_DEBUG_LOG_INTERFACE_UART` is enabled, so bootloader log messages are visible.
3. `tlk_mcuboot_run()` is called, handing control to the bootloader core.

### Boot Sequence (`tlk_mcuboot_run`)
1. Logs startup and warns if the image is verified against the SDK's development signing key (see `subsystem/mcuboot/port/keys.c`).
2. Initializes the crypto backend used for signature verification.
3. Calls `tlk_mcuboot_recovery_requested()`. If it returns `true`, the bootloader enters serial recovery instead of booting:
   - `tlk_mcuboot_prepare_recovery()` configures the recovery transport (UART pins, or nothing extra for USB) and the DFU indicator LED.
   - `boot_serial_start()` runs the `boot_serial` console until reset, over whichever transport is selected in Kconfig.
4. Otherwise, `boot_go()` locates and validates a bootable image in Slot 0/Slot 1 and the sample jumps to it.
5. If no valid image is found, the bootloader logs an error and halts.

### DFU-Entry Detection
`tlk_mcuboot_recovery_requested()` in `main.c` is checked **once, at boot, before `boot_go()` runs**, and enters recovery if *either* of two independent triggers fires:

- **Button**: the `KEY_0` button must be held through power-up/reset, not pressed after the app has already booted -- a 30 ms debounce confirms the press.
- **Magic word**: an application running under `CONFIG_TLK_MCUBOOT_MODE_APP` can request recovery in software by calling `tlk_mcuboot_request_recovery()` (declared in `subsystem/mcuboot/include/tlk_mcuboot_recovery.h`) and then resetting. This writes a magic/inverted-magic pair to a small fixed-address RAM mailbox (`.boot_mailbox`, carved out of the top of physical IRAM in `flash_boot.link`, independent of PM retention settings) that survives a warm reset. `tlk_mcuboot_recovery_mailbox_pending()` checks and consumes it on the bootloader side -- the request is one-shot, so a normal reset afterward boots the application again.

When recovery is entered (by either trigger), `tlk_mcuboot_prepare_recovery()` turns on `LED1` as a visual indicator. With the UART transport it also configures the recovery UART (`RECOVERY_UART_TX_PORT`/`PIN` = PC4, `RECOVERY_UART_RX_PORT`/`PIN` = PC5) at 115200 baud on the `tlk_uart_module` selected by `CONFIG_TLK_MCUBOOT_SERIAL_UART`. With the USB transport, the device enumerates as a CDC ACM virtual COM port instead — no extra pin setup is needed, `tlk_mcuboot_serial_usb_init()` brings up the USB stack directly.

## Configuration Options
This sample uses the following configuration settings:

`CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY`
Compiles in `boot_serial` and enables serial recovery, including both DFU-entry detection mechanisms (`KEY_0` and the `tlk_mcuboot_request_recovery()` mailbox). Disabled by default — without it, the sample only boots the application.

`CONFIG_TLK_MCUBOOT_SERIAL_TRANSPORT_UART` / `CONFIG_TLK_MCUBOOT_SERIAL_TRANSPORT_USB`
Selects which transport carries the recovery console (default: UART). Only one can be active at a time.

`CONFIG_TLK_MCUBOOT_SERIAL_UART`
UART transport only. Selects which `tlk_uart_module` carries the recovery console (default `TLK_UART0`). The corresponding `TLK_UARTx` must be enabled.

For USB transport, device descriptor fields come from the TinyUSB "Device parameters" Kconfig menu (`CONFIG_TLK_USB_VENDOR_ID`, `CONFIG_TLK_USB_PRODUCT_ID`, `CONFIG_TLK_USB_DEVICE_MANUFACTURER`, etc.); CDC endpoint addresses and buffer sizes come from "CDC ACM settings" (`CONFIG_TLK_USB_CDC_NOTIF_EP`, `CONFIG_TLK_USB_CDC_OUT_EP`, `CONFIG_TLK_USB_CDC_IN_EP`, `CONFIG_TLK_USB_CDC_RX_BUF_SIZE`, `CONFIG_TLK_USB_CDC_TX_BUF_SIZE`).

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

Concrete example using the Kconfig defaults (`CONFIG_TLK_MCUBOOT_HEADER_SIZE=0x200`, `CONFIG_TLK_MCUBOOT_SLOT_SIZE=0xB0000`):

```bash
imgtool sign \
    --header-size   0x200 \
    --pad-header \
    --slot-size     0xB0000 \
    --version       1.0.0 \
    --key           my-release-key.pem \
    app.bin app_signed.bin
```

The SDK's development private key (matching the public key baked into `subsystem/mcuboot/port/keys.c`) ships with the SDK docs, not in this repo — `*.pem` is git-ignored on purpose. For local testing without it, or before release, generate your own pair, matching `subsystem/mcuboot/port/keys.c`'s own instructions:

```bash
imgtool keygen -k my-release-key.pem -t ecdsa-p256
imgtool getpub -k my-release-key.pem
```

Copy the printed public key bytes into `mcuboot_dev_ec256_pub_key[]` in `subsystem/mcuboot/port/keys.c`, then set `CONFIG_TLK_MCUBOOT_PRODUCTION_KEY` and sign with `--key my-release-key.pem` as above.

## Testing Serial Recovery (DFU)
1. Build and flash this sample with `CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY=y` and the desired transport (`CONFIG_TLK_MCUBOOT_SERIAL_TRANSPORT_UART` or `CONFIG_TLK_MCUBOOT_SERIAL_TRANSPORT_USB`).
2. Enter recovery mode by either trigger — `LED1` turns on once recovery mode is entered:
   - Hold `KEY_0` down **through power-up or reset**.
   - Or, from an app built with `CONFIG_TLK_MCUBOOT_MODE_APP`, call `tlk_mcuboot_request_recovery()` and reset — the bootloader picks up the mailbox flag on the next boot.
3. Connect to `boot_serial` and use an `mcumgr`-compatible client:
   - UART transport: connect to the recovery UART pins at 115200 baud.
   - USB transport: plug in the board — it enumerates as a CDC ACM virtual COM port — and connect to that port at any baud rate (ignored by CDC ACM).
4. Interact with `boot_serial`, e.g.:
   - `image list` — list images currently in the slots.
   - `image upload` — upload a signed image. By default this targets the **primary** slot; pass the client's image-index flag (e.g. `-n 2`) to target the secondary slot for a normal test/confirm upgrade flow.
   - `image test` / `reset` / `image confirm` — mark the uploaded image for one-time boot or permanent confirmation.

Example session with the `mcumgr` CLI, UART transport (Windows `COM5`; on Linux/macOS use e.g. `/dev/ttyUSB0`):

```bash
mcumgr --conntype serial --connstring "dev=COM5,baud=115200" image list

# Upload to the secondary slot (-n 2) for a normal test/confirm upgrade
mcumgr --conntype serial --connstring "dev=COM5,baud=115200" image upload app_signed.bin -n 2

# Hash comes from the "image list" output above
mcumgr --conntype serial --connstring "dev=COM5,baud=115200" image test <image-hash>

mcumgr --conntype serial --connstring "dev=COM5,baud=115200" reset

# After reboot, once the new image is confirmed running correctly:
mcumgr --conntype serial --connstring "dev=COM5,baud=115200" image confirm
```

Same session over the USB transport — only the port changes (Windows: the `COM` port assigned to the CDC ACM device; Linux/macOS: e.g. `/dev/ttyACM0`). The baud rate is required by the CLI syntax but ignored by CDC ACM:

```bash
mcumgr --conntype serial --connstring "dev=COM7,baud=115200" image list
mcumgr --conntype serial --connstring "dev=COM7,baud=115200" image upload app_signed.bin -n 2
mcumgr --conntype serial --connstring "dev=COM7,baud=115200" image test <image-hash>
mcumgr --conntype serial --connstring "dev=COM7,baud=115200" reset
mcumgr --conntype serial --connstring "dev=COM7,baud=115200" image confirm
```
