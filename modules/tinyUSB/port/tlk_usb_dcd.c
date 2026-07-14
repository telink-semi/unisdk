#include "core/include/tlk_analog.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_plic.h"
#include "device/dcd.h"
#include "properties/tlk_plic.h"
#include "registers/tlk_gpio.h"
#include "registers/tlk_usb.h"

#include "core/include/tlk_usb.h"

#define SETUP_LEN 8
#define RHPORT    0

#define USB_IN_EDP_IRQ_BITS                                                                        \
    (TLK_USB_EDP1_IRQ | TLK_USB_EDP2_IRQ | TLK_USB_EDP3_IRQ | TLK_USB_EDP4_IRQ |                   \
     TLK_USB_EDP7_IRQ | TLK_USB_EDP8_IRQ)
#define USB_OUT_EDP_IRQ_BITS (TLK_USB_EDP5_IRQ | TLK_USB_EDP6_IRQ)

/**************************************************************
 *   Driver data structures
 ***************************************************************/

struct tlk_usb_dc_ep_data
{
    uint8_t* buffer;
    uint16_t bytes_to_transfer;
    uint16_t transferred_bytes;
};

struct tlk_usb_dc_data
{
    bool suspend;
    bool suspend_ignore;
    union
    {
        uint8_t                buffer[SETUP_LEN];
        tusb_control_request_t packet;
    } setup_data;
    struct tlk_usb_dc_ep_data ep_data[9];
};

struct tlk_usb_dc_data tlk_usb_ctx = {0};

/**************************************************************
 *   Driver's internal functions
 ***************************************************************/

static struct tlk_usb_dc_data* get_ctx(void)
{
    return &tlk_usb_ctx;
}

static struct tlk_usb_dc_ep_data* get_ep_ctx(enum tlk_usb_ep_idx ep_id)
{
    return &(tlk_usb_ctx.ep_data[ep_id]);
}

static void tlk_usb_dc_setup_cb_handler(void)
{
    struct tlk_usb_dc_data* ctx = get_ctx();

    uint16_t result = tlk_usb_ctrl_ep_read(ctx->setup_data.buffer, SETUP_LEN);
    if (result)
    {
        if (ctx->suspend)
        {
            ctx->suspend        = false;
            ctx->suspend_ignore = true;
            tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_USB_PWDN);
            dcd_event_bus_signal(RHPORT, DCD_EVENT_RESUME, true);
        }
        dcd_event_setup_received(RHPORT, ctx->setup_data.buffer, true);
    }
}

static void tlk_usb_dc_data_cb_handler(void)
{
    struct tlk_usb_dc_data*    ctx    = get_ctx();
    struct tlk_usb_dc_ep_data* ep_ctx = get_ep_ctx(TLK_USB_EP0_IDX);
    uint8_t ep_addr = TLK_USB_EP0_IDX | (ctx->setup_data.packet.bmRequestType_bit.direction << 7);

    if (ctx->setup_data.packet.bmRequestType_bit.direction == TUSB_DIR_OUT)
    {
        ep_ctx->transferred_bytes = tlk_usb_ctrl_ep_read(ep_ctx->buffer, ep_ctx->bytes_to_transfer);
    }

    dcd_event_xfer_complete(RHPORT, ep_addr, ep_ctx->transferred_bytes, XFER_RESULT_SUCCESS, true);
    memset(ep_ctx, 0, sizeof(struct tlk_usb_dc_ep_data));
}

static void tlk_usb_dc_handle_out_eps(uint8_t eps_irq)
{
    enum tlk_usb_ep_idx ep_id = 0;

    if (eps_irq & TLK_USB_EDP5_IRQ)
    {
        ep_id = TLK_USB_OUT_EP5_IDX;
    }
    else if (eps_irq & TLK_USB_EDP6_IRQ)
    {
        ep_id = TLK_USB_OUT_EP6_IDX;
    }
    else
    {
        return;
    }

    struct tlk_usb_dc_ep_data* ep_ctx = get_ep_ctx(ep_id);
    ep_ctx->transferred_bytes = tlk_usb_ep_read(ep_id, ep_ctx->buffer, ep_ctx->bytes_to_transfer);
    dcd_event_xfer_complete(RHPORT, ep_id, ep_ctx->transferred_bytes, XFER_RESULT_SUCCESS, true);
    memset(ep_ctx, 0, sizeof(struct tlk_usb_dc_ep_data));
}

static void tlk_usb_dc_handle_in_eps(uint8_t eps_irq)
{
    uint8_t ep_id = 0;

    if (eps_irq & TLK_USB_EDP1_IRQ)
    {
        ep_id = TLK_USB_IN_EP1_IDX;
    }
    if (eps_irq & TLK_USB_EDP2_IRQ)
    {
        ep_id = TLK_USB_IN_EP2_IDX;
    }
    if (eps_irq & TLK_USB_EDP3_IRQ)
    {
        ep_id = TLK_USB_IN_EP3_IDX;
    }
    if (eps_irq & TLK_USB_EDP4_IRQ)
    {
        ep_id = TLK_USB_IN_EP4_IDX;
    }
    if (eps_irq & TLK_USB_EDP7_IRQ)
    {
        ep_id = TLK_USB_IN_EP7_IDX;
    }
    if (eps_irq & TLK_USB_EDP8_IRQ)
    {
        ep_id = TLK_USB_IN_EP8_IDX;
    }

    tlk_usb_reset_ep_ptr(ep_id);
}

static void tlk_usb_dc_eos_cb_handler(void)
{
    uint8_t irq_eps = tlk_usb_ep_irq_get();
    tlk_usb_ep_irq_clear(irq_eps);

    uint8_t in_ep_irqs  = irq_eps & USB_IN_EDP_IRQ_BITS;
    uint8_t out_ep_irqs = irq_eps & USB_OUT_EDP_IRQ_BITS;
    if (in_ep_irqs)
    {
        tlk_usb_dc_handle_in_eps(in_ep_irqs);
    }
    else if (out_ep_irqs)
    {
        tlk_usb_dc_handle_out_eps(out_ep_irqs);
    }
}

static void tlk_usb_dc_suspend_cb_handler(void)
{
    struct tlk_usb_dc_data* ctx = get_ctx();
    if (ctx->suspend_ignore)
    {
        ctx->suspend_ignore = false;
        return;
    }
    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_USB_PWDN);
    if (!ctx->suspend)
    {
        ctx->suspend = true;
        dcd_event_bus_signal(RHPORT, DCD_EVENT_RESUME, true);
        if (!(tlk_usb_is_wakeup_en()))
        {
            dcd_event_bus_signal(RHPORT, DCD_EVENT_UNPLUGGED, true);
        }
    }
}

static void tlk_usb_dc_reset_cb_handler(void)
{
    struct tlk_usb_dc_data* ctx = get_ctx();
    uint32_t                i;

    for (i = 1; i < 8; i++)
    {
        tlk_usb_reset_ep(i);
    }

    dcd_event_bus_reset(RHPORT, TUSB_SPEED_FULL, true);

    if (ctx->suspend)
    {
        ctx->suspend        = false;
        ctx->suspend_ignore = true;
        tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_USB_PWDN);
        dcd_event_bus_signal(RHPORT, DCD_EVENT_RESUME, true);
    }
}

static void tlk_usb_dc_sof_cb_handler(uint32_t frame_num)
{
    dcd_event_sof(RHPORT, frame_num, true);
}

/**************************************************************
 *   TinyUSB DCD interface implementation
 ***************************************************************/

bool dcd_dcache_clean(const void* addr, uint32_t data_size)
{
    (void) addr;
    (void) data_size;
    return true;
}

bool dcd_dcache_invalidate(const void* addr, uint32_t data_size)
{
    (void) addr;
    (void) data_size;
    return true;
}

bool dcd_dcache_clean_invalidate(const void* addr, uint32_t data_size)
{
    (void) addr;
    (void) data_size;
    return true;
}

bool dcd_init(uint8_t rhport, const tusb_rhport_init_t* rh_init)
{
    (void) rhport;
    if (rh_init->role != TUSB_ROLE_DEVICE)
    {
        return false;
    }

    tlk_usb_init();

    dcd_int_enable(rhport);
    dcd_connect(rhport);
    tlk_core_interrupt_enable();

    return true;
}

bool dcd_deinit(uint8_t rhport)
{
    (void) rhport;

    tlk_usb_deinit();
    return true;
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t* buffer, uint16_t total_bytes)
{
    (void) rhport;

    enum tlk_usb_ep_idx        ep_id  = tu_edpt_number(ep_addr);
    tusb_dir_t                 dir    = tu_edpt_dir(ep_addr);
    struct tlk_usb_dc_ep_data* ep_ctx = get_ep_ctx(tu_edpt_number(ep_addr));

    ep_ctx->bytes_to_transfer = total_bytes;

    uint16_t result = 0;

    if (dir == TUSB_DIR_OUT)
    {
        if (ep_id == TLK_USB_EP0_IDX && total_bytes == 0)
        {
            tlk_usb_ctrl_ep_prepare();
            tlk_usb_write_ctrl_ep_ctrl(TLK_USB_EP_STA_ACK);
            dcd_event_xfer_complete(
                rhport, TLK_USB_EP0_IDX, total_bytes, XFER_RESULT_SUCCESS, true);
        }
        else
        {
            ep_ctx->buffer = buffer;
            tlk_usb_write_ctrl_ep_ctrl(TLK_USB_EP_DAT_ACK);
        }
    }
    else
    {
        if (ep_id == TLK_USB_EP0_IDX)
        {
            result = tlk_usb_ctrl_ep_write(buffer, total_bytes);
            if (total_bytes == 0)
            {
                dcd_event_xfer_complete(rhport,
                                        (TUSB_DIR_IN_MASK | TLK_USB_EP0_IDX),
                                        result,
                                        XFER_RESULT_SUCCESS,
                                        false);
            }
        }
        else
        {
            result = tlk_usb_ep_write(ep_id, buffer, total_bytes);
            dcd_event_xfer_complete(
                rhport, (TUSB_DIR_IN_MASK | ep_id), result, XFER_RESULT_SUCCESS, false);
        }
        ep_ctx->transferred_bytes = result;
    }

    return true;
}

void dcd_int_enable(uint8_t rhport)
{
    (void) rhport;

    struct tlk_usb_cb cb = {.setup_cb   = tlk_usb_dc_setup_cb_handler,
                            .data_cb    = tlk_usb_dc_data_cb_handler,
                            .eos_cb     = tlk_usb_dc_eos_cb_handler,
                            .suspend_cb = tlk_usb_dc_suspend_cb_handler,
                            .reset_cb   = tlk_usb_dc_reset_cb_handler,
                            .sof_cb     = tlk_usb_dc_sof_cb_handler};
    tlk_usb_set_cb(cb);

    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_USB_SETUP);
    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_USB_DATA);
    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_USB_EDP);
    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_USB_PWDN);
    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_USB_RESET);

    tlk_usb_enable_manual_interrupt(TLK_USB_CTRL_EP_AUTO_CFG | TLK_USB_CTRL_EP_AUTO_DESC |
                                    TLK_USB_CTRL_EP_AUTO_FEAT | TLK_USB_CTRL_EP_AUTO_STD);

    tlk_usb_clr_irq_status(USB_IRQ_RESET_STATUS);
}

void dcd_int_disable(uint8_t rhport)
{
    (void) rhport;
    tlk_plic_interrupt_disable(UNISDK_PLIC_IRQ_NUM_USB_SETUP);
    tlk_plic_interrupt_disable(UNISDK_PLIC_IRQ_NUM_USB_DATA);
    tlk_plic_interrupt_disable(UNISDK_PLIC_IRQ_NUM_USB_EDP);
    tlk_plic_interrupt_disable(UNISDK_PLIC_IRQ_NUM_USB_PWDN);
    tlk_plic_interrupt_disable(UNISDK_PLIC_IRQ_NUM_USB_RESET);
}

void dcd_set_address(uint8_t rhport, uint8_t dev_addr)
{
    (void) rhport;
    (void) dev_addr;
}

void dcd_remote_wakeup(uint8_t rhport)
{
    (void) rhport;
    tlk_usb_wakeup();
}

void dcd_connect(uint8_t rhport)
{
    (void) rhport;

    tlk_usb_pull_up(1);
}

void dcd_disconnect(uint8_t rhport)
{
    (void) rhport;

    tlk_usb_pull_up(0);
}

void dcd_sof_enable(uint8_t rhport, bool en)
{
    (void) rhport;
    (void) en;
}

void dcd_edpt0_status_complete(uint8_t rhport, tusb_control_request_t const* request)
{
    (void) rhport;
    (void) request;
}

bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const* desc_ep)
{
    (void) rhport;
    enum tlk_usb_ep_idx ep_id = tu_edpt_number(desc_ep->bEndpointAddress);

    tlk_usb_set_eps_en(TLK_BIT(ep_id));
    if ((desc_ep->bmAttributes.xfer == TUSB_XFER_BULK) &&
        (tu_edpt_dir(desc_ep->bEndpointAddress) == TUSB_DIR_OUT))
    {
        tlk_usb_data_ep_ack(ep_id);
    }

    return true;
}

void dcd_edpt_close_all(uint8_t rhport)
{
    (void) rhport;
    tlk_usb_set_eps_dis(TLK_USB_ALL_EN);
}

void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr)
{
    (void) rhport;
    (void) ep_addr;

    tlk_usb_write_ctrl_ep_ctrl(TLK_USB_EP_DAT_STALL);
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr)
{
    (void) rhport;
    (void) ep_addr;

    tlk_usb_write_ctrl_ep_ctrl(TLK_USB_EP_DAT_ACK);
}

#ifdef TUP_DCD_EDPT_CLOSE_API
// Close an endpoint.
void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr)
{
    (void) rhport;

    tlk_usb_set_eps_dis(tu_edpt_number(ep_addr));
}
#endif

bool dcd_edpt_iso_alloc(uint8_t rhport, uint8_t ep_addr, uint16_t largest_packet_size)
{
    (void) rhport;
    (void) ep_addr;
    (void) largest_packet_size;

    return true;
}

bool dcd_edpt_iso_activate(uint8_t rhport, tusb_desc_endpoint_t const* desc_ep)
{
    return dcd_edpt_open(rhport, desc_ep);
}
