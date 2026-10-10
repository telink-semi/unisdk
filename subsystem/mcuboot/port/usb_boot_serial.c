#include "include/usb_boot_serial.h"

#include "common/include/tlk_config_macro.h"

#include "boot_serial/boot_serial.h"
#include "tusb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define BOOT_SERIAL_RX_BUF_SIZE 256

static uint8_t  tlk_usb_rx_buf[BOOT_SERIAL_RX_BUF_SIZE];
static uint32_t tlk_usb_rx_count;
static uint32_t tlk_usb_rx_pos;

/* ------------------------------------------------------------------------
 * USB device descriptors
 * ---------------------------------------------------------------------- */

static tusb_desc_device_t const tlk_desc_device = {
    .bLength         = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB          = 0x0200,

    /* Class 0xEF/0x02/0x01 (IAD) so CDC enumerates cleanly on Windows/Linux. */
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

    .bNumConfigurations = 0x01,
};

uint8_t const* tud_descriptor_device_cb(void)
{
    return (uint8_t const*) &tlk_desc_device;
}

#define EPNUM_CDC_NOTIF CONFIG_TLK_USB_CDC_NOTIF_EP
#define EPNUM_CDC_OUT   CONFIG_TLK_USB_CDC_OUT_EP
#define EPNUM_CDC_IN    CONFIG_TLK_USB_CDC_IN_EP

enum
{
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

static uint8_t const tlk_desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index;
    return tlk_desc_configuration;
}

static char const* tlk_string_desc_arr[] = {
    (const char[]){0x09, 0x04},
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_MANUFACTURER),
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_PRODUCT_NAME),
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_SERIAL_NUMBER),
    TLK_TO_STR(CONFIG_TLK_USB_DEVICE_INTERFACE_NAME),
};

static uint16_t tlk_desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;
    uint8_t chr_count;

    if (index == 0)
    {
        memcpy(&tlk_desc_str[1], tlk_string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        if (!(index < sizeof(tlk_string_desc_arr) / sizeof(tlk_string_desc_arr[0])))
        {
            return NULL;
        }

        const char* str = tlk_string_desc_arr[index];
        chr_count       = (uint8_t) strlen(str);

        if (chr_count > sizeof(tlk_desc_str) / sizeof(tlk_desc_str[0]))
        {
            return NULL;
        }

        for (uint8_t i = 0; i < chr_count; i++)
        {
            tlk_desc_str[1 + i] = (uint16_t) str[i];
        }
    }

    tlk_desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return tlk_desc_str;
}

/* ------------------------------------------------------------------------
 * TinyUSB device callbacks
 * ---------------------------------------------------------------------- */

void tud_mount_cb(void) {}
void tud_umount_cb(void) {}
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
}
void tud_resume_cb(void) {}

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

/* ------------------------------------------------------------------------
 * boot_uart_funcs backend
 * ---------------------------------------------------------------------- */

static void tlk_usb_rx_fill(void)
{
    tlk_usb_rx_count = tud_cdc_read(tlk_usb_rx_buf, sizeof(tlk_usb_rx_buf));
    tlk_usb_rx_pos   = 0;
}

static int tlk_boot_usb_read(char* str, int cnt, int* newline)
{
    int off = 0;

    *newline = 0;

    tud_task();

    if (tlk_usb_rx_pos >= tlk_usb_rx_count)
    {
        if (!tud_cdc_available())
        {
            return 0;
        }

        tlk_usb_rx_fill();
    }

    while ((off < cnt) && (tlk_usb_rx_pos < tlk_usb_rx_count))
    {
        uint8_t byte = tlk_usb_rx_buf[tlk_usb_rx_pos++];

        if (byte == '\n')
        {
            str[off++] = '\0';
            *newline   = 1;
            break;
        }

        str[off++] = (char) byte;
    }

    return off;
}

#define BOOT_SERIAL_TX_BUF_SIZE 256

static uint8_t  tlk_usb_tx_buf[BOOT_SERIAL_TX_BUF_SIZE];
static uint32_t tlk_usb_tx_len;

static void tlk_usb_tx_flush_now(void)
{
    const uint8_t* buf       = tlk_usb_tx_buf;
    uint32_t       remaining = tlk_usb_tx_len;
    uint32_t       spins     = 0;

    while (remaining > 0)
    {
        tud_task();

        uint32_t avail = tud_cdc_write_available();

        if (avail == 0)
        {
            spins++;
            continue;
        }

        uint32_t chunk   = (remaining < avail) ? remaining : avail;
        uint32_t written = tud_cdc_write(buf, chunk);

        buf += written;
        remaining -= written;

        tud_cdc_write_flush();
    }

    (void) spins;
    tlk_usb_tx_len = 0;
}

static void tlk_boot_usb_write(const char* ptr, int cnt)
{
    const uint8_t* buf = (const uint8_t*) ptr;

    for (int i = 0; i < cnt; i++)
    {
        if (tlk_usb_tx_len >= sizeof(tlk_usb_tx_buf))
        {
            tlk_usb_tx_flush_now();
        }

        tlk_usb_tx_buf[tlk_usb_tx_len++] = buf[i];

        if (buf[i] == '\n')
        {
            tlk_usb_tx_flush_now();
        }
    }
}

static const struct boot_uart_funcs tlk_usb_funcs = {
    .read  = tlk_boot_usb_read,
    .write = tlk_boot_usb_write,
};

const struct boot_uart_funcs* tlk_mcuboot_serial_usb_init(void)
{
    tlk_usb_rx_count = 0;
    tlk_usb_rx_pos   = 0;

    tud_init(0);

    return &tlk_usb_funcs;
}
