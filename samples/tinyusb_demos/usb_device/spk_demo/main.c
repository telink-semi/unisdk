#include "class/audio/audio.h"
#include "core/include/tlk_audio.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_dma.h"
#include "core/include/tlk_gpio.h"
#include "device/usbd.h"
#include "system/debug/log/tlk_log.h"
#include "tusb.h"

TLK_LOG_CREATE(usb_spk_demo, "USB_SPEAKER_DEMO");

// Interface / entity IDs used inside the UAC2 descriptor tree.
enum
{
    ITF_NUM_AUDIO_CONTROL = 0,
    ITF_NUM_AUDIO_STREAMING,
    ITF_NUM_TOTAL
};

// Entity IDs inside the speaker function
enum
{
    UAC2_ENTITY_INPUT_TERMINAL  = 0x01, // USB streaming input (from host)
    UAC2_ENTITY_FEATURE_UNIT    = 0x02, // volume/mute control
    UAC2_ENTITY_OUTPUT_TERMINAL = 0x03, // physical speaker output terminal
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

// UAC2 feedback endpoint payload is always 4 bytes (16.16 fixed point),
// independent of speed, per USB Audio 2.0 spec 3.16.2.2.
#define AUDIO_FB_EP_SIZE 4

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + CFG_TUD_AUDIO_FUNC_1_DESC_LEN)

uint8_t const desc_configuration[] = {
    // Config: total length, number of interfaces, string index, total len, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0, 100),

    // TUD_AUDIO20_SPEAKER_MONO_FB_DESCRIPTOR lays out a complete UAC2
    // speaker function: IAD, Audio Control interface (clock source, USB
    // streaming input terminal, feature unit, physical speaker output
    // terminal) plus one Audio Streaming interface with alt-0
    // (zero-bandwidth) and alt-1 (active OUT + feedback IN endpoints).
    //
    // Args: interface control number, string index, bytes/bits per
    //       sample, EP out address + size, EP feedback address + size
    TUD_AUDIO20_SPEAKER_MONO_FB_DESCRIPTOR(
        ITF_NUM_AUDIO_CONTROL, 0, CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE_RX,
        CONFIG_TLK_USB_AUDIO_SPK_BITS_PER_SAMPLE, CONFIG_TLK_USB_AUDIO_SPK_OUT_EP,
        CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX, CONFIG_TLK_USB_AUDIO_SPK_FB_IN_EP, AUDIO_FB_EP_SIZE)};

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
// "host is about to send audio" (playback started), alt 0 means idle/closed.
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
            tlk_audio_output_enable();
        }
        else
        {
            tlk_audio_output_disable();
        }
    }

    return true;
}

//--------------------------------------------------------------------
// Mocked Feature Unit / Clock Source state
//--------------------------------------------------------------------

static bool          spk_mute_state = false;
static int16_t       spk_volume_cur = 0; // 0 dB, in 1/256 dB units (UAC2 fixed-point)
static const int16_t spk_volume_min = (int16_t) (-90 * 256); // -90 dB
static const int16_t spk_volume_max = 0;                     //   0 dB
static const int16_t spk_volume_res = 256;                   //   1 dB steps

// Host GET requests targeting an entity (Feature Unit or Clock Source).
// Windows' UAC2 class driver queries these to build its mixer/format UI.
//
// Note: unlike the mic example, the Input Terminal (entity 1) and Output
// Terminal (entity 3) here both declare _ctrl = 0 in the descriptor (no
// controls), so we don't implement a Terminal Connector responder for
// them - if a host queries one anyway, correctly stalling (returning
// false) is the spec-compliant response for an undeclared control.
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const* p_request)
{
    uint8_t const entity_id = tu_u16_high(p_request->wIndex);
    uint8_t const ctrl_sel  = tu_u16_high(p_request->wValue);

    if (entity_id == UAC2_ENTITY_FEATURE_UNIT)
    {
        if (ctrl_sel == AUDIO20_FU_CTRL_MUTE && p_request->bRequest == AUDIO20_CS_REQ_CUR)
        {
            uint8_t mute = spk_mute_state ? 1 : 0;
            return tud_audio_buffer_and_schedule_control_xfer(
                rhport, p_request, &mute, sizeof(mute));
        }

        if (ctrl_sel == AUDIO20_FU_CTRL_VOLUME)
        {
            if (p_request->bRequest == AUDIO20_CS_REQ_CUR)
            {
                return tud_audio_buffer_and_schedule_control_xfer(
                    rhport, p_request, &spk_volume_cur, sizeof(spk_volume_cur));
            }
            if (p_request->bRequest == AUDIO20_CS_REQ_RANGE)
            {
                // UAC2 RANGE parameter block, 1 subrange: wNumSubRanges, then
                // {MIN, MAX, RES} - each field little-endian 16-bit here.
                uint8_t        buf[2 + 2 + 2 + 2];
                uint16_t const num_sub = 1;
                memcpy(&buf[0], &num_sub, 2);
                memcpy(&buf[2], &spk_volume_min, 2);
                memcpy(&buf[4], &spk_volume_max, 2);
                memcpy(&buf[6], &spk_volume_res, 2);
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
            spk_mute_state = (pBuff[0] != 0);
            return true;
        }
        if (ctrl_sel == AUDIO20_FU_CTRL_VOLUME)
        {
            memcpy(&spk_volume_cur, pBuff, sizeof(spk_volume_cur));
            return true;
        }
    }

    // Our clock source is fixed (AUDIO20_CLOCK_SOURCE_ATT_INT_FIX_CLK in
    // the descriptor), so it has no writable controls to accept here.
    return false;
}

//--------------------------------------------------------------------
// Feedback endpoint
//--------------------------------------------------------------------

#if CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP

// Reports our sample clock back to the host so it can pace how much audio it sends.
void tud_audio_feedback_params_cb(uint8_t func_id, uint8_t alt_itf,
                                  audio_feedback_params_t* feedback_param)
{
    (void) func_id;
    (void) alt_itf;

    feedback_param->method                    = AUDIO_FEEDBACK_METHOD_FIFO_COUNT;
    feedback_param->sample_freq               = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
    feedback_param->fifo_count.fifo_threshold = 0; // 0 -> default to half-FIFO
}

#endif

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

static void fill_buffer(void)
{
    tud_audio_read(audio_buffer[audio_buffer_index % 2], BUFFER_SIZE_BYTES);

    audio_buffer_index++;
}

static void on_audio_tx_done(enum tlk_dma_irq_type irq)
{
    if (irq != TLK_DMA_IRQ_TC)
    {
        TLK_LOG_ERROR(usb_spk_demo, "TX DMA IRQ status: %d", irq);
        return;
    }

    fill_buffer();
}

static void audio_init(void)
{
// TODO: refactor this
#if TLK_IS_ENABLED(CONFIG_TLK_CORE_TL321X)
    struct tlk_audio_sdm_pinmux sdm_pinmux = {
        .sdm0_p = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_5},
        .sdm0_n = {TLK_GPIO_PORT_E, TLK_GPIO_PIN_2},
        .sdm1_p = {TLK_GPIO_PORT_E, TLK_GPIO_PIN_3},
        .sdm1_n = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_0},
    };
#elif TLK_IS_ENABLED(CONFIG_TLK_CORE_TL721X)
    struct tlk_audio_sdm_pinmux sdm_pinmux = {
        .sdm0_p = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_0},
        .sdm0_n = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_1},
        .sdm1_p = {TLK_GPIO_PORT_F, TLK_GPIO_PIN_4},
        .sdm1_n = {TLK_GPIO_PORT_F, TLK_GPIO_PIN_5},
    };
#elif TLK_IS_ENABLED(CONFIG_TLK_CORE_B92)
    struct tlk_audio_sdm_pinmux sdm_pinmux = {};
#endif

    struct tlk_dma_ll_node tx_buffer0 = {
        .length   = BUFFER_SIZE_BYTES,
        .src_addr = audio_buffer[0],
    };

    struct tlk_dma_ll_node tx_buffer1 = {
        .next     = &tx_buffer0,
        .length   = BUFFER_SIZE_BYTES,
        .src_addr = audio_buffer[1],
    };

    tx_buffer0.next = &tx_buffer1;

    struct tlk_audio_output tx_config = {
        .source        = TLK_AUDIO_OUTPUT_LINE,
        .sample_rate   = TLK_AUDIO_48K,
        .audio_channel = TLK_I2S_CHANNEL_LEFT,
        .data_width    = TLK_I2S_DATA_WIDTH_16_BIT,
        .dma_channel   = tlk_dma_chn_request(),
        .fifo          = TLK_I2S_FIFO0,
        .ll_head       = &tx_buffer0,
    };

    tlk_core_interrupt_enable();

    tlk_audio_init();

    tlk_audio_sdm_pinmux_configure(&sdm_pinmux);

    tlk_dma_chn_ll_set_irq_mode(tx_config.dma_channel, TLK_DMA_LL_IRQ_INTERRUPT_MODE);
    tlk_dma_chn_add_callback(tx_config.dma_channel, on_audio_tx_done);

    tlk_audio_output_configure(&tx_config);
}

static void audio_task(void) {}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

void tud_mount_cb(void)
{
    TLK_LOG_INFO(usb_spk_demo, "Mounted.");
}

void tud_umount_cb(void)
{
    TLK_LOG_INFO(usb_spk_demo, "Unmounted.");
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
}

void tud_resume_cb(void) {}
