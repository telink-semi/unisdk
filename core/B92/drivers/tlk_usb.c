#include "core/include/tlk_usb.h"
#include "registers/tlk_usb.h"
#include "registers/tlk_gpio.h"
#include "registers/tlk_clock.h"
#include "registers/tlk_chip.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_plic.h"
#include "system/debug/log/tlk_log.h"
#include "properties/tlk_plic.h"

TLK_LOG_CREATE(usb_log,"USB");

#define CTRL_EP_NORMAL_PACKET_REG_VALUE 0x38

struct tlk_usb_cb usb_irq_cb;

/**************************************************************
*   Internal functions for working with USB HW registers
***************************************************************/

static void set_pin_to_input(enum tlk_gpio_port port, enum tlk_gpio_pin pin)
{
    TLK_BM_SET(tlk_gpio_reg.port_config[port].output_en, pin);
    TLK_BM_SET(tlk_gpio_reg.port_config[port].input_en, pin);
}

static void usb_set_pin_en(enum tlk_gpio_port port_dm, enum tlk_gpio_port port_dp,
                    enum tlk_gpio_pin pin_dm, enum tlk_gpio_pin pin_dp)
{
    tlk_gpio_reg.pin_func[port_dm][TLK_BIT_LOW_BIT(pin_dm)] = 0x00;   /* DM */
    tlk_gpio_reg.pin_func[port_dp][TLK_BIT_LOW_BIT(pin_dp)] = 0x00;   /* DP */
    tlk_gpio_disable(port_dm, pin_dm);
    tlk_gpio_disable(port_dp, pin_dp);
    set_pin_to_input(port_dm, pin_dm);
    set_pin_to_input(port_dp, pin_dp);

    write_reg8(0x100c01, (read_reg8(0x100c01) | TLK_BIT(7)));   //swire_usb_en

}

static void tlk_usb_setup_irq_handler(void)
{
    tlk_usb_clr_irq_status(USB_IRQ_EP_SETUP_STATUS);
    if(usb_irq_cb.setup_cb != NULL) {
        usb_irq_cb.setup_cb();
    } else {
        TLK_LOG_ERROR(usb_log, "No callback for setup event is set!");
        return;
    }
}

static void tlk_usb_data_irq_handler(void)
{
    tlk_usb_clr_irq_status(USB_IRQ_EP_DATA_STATUS);
    if(usb_irq_cb.data_cb != NULL) {
        usb_irq_cb.data_cb();
    } else {
        TLK_LOG_ERROR(usb_log, "No callback for data event is set!");
        return;
    }
}

static void tlk_usb_eos_irq_handler(void)
{
    if(usb_irq_cb.eos_cb != NULL) {
        usb_irq_cb.eos_cb();
    } else {
        TLK_LOG_ERROR(usb_log, "No callback for endpoint data event is set!");
        return;
    }
}

static void tlk_usb_suspend_irq_handler(void)
{
    if(usb_irq_cb.suspend_cb != NULL) {
        usb_irq_cb.suspend_cb();
    } else {
        TLK_LOG_ERROR(usb_log, "No callback for suspend event is set!");
        return;
    }
}

static void tlk_usb_reset_irq_handler(void)
{
    tlk_usb_clr_irq_status(USB_IRQ_RESET_STATUS);
    if(usb_irq_cb.reset_cb != NULL) {
        usb_irq_cb.reset_cb();
    } else {
        TLK_LOG_ERROR(usb_log, "No callback for reset event is set!");
        return;
    }
}


TLK_PLIC_ISR_REGISTER(tlk_usb_setup_irq_handler, UNISDK_PLIC_IRQ_NUM_USB_SETUP);
TLK_PLIC_ISR_REGISTER(tlk_usb_data_irq_handler, UNISDK_PLIC_IRQ_NUM_USB_DATA);
TLK_PLIC_ISR_REGISTER(tlk_usb_eos_irq_handler, UNISDK_PLIC_IRQ_NUM_USB_EDP);
TLK_PLIC_ISR_REGISTER(tlk_usb_suspend_irq_handler, UNISDK_PLIC_IRQ_NUM_USB_PWDN);
TLK_PLIC_ISR_REGISTER(tlk_usb_reset_irq_handler, UNISDK_PLIC_IRQ_NUM_USB_RESET);

/**************************************************************
*   Driver's public functionality
***************************************************************/

void tlk_usb_init(void)
{
    tlk_clock_reg.wakeupen = 0;
    usb_set_pin_en(PINMUX_USB_DM_PORT, PINMUX_USB_DP_PORT, PINMUX_USB_DM_PIN, PINMUX_USB_DP_PIN);
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

    tlk_usb_set_irq_mask( USB_IRQ_RESET_MASK | USB_IRQ_SUSPEND_MASK );
}

void tlk_usb_set_irq_mask(enum tlk_usb_irq_mask mask)
{
    tlk_usb_reg.irq_mask |= mask;
}

void tlk_usb_clr_irq_status(enum tlk_usb_irq_status status)
{
    tlk_usb_reg.edp0_status = status;
}

void tlk_usb_pull_up(int en)
{
    uint8_t dat = tlk_analog_read_reg8(0x0b);
    if (en) {
        dat = dat | TLK_BIT(7);
    } else {
        dat = dat & (~TLK_BIT(7));
    }

    tlk_analog_write_reg8(0x0b, dat);
}

void tlk_usb_wakeup(void)
{
    if (tlk_usb_reg.mdev_bit.wakeup_feature_o) {
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

void tlk_usb_reset_ctrl_ep_ptr(void)
{
    tlk_usb_reg.edp0_pointer = 0;
}

uint8_t tlk_usb_read_ctrl_ep_data(void)
{
    return tlk_usb_reg.edp0_data;
}

void tlk_usb_reset_ep_ptr(enum tlk_usb_ep_idx ep)
{
    tlk_usb_reg.edps_ptr_low[ep]  = 0;
    tlk_usb_reg.edps_ptr_high_bit[ep].val = 0;
}

uint16_t tlk_usb_get_ep_ptr(enum tlk_usb_ep_idx ep)
{
    return ((tlk_usb_reg.edps_ptr_high_bit[ep].val << 8) | (tlk_usb_reg.edps_ptr_low[ep]));
}

void tlk_usb_data_ep_ack(unsigned int ep)
{
    tlk_usb_reg.edps_ct_bit[ep].rd_ack = 1;
}

void tlk_usb_write_ctrl_ep_ctrl(unsigned char data)
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

void tlk_usb_reset_ep(unsigned int ep)
{
    tlk_usb_reg.edps_ct[ep] = 0;
}

uint8_t tlk_usb_ep_irq_get()
{
    return tlk_usb_reg.usb_irq;
}

void tlk_usb_ep_irq_clear(enum tlk_usb_ep_en ep)
{
    tlk_usb_reg.usb_irq = ep;
}

uint16_t tlk_usb_ctrl_ep_write(uint8_t *buffer, uint16_t total_bytes)
{
    uint16_t i = 0;

    tlk_usb_ctrl_ep_prepare();
    tlk_usb_reset_ctrl_ep_ptr();

    
    if(total_bytes == 0){ /* Send zero length packet for acknowledge */
        tlk_usb_write_ctrl_ep_ctrl(TLK_USB_EP_STA_ACK);
    } else {
        for (i = 0; i < total_bytes; i++) {
            tlk_usb_reg.edp0_data = buffer[i];
        }
        tlk_usb_write_ctrl_ep_ctrl(TLK_USB_EP_DAT_ACK);
    }

    return i;
}

uint16_t tlk_usb_ctrl_ep_read(uint8_t * buffer, uint16_t total_bytes)
{
    uint16_t i = 0;

    tlk_usb_ctrl_ep_prepare();
    tlk_usb_reset_ctrl_ep_ptr();

    for(i = 0; i < total_bytes; i++) {
        buffer[i] = tlk_usb_read_ctrl_ep_data();
    }

    return i;
}

uint16_t tlk_usb_ep_read(enum tlk_usb_ep_idx ep_id, uint8_t * buffer, uint16_t total_bytes)
{
    uint16_t i = 0;
    uint16_t len = 0;

    len = tlk_usb_get_ep_ptr(ep_id);
    tlk_usb_reset_ep_ptr((uint8_t)ep_id);

    if(len && (len <= total_bytes)){
        for(i = 0; i < len; i++) {
            buffer[i] = tlk_usb_reg.edps_data[ep_id]; 
        }
    }
    tlk_usb_data_ep_ack(ep_id);

    return i;
}

uint16_t tlk_usb_ep_write(enum tlk_usb_ep_idx ep_id, uint8_t * buffer, uint16_t total_bytes)
{
    uint8_t i = 0;

    if((ep_id == TLK_USB_OUT_EP5_IDX) && (ep_id != TLK_USB_OUT_EP6_IDX)) {
        TLK_LOG_ERROR(usb_log, "Wrong endpoint for write operation");
        return 0;
    }
    tlk_usb_reset_ep_ptr(ep_id);

    for(i = 0; i < total_bytes; i++) {
        tlk_usb_reg.edps_data[ep_id] = buffer[i];
    }
    tlk_usb_data_ep_ack(ep_id);

    return i;
}
