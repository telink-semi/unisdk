#include "core/include/tlk_usb.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_plic.h"
#include "properties/tlk_plic.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_clock.h"
#include "registers/tlk_gpio.h"
#include "registers/tlk_usb.h"
#include "system/debug/log/tlk_log.h"

#define CTRL_EP_NORMAL_PACKET_REG_VALUE 0x38

TLK_LOG_CREATE(usb_log, "USB");

struct tlk_usb_cb usb_irq_cb;

/**************************************************************
 *   Internal functions for working with USB HW registers
 ***************************************************************/

static void usb_pin_set_to_input(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    TLK_BM_SET(tlk_gpio_reg.port_config[port].output_en, pin);
    TLK_BM_SET(tlk_gpio_reg.port_config[port].input_en, pin);
}

static void usb_pin_set_en(enum tlk_gpio_port port_dm, enum tlk_gpio_port port_dp,
                           enum tlk_gpio_pin pin_dm, enum tlk_gpio_pin pin_dp)
{
    tlk_gpio_reg.pin_func[port_dm][TLK_BIT_LOW_BIT(pin_dm)] = 0x00; /* DM */
    tlk_gpio_reg.pin_func[port_dp][TLK_BIT_LOW_BIT(pin_dp)] = 0x00; /* DP */
    tlk_gpio_disable(port_dm, pin_dm);
    tlk_gpio_disable(port_dp, pin_dp);
    usb_pin_set_to_input(port_dm, pin_dm);
    usb_pin_set_to_input(port_dp, pin_dp);

    write_reg8(0x100c01, (read_reg8(0x100c01) | TLK_BIT(7))); // swire_usb_en
}

_tlk_attribute_ram_code_sec_ static void tlk_usb_setup_irq_handle(void)
{
    tlk_usb_irq_clr_status(USB_IRQ_EP_SETUP_STATUS);
    if (usb_irq_cb.setup_cb != NULL)
    {
        usb_irq_cb.setup_cb();
    }
    else
    {
        TLK_LOG_ERROR(usb_log, "No callback for setup event is set!");
        return;
    }
}

_tlk_attribute_ram_code_sec_ static void tlk_usb_data_irq_handle(void)
{
    tlk_usb_irq_clr_status(USB_IRQ_EP_DATA_STATUS);
    if (usb_irq_cb.data_cb != NULL)
    {
        usb_irq_cb.data_cb();
    }
    else
    {
        TLK_LOG_ERROR(usb_log, "No callback for data event is set!");
        return;
    }
}

_tlk_attribute_ram_code_sec_ static void tlk_usb_eos_irq_handle(void)
{
    if (usb_irq_cb.eos_cb != NULL)
    {
        usb_irq_cb.eos_cb();
    }
    else
    {
        TLK_LOG_ERROR(usb_log, "No callback for endpoint data event is set!");
        return;
    }
}

_tlk_attribute_ram_code_sec_ static void tlk_usb_suspend_irq_handle(void)
{
    if (usb_irq_cb.suspend_cb != NULL)
    {
        usb_irq_cb.suspend_cb();
    }
    else
    {
        TLK_LOG_ERROR(usb_log, "No callback for suspend event is set!");
        return;
    }
}

_tlk_attribute_ram_code_sec_ static void tlk_usb_reset_irq_handle(void)
{
    tlk_usb_irq_clr_status(USB_IRQ_RESET_STATUS);
    if (usb_irq_cb.reset_cb != NULL)
    {
        usb_irq_cb.reset_cb();
    }
    else
    {
        TLK_LOG_ERROR(usb_log, "No callback for reset event is set!");
        return;
    }
}

TLK_PLIC_ISR_REGISTER(tlk_usb_setup_irq_handle, UNISDK_PLIC_IRQ_NUM_USB_SETUP);
TLK_PLIC_ISR_REGISTER(tlk_usb_data_irq_handle, UNISDK_PLIC_IRQ_NUM_USB_DATA);
TLK_PLIC_ISR_REGISTER(tlk_usb_eos_irq_handle, UNISDK_PLIC_IRQ_NUM_USB_EDP);
TLK_PLIC_ISR_REGISTER(tlk_usb_suspend_irq_handle, UNISDK_PLIC_IRQ_NUM_USB_PWDN);
TLK_PLIC_ISR_REGISTER(tlk_usb_reset_irq_handle, UNISDK_PLIC_IRQ_NUM_USB_RESET);

/**************************************************************
 *   Driver's public functionality
 ***************************************************************/

void tlk_usb_init(void)
{
    tlk_clock_reg.wakeupen = 0;
    usb_pin_set_en(PINMUX_USB_DM_PORT, PINMUX_USB_DP_PORT, PINMUX_USB_DM_PIN, PINMUX_USB_DP_PIN);
}

void tlk_usb_deinit(void)
{
    return;
}

void tlk_usb_set_cb(struct tlk_usb_cb cb)
{
    usb_irq_cb = cb;
}

void tlk_usb_enable_manual_interrupt(int m)
{
    TLK_BM_CLR(tlk_usb_reg.edp0_mode, m);

    tlk_usb_irq_set_mask(USB_IRQ_RESET_MASK | USB_IRQ_SUSPEND_MASK);
}

void tlk_usb_irq_set_mask(enum tlk_usb_irq_mask mask)
{
    tlk_usb_reg.irq_mask |= mask;
}

void tlk_usb_irq_clr_status(enum tlk_usb_irq_status status)
{
    tlk_usb_reg.edp0_status = status;
}

void tlk_usb_pull_up(int en)
{
    TLK_ANALOG_MODIFY(TLK_AREG_POWER5, (value.bit.dp_pullup_res_en_b = en;));
}

void tlk_usb_wakeup(void)
{
    if (tlk_usb_reg.mdev_bit.wakeup_feature_o)
    {
        tlk_clock_reg.wakeupen_bit.usb_resume = 1;
        tlk_clock_reg.wakeupen_bit.usb_pwdn_i = 1;
    }
}

bool tlk_usb_is_wakeup_en(void)
{
    return tlk_usb_reg.mdev_bit.wakeup_feature_o;
}

void tlk_usb_ctrl_ep_prepare(void)
{
    tlk_usb_reg.suspend_cyc = CTRL_EP_NORMAL_PACKET_REG_VALUE;
}

void tlk_usb_ctrl_ep_reset_ptr(void)
{
    tlk_usb_reg.edp0_pointer = 0;
}

uint8_t tlk_usb_ctrl_ep_read_data(void)
{
    return tlk_usb_reg.edp0_data;
}

void tlk_usb_ep_reset_ptr(enum tlk_usb_ep_idx ep)
{
    tlk_usb_reg.edps_ptr_low[ep]          = 0;
    tlk_usb_reg.edps_ptr_high_bit[ep].val = 0;
}

uint16_t tlk_usb_get_ep_ptr(enum tlk_usb_ep_idx ep)
{
    return ((tlk_usb_reg.edps_ptr_high_bit[ep].val << 8) | (tlk_usb_reg.edps_ptr_low[ep]));
}

void tlk_usb_ep_ack_data(unsigned int ep)
{
    tlk_usb_reg.edps_ct_bit[ep].rd_ack = 1;
}

void tlk_usb_ctrl_ep_write_ctrl(unsigned char data)
{
    tlk_usb_reg.edp0_control = data;
}

void tlk_usb_set_eps_en(enum tlk_usb_ep_en ep)
{
    tlk_usb_reg.edps_en |= ep;
    tlk_usb_reg.usb_mask |= ep;
}

void tlk_usb_set_eps_dis(enum tlk_usb_ep_en ep)
{
    tlk_usb_reg.edps_en &= ~ep;
}

void tlk_usb_ep_reset(unsigned int ep)
{
    ((volatile uint8_t*) &tlk_usb_reg.edps_ct_bit[0])[ep] = 0;
}

uint8_t tlk_usb_ep_irq_get(void)
{
    return tlk_usb_reg.usb_irq;
}

void tlk_usb_ep_irq_clear(enum tlk_usb_ep_en ep)
{
    tlk_usb_reg.usb_irq = ep;
}

uint16_t tlk_usb_ctrl_ep_write(uint8_t* buffer, uint16_t total_bytes)
{
    uint16_t i = 0;

    tlk_usb_ctrl_ep_prepare();
    tlk_usb_ctrl_ep_reset_ptr();

    if (total_bytes == 0)
    { /* Send zero length packet for acknowledge */
        tlk_usb_ctrl_ep_write_ctrl(TLK_USB_EP_STA_ACK);
    }
    else
    {
        for (i = 0; i < total_bytes; i++)
        {
            tlk_usb_reg.edp0_data = buffer[i];
        }
        tlk_usb_ctrl_ep_write_ctrl(TLK_USB_EP_DAT_ACK);
    }

    return i;
}

uint16_t tlk_usb_ctrl_ep_read(uint8_t* buffer, uint16_t total_bytes)
{
    uint16_t i = 0;

    tlk_usb_ctrl_ep_prepare();
    tlk_usb_ctrl_ep_reset_ptr();

    for (i = 0; i < total_bytes; i++)
    {
        buffer[i] = tlk_usb_ctrl_ep_read_data();
    }

    return i;
}

uint16_t tlk_usb_ep_read(enum tlk_usb_ep_idx ep_id, uint8_t* buffer, uint16_t total_bytes)
{
    uint16_t i   = 0;
    uint16_t len = 0;

    len = tlk_usb_get_ep_ptr(ep_id);
    tlk_usb_ep_reset_ptr((uint8_t) ep_id);

    if (len && (len <= total_bytes))
    {
        for (i = 0; i < len; i++)
        {
            buffer[i] = tlk_usb_reg.edps_data[ep_id];
        }
    }
    tlk_usb_ep_ack_data(ep_id);

    return i;
}

uint16_t tlk_usb_ep_write(enum tlk_usb_ep_idx ep_id, uint8_t* buffer, uint16_t total_bytes)
{
    uint8_t i = 0;

    if ((ep_id == TLK_USB_OUT_EP5_IDX) && (ep_id != TLK_USB_OUT_EP6_IDX))
    {
        TLK_LOG_ERROR(usb_log, "Wrong endpoint for write operation");
        return 0;
    }
    tlk_usb_ep_reset_ptr(ep_id);

    for (i = 0; i < total_bytes; i++)
    {
        tlk_usb_reg.edps_data[ep_id] = buffer[i];
    }
    tlk_usb_ep_ack_data(ep_id);

    return i;
}
