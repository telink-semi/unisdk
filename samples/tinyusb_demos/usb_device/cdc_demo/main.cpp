#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_sys.h"
#include "tusb.h"
#ifdef __cplusplus
}
#endif

tusb_desc_device_t const desc_device = {
    .bLength         = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB          = 0x0200,

    // Use Miscellaneous Class (0xEF) to support Interface Association Descriptor (IAD)
    .bDeviceClass    = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor  = CONFIG_TLK_USB_VENDOR_ID,
    .idProduct = CONFIG_TLK_USB_PRODUCT_ID,
    .bcdDevice = CONFIG_TLK_USB_DEVICE_VERSION,

    .iManufacturer = 0x01,
    .iProduct      = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01};

#ifdef __cplusplus
extern "C"
{
#endif
    uint8_t const* tud_descriptor_device_cb(void)
    {
        return (uint8_t const*) &desc_device;
    }
#ifdef __cplusplus
}
#endif

// --------------------------------------------------------------------+
// Configuration Descriptor
// --------------------------------------------------------------------+

// Define Endpoint Numbers
#define EPNUM_CDC_NOTIF CONFIG_TLK_USB_CDC_NOTIF_EP
#define EPNUM_CDC_OUT   CONFIG_TLK_USB_CDC_OUT_EP
#define EPNUM_CDC_IN    CONFIG_TLK_USB_CDC_IN_EP

// Define Interface Indices
enum
{
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attributes, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in)
    // and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

#ifdef __cplusplus
extern "C"
{
#endif
    uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
    {
        (void) index; // ignore index
        return desc_configuration;
    }
#ifdef __cplusplus
}
#endif

// --------------------------------------------------------------------+
// String Descriptors
// --------------------------------------------------------------------+
char const* string_desc_arr[] = {
    (const char[]) {0x09, 0x04},                      // 0: Supported Language is English (0x0409)
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_MANUFACTURER),   // 1: Manufacturer
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_PRODUCT_NAME),   // 2: Product
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_SERIAL_NUMBER),  // 3: Serials
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_INTERFACE_NAME), // 4: Interface Name
};

static uint16_t _desc_str[32];

#ifdef __cplusplus
extern "C"
{
#endif
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
#ifdef __cplusplus
}
#endif

void cdc_task(void);

//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+
int main(void)
{
    tud_init(BOARD_TUD_RHPORT); // Initialize USB Device Stack

    while (1)
    {
        tud_task(); // Device task: Handle USB events (MUST be called frequently)
        cdc_task(); // Application task: Handle Serial data
    }

    return 0;
}

//--------------------------------------------------------------------+
// CDC Application Task
//--------------------------------------------------------------------+
void cdc_task(void)
{
    // connected() check is optional, but efficient
    if (tud_cdc_connected())
    {
        // Check for available bytes in the RX FIFO
        if (tud_cdc_available())
        {
            uint8_t buf[64];

            // Read data from Host
            uint32_t count = tud_cdc_read(buf, sizeof(buf));

            // Echo data back to Host
            tud_cdc_write(buf, count);
            tud_cdc_write_flush(); // Force sending the packet immediately
        }
    }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

#ifdef __cplusplus
extern "C"
{
#endif

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

#ifdef __cplusplus
}
#endif
