#include "class/audio/audio.h"
#include "core/include/tlk_audio.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_gpio.h"
#include "device/usbd.h"
#include "system/debug/log/tlk_log.h"
#include "tusb.h"

TLK_LOG_CREATE(usb_mic_demo, "USB_MIC_DEMO");

// Interface / entity IDs used inside the UAC2 descriptor tree.
enum
{
    ITF_NUM_AUDIO_CONTROL = 0,
    ITF_NUM_AUDIO_STREAMING,
    ITF_NUM_TOTAL
};

// Entity IDs inside the mic function
enum
{
    UAC2_ENTITY_INPUT_TERMINAL  = 0x01, // microphone input terminal
    UAC2_ENTITY_FEATURE_UNIT    = 0x02, // volume/mute control
    UAC2_ENTITY_OUTPUT_TERMINAL = 0x03, // USB streaming output terminal
    UAC2_ENTITY_CLOCK           = 0x04, // sampling frequency control
};

//--------------------------------------------------------------------
// Device Descriptor
//--------------------------------------------------------------------

tusb_desc_device_t const desc_device = {
    .bLength         = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB          = 0x0200,

    // Use the UAC2 IAD-friendly class/subclass/protocol at the device level
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

uint8_t const* tud_descriptor_device_cb(void)
{
    return (uint8_t const*) &desc_device;
}

//--------------------------------------------------------------------
// Configuration Descriptor
//--------------------------------------------------------------------

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_AUDIO_FUNC_1_DESC_LEN)

uint8_t const desc_configuration[] = {
    // Config: total length, number of interfaces, string index, total len, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0, 100),

    // TUD_AUDIO20_MIC_ONE_CH_DESCRIPTOR (UAC2 variant - tinyusb split the old
    // TUD_AUDIO_MIC_ONE_CH_DESCRIPTOR into TUD_AUDIO10_* and TUD_AUDIO20_* families) lays out a
    // complete UAC2 function: IAD, Audio Control interface (clock source, input terminal, feature
    // unit, output terminal) plus one Audio Streaming interface with alt-0 (zero-bandwidth) and
    // alt-1 (active, isochronous IN endpoint) - exactly what a single-channel mic needs.
    //
    // Args: interface control number, string index, bytes and bits per sample, EP in, EP size
    TUD_AUDIO20_MIC_ONE_CH_DESCRIPTOR(
        ITF_NUM_AUDIO_CONTROL, 0, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_TX,
        CONFIG_TLK_USB_AUDIO_MIC_BITS_PER_SAMPLE, CONFIG_TLK_USB_AUDIO_MIC_IN_EP,
        CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX)};

uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index;
    return desc_configuration;
}

//--------------------------------------------------------------------
// String Descriptors
//--------------------------------------------------------------------

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
    size_t chr_count;

    if (index == 0)
    {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
            return NULL;

        const char* str        = string_desc_arr[index];
        chr_count              = strlen(str);
        size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
        if (chr_count > max_count)
            chr_count = max_count;

        for (size_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = str[i];
        }
    }

    _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return _desc_str;
}

//--------------------------------------------------------------------
// TinyUSB audio class callbacks
//--------------------------------------------------------------------

bool is_streaming;

// Fired when the host changes the streaming alt-setting - alt 1 means
// "host wants audio now" (it opened the mic / started recording), alt 0 means idle/closed.
bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const* p_request)
{
    (void) rhport;
    uint8_t const itf = tu_u16_low(p_request->wIndex);
    uint8_t const alt = tu_u16_low(p_request->wValue);

    if (itf == ITF_NUM_AUDIO_STREAMING)
    {
        is_streaming = alt != 0;
        tlk_gpio_pin_write(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN, is_streaming);

        if (is_streaming)
        {
            tlk_audio_input_enable();
        }
        else
        {
            tlk_audio_input_disable();
        }
    }

    return true;
}

//--------------------------------------------------------------------
// Mocked Feature Unit / Clock Source state
//--------------------------------------------------------------------

static bool          mic_mute_state = false;
static int16_t       mic_volume_cur = 0; // 0 dB, in 1/256 dB units (UAC2 fixed-point)
static const int16_t mic_volume_min = (int16_t) (-90 * 256); // -90 dB
static const int16_t mic_volume_max = 0;                     //   0 dB
static const int16_t mic_volume_res = 256;                   //   1 dB steps

// Host GET requests targeting an entity (Input Terminal, Feature Unit, or Clock Source).
// Windows' UAC2 class driver queries these to build it mixer/format UI and channel layout.
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const* p_request)
{
    uint8_t const entity_id = tu_u16_high(p_request->wIndex);
    uint8_t const ctrl_sel  = tu_u16_high(p_request->wValue);

    if (entity_id == UAC2_ENTITY_INPUT_TERMINAL)
    {
        if (ctrl_sel == AUDIO20_TE_CTRL_CONNECTOR && p_request->bRequest == AUDIO20_CS_REQ_CUR)
        {
            // Terminal Connector control only has a CUR (no RANGE) - reports
            // this terminal's channel cluster (count/spatial layout/naming).
            audio20_desc_channel_cluster_t ret;
            ret.bNrChannels = CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX;
            ret.bmChannelConfig =
                (audio20_channel_config_t) 0; // no defined spatial position - mono/mock
            ret.iChannelNames = 0;            // no string descriptor for channel names
            return tud_audio_buffer_and_schedule_control_xfer(
                rhport, p_request, (void*) &ret, sizeof(ret));
        }
    }
    else if (entity_id == UAC2_ENTITY_FEATURE_UNIT)
    {
        if (ctrl_sel == AUDIO20_FU_CTRL_MUTE && p_request->bRequest == AUDIO20_CS_REQ_CUR)
        {
            uint8_t mute = mic_mute_state ? 1 : 0;
            return tud_audio_buffer_and_schedule_control_xfer(
                rhport, p_request, &mute, sizeof(mute));
        }

        if (ctrl_sel == AUDIO20_FU_CTRL_VOLUME)
        {
            if (p_request->bRequest == AUDIO20_CS_REQ_CUR)
            {
                return tud_audio_buffer_and_schedule_control_xfer(
                    rhport, p_request, &mic_volume_cur, sizeof(mic_volume_cur));
            }
            if (p_request->bRequest == AUDIO20_CS_REQ_RANGE)
            {
                // UAC2 RANGE parameter block, 1 subrange: wNumSubRanges, then
                // {MIN, MAX, RES} - each field little-endian 16-bit here.
                uint8_t        buf[2 + 2 + 2 + 2];
                uint16_t const num_sub = 1;
                memcpy(&buf[0], &num_sub, 2);
                memcpy(&buf[2], &mic_volume_min, 2);
                memcpy(&buf[4], &mic_volume_max, 2);
                memcpy(&buf[6], &mic_volume_res, 2);
                return tud_audio_buffer_and_schedule_control_xfer(
                    rhport, p_request, buf, sizeof(buf));
            }
        }
    }
    else if (entity_id == UAC2_ENTITY_CLOCK)
    {
        if (ctrl_sel == AUDIO20_CS_CTRL_SAM_FREQ)
        {
            uint32_t freq = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;

            if (p_request->bRequest == AUDIO20_CS_REQ_CUR)
            {
                return tud_audio_buffer_and_schedule_control_xfer(
                    rhport, p_request, (void*) &freq, sizeof(freq));
            }
            if (p_request->bRequest == AUDIO20_CS_REQ_RANGE)
            {
                // Fixed clock -> single subrange collapsed to one point (MIN == MAX, RES == 0)
                uint8_t        buf[2 + 4 + 4 + 4];
                uint16_t const num_sub = 1;
                uint32_t const res     = 0;
                memcpy(&buf[0], &num_sub, 2);
                memcpy(&buf[2], &freq, 4);
                memcpy(&buf[6], &freq, 4);
                memcpy(&buf[10], &res, 4);
                return tud_audio_buffer_and_schedule_control_xfer(
                    rhport, p_request, buf, sizeof(buf));
            }
        }

        if (ctrl_sel == AUDIO20_CS_CTRL_CLK_VALID && p_request->bRequest == AUDIO20_CS_REQ_CUR)
        {
            uint8_t valid = 1; // our mock "clock" is always locked/valid
            return tud_audio_buffer_and_schedule_control_xfer(
                rhport, p_request, &valid, sizeof(valid));
        }
    }

    return false; // stall anything we don't recognize
}

// Host SET requests targeting an entity (host changing mute/volume).
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const* p_request,
                                 uint8_t* pBuff)
{
    (void) rhport;
    uint8_t const entity_id = tu_u16_high(p_request->wIndex);
    uint8_t const ctrl_sel  = tu_u16_high(p_request->wValue);

    if (entity_id == UAC2_ENTITY_FEATURE_UNIT)
    {
        if (ctrl_sel == AUDIO20_FU_CTRL_MUTE)
        {
            mic_mute_state = (pBuff[0] != 0);
            return true;
        }
        if (ctrl_sel == AUDIO20_FU_CTRL_VOLUME)
        {
            memcpy(&mic_volume_cur, pBuff, sizeof(mic_volume_cur));
            return true;
        }
    }

    // Our clock source is fixed (AUDIO_CLOCK_SOURCE_ATT_INT_FIX_CLK in the
    // descriptor), so it has no writable controls to accept here.
    return false;
}

//--------------------------------------------------------------------
// Main
//--------------------------------------------------------------------

static void audio_init(void);
static void audio_task(void);

int main(void)
{
    tlk_gpio_configure(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN, TLK_GPIO_OUTPUT);
    tusb_init();
    audio_init();

    while (1)
    {
        tud_task();
        audio_task();
    }

    return 0;
}

//--------------------------------------------------------------------+
// Audio Application
//--------------------------------------------------------------------+

#define AUDIO_BUFFER_TYPE uint16_t
#define BUFFER_SIZE       48
#define BUFFER_SIZE_BYTES (BUFFER_SIZE * sizeof(AUDIO_BUFFER_TYPE))

AUDIO_BUFFER_TYPE audio_buffer[2][BUFFER_SIZE] __attribute__((aligned(4)));
volatile uint8_t  audio_buffer_index;

static void send_buffer(void)
{
    tud_audio_write(audio_buffer[audio_buffer_index % 2], BUFFER_SIZE_BYTES);

    audio_buffer_index++;
}

static void on_audio_rx_done(enum tlk_dma_irq_type irq)
{
    if (irq != TLK_DMA_IRQ_TC)
    {
        TLK_LOG_ERROR(usb_mic_demo, "RX DMA IRQ status: %d", irq);
        return;
    }

    send_buffer();
}

static void audio_init(void)
{
// TODO: refactor this
#if TLK_IS_ENABLED(CONFIG_TLK_CORE_TL321X)
    struct tlk_audio_amic_pinmux amic_pinmux = {
        .bias = {TLK_GPIO_PORT_B, TLK_GPIO_PIN_4},
    };

    struct tlk_audio_dmic_pinmux dmic_pinmux = {
        .data = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_2},
        .clk1 = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_3},
        .clk2 = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_4},
    };
#elif TLK_IS_ENABLED(CONFIG_TLK_CORE_TL721X)
    struct tlk_audio_amic_pinmux amic_pinmux = {
        .bias = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_1},
    };

    struct tlk_audio_dmic_pinmux dmic_pinmux = {
        .data = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_2},
        .clk1 = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_3},
        .clk2 = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_4},
    };
#elif TLK_IS_ENABLED(CONFIG_TLK_CORE_B92)
    struct tlk_audio_amic_pinmux amic_pinmux = {};

    struct tlk_audio_dmic_pinmux dmic_pinmux = {
        .data = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_5},
        .clk1 = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_4},
        .clk2 = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_3},
    };
#endif

    struct tlk_dma_ll_node rx_buffer0 = {
        .length   = BUFFER_SIZE_BYTES,
        .dst_addr = audio_buffer[0],
    };

    struct tlk_dma_ll_node rx_buffer1 = {
        .next     = &rx_buffer0,
        .length   = BUFFER_SIZE_BYTES,
        .dst_addr = audio_buffer[1],
    };

    rx_buffer0.next = &rx_buffer1;

    struct tlk_audio_input rx_config = {
        .stream        = TLK_AUDIO_INPUT_STREAM0,
        .source        = TLK_AUDIO_INPUT_DMIC,
        .audio_channel = TLK_I2S_CHANNEL_LEFT,
        .sample_rate   = TLK_AUDIO_48K,
        .data_width    = TLK_I2S_DATA_WIDTH_16_BIT,
        .dma_channel   = tlk_dma_chn_request(),
        .fifo          = TLK_I2S_FIFO0,
        .ll_head       = &rx_buffer0,
    };

    tlk_core_interrupt_enable();

    tlk_audio_init();

    tlk_audio_amic_pinmux_configure(&amic_pinmux);
    tlk_audio_dmic_pinmux_configure(rx_config.stream, &dmic_pinmux);

    tlk_dma_chn_ll_set_irq_mode(rx_config.dma_channel, TLK_DMA_LL_IRQ_INTERRUPT_MODE);
    tlk_dma_chn_add_callback(rx_config.dma_channel, on_audio_rx_done);

    tlk_audio_input_configure(&rx_config);
}

static void audio_task(void) {}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

void tud_mount_cb(void)
{
    TLK_LOG_INFO(usb_mic_demo, "Mounted.");
}

void tud_umount_cb(void)
{
    TLK_LOG_INFO(usb_mic_demo, "Unmounted.");
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
}

void tud_resume_cb(void) {}
