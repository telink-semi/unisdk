#include "tusb.h"

#include "core/include/tlk_gpio.h"
#include "core/include/tlk_sys.h"

tusb_desc_device_t const desc_device = {
    .bLength         = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB          = 0x0200,

    // Use Miscellaneous Class (0xEF) to support Interface Association Descriptor (IAD)
    // This is best practice for CDC to ensure drivers load correctly on Windows/Linux
    .bDeviceClass    = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor  = CONFIG_TLK_USB_VENDOR_ID,  // Example Vendor ID
    .idProduct = CONFIG_TLK_USB_PRODUCT_ID, // Example Product ID
    .bcdDevice = CONFIG_TLK_USB_DEVICE_VERSION,

    .iManufacturer = 0x01,
    .iProduct      = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01};

uint8_t const* tud_descriptor_device_cb(void)
{
    return (uint8_t const*) &desc_device;
}

// --------------------------------------------------------------------+
// Configuration Descriptor
// --------------------------------------------------------------------+

// Define Endpoint Numbers
#define EPNUM_CDC_NOTIF CONFIG_TLK_USB_CDC_NOTIF_EP
#define EPNUM_CDC_OUT   CONFIG_TLK_USB_CDC_OUT_EP
#define EPNUM_CDC_IN    CONFIG_TLK_USB_CDC_IN_EP

// Define Interface Indices (Fixes your "invalid interface number" error)
enum
{
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL // EQUALS 2. This fixes "descriptor's value: 1" error.
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attributes, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in)
    // and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index; // ignore index
    return desc_configuration;
}

// --------------------------------------------------------------------+
// String Descriptors
// --------------------------------------------------------------------+
char const* string_desc_arr[] = {
    (const char[]){0x09, 0x04},                       // 0: Supported Language is English (0x0409)
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_MANUFACTURER),   // 1: Manufacturer
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_PRODUCT_NAME),   // 2: Product
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_SERIAL_NUMBER),  // 3: Serials
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_INTERFACE_NAME), // 4: Interface Name
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;
    uint8_t chr_count;

    if (index == 0)
    {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
            return NULL;

        const char* str = string_desc_arr[index];
        chr_count       = strlen(str);

        if (chr_count > sizeof(_desc_str) / sizeof(_desc_str[0]))
            return NULL;

        // Convert ASCII string into UTF-16
        for (uint8_t i = 0; i < chr_count; i++)
            _desc_str[1 + i] = str[i];
    }

    // first byte is length, second byte is descriptor type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}

#ifndef TLK_LOG_USB_DROP_IF_FULL
#define TLK_LOG_USB_DROP_IF_FULL 0
#endif

#ifndef TLK_LOG_USB_MAX_SPINS
#define TLK_LOG_USB_MAX_SPINS 4
#endif

/**
 * Print an already-formatted buffer over TinyUSB CDC.
 *
 * Returns number of bytes accepted into TinyUSB's CDC TX FIFO.
 *
 * Notes:
 * - Best called from task/thread context, not from hard ISR context.
 * - Requires tud_task() to be called regularly elsewhere in your main loop
 *   or USB task.
 * - Non-blocking by default: if the CDC TX FIFO is full, remaining bytes
 *   are dropped.
 */
uint16_t tlk_log_usb_print_buffer(const uint8_t* buf, uint16_t len)
{
    if (buf == NULL || len == 0)
    {
        return 0;
    }

    /*
     * tud_mounted() means the USB device is configured by the host.
     * tud_cdc_connected() means CDC DTR/RTS line state indicates an open
     * terminal on many hosts. Depending on your use case, you may remove
     * tud_cdc_connected() if you want to queue even before a terminal opens.
     */
    if (!tud_mounted())
    {
        return 0;
    }

    uint16_t total_written = 0;
    uint8_t  spins         = 0;

    while (total_written < len)
    {
        uint32_t avail = tud_cdc_write_available();

        if (avail == 0)
        {
            /*
             * Try to push whatever is already queued. This can free FIFO
             * space once the USB endpoint transfer progresses.
             */
            tud_cdc_write_flush();

#if TLK_LOG_USB_DROP_IF_FULL
            break;
#else
            /*
             * Bounded wait. Avoid an infinite loop if the host stops polling,
             * the cable is unplugged, or CDC becomes disconnected.
             */
            if (++spins >= TLK_LOG_USB_MAX_SPINS)
            {
                break;
            }

            tud_task();

            if (!tud_mounted() || !tud_cdc_connected())
            {
                break;
            }

            continue;
#endif
        }

        uint32_t remaining = (uint32_t) (len - total_written);
        uint32_t chunk     = (remaining < avail) ? remaining : avail;

        uint32_t written = tud_cdc_write(buf + total_written, chunk);

        if (written == 0)
        {
            tud_cdc_write_flush();

#if TLK_LOG_USB_DROP_IF_FULL
            break;
#else
            if (++spins >= TLK_LOG_USB_MAX_SPINS)
            {
                break;
            }

            tud_task();
            continue;
#endif
        }

        total_written += (uint16_t) written;
        spins = 0;
    }

    if (total_written > 0)
    {
        tud_cdc_write_flush();
    }

    return total_written;
}

void tlk_log_usb_cdc_init(void)
{
    tud_init(BOARD_TUD_RHPORT);
}

void tlk_log_usb_run(void)
{
    tud_task();
}

// Invoked when device is mounted (configured)
void tud_mount_cb(void) {}

// Invoked when device is unmounted
void tud_umount_cb(void) {}

// Invoked when USB bus is suspended
// remote_wakeup_en: if host allows us to perform remote wakeup
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
}

// Invoked when USB bus is resumed
void tud_resume_cb(void) {}

// Invoked when CDC line coding is changed (baud rate, parity, etc.)
void tud_cdc_line_coding_cb(uint8_t itf, const cdc_line_coding_t* p_line_coding)
{
    (void) itf;
    (void) p_line_coding;
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void) itf;
    (void) dtr;
    (void) rts;
}
