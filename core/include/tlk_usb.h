#ifndef TLK_INCLUDE_DRIVERS_USB_H_
#define TLK_INCLUDE_DRIVERS_USB_H_

enum tlk_usb_ep_en
{
    TLK_USB_EDP8_EN = TLK_BIT(0),
    TLK_USB_EDP1_EN = TLK_BIT(1),
    TLK_USB_EDP2_EN = TLK_BIT(2),
    TLK_USB_EDP3_EN = TLK_BIT(3),
    TLK_USB_EDP4_EN = TLK_BIT(4),
    TLK_USB_EDP5_EN = TLK_BIT(5),
    TLK_USB_EDP6_EN = TLK_BIT(6),
    TLK_USB_EDP7_EN = TLK_BIT(7),
    TLK_USB_ALL_EN  = 0xFF,
};

enum tlk_usb_ep_state
{
    TLK_USB_EP_DAT_ACK     = TLK_BIT(0),
    TLK_USB_EP_DAT_BUSY    = TLK_BIT(0),
    TLK_USB_EP_DAT_STALL   = TLK_BIT(1),
    TLK_USB_EP_STA_ACK     = TLK_BIT(2),
    TLK_USB_EP_STA_STALL   = TLK_BIT(3),
    TLK_USB_EP_IRQ_SETADDR = TLK_BIT(6),
};

enum tlk_usb_ep_irq
{
    TLK_USB_EDP8_IRQ = TLK_BIT(0),
    TLK_USB_EDP1_IRQ = TLK_BIT(1),
    TLK_USB_EDP2_IRQ = TLK_BIT(2),
    TLK_USB_EDP3_IRQ = TLK_BIT(3),
    TLK_USB_EDP4_IRQ = TLK_BIT(4),
    TLK_USB_EDP5_IRQ = TLK_BIT(5),
    TLK_USB_EDP6_IRQ = TLK_BIT(6),
    TLK_USB_EDP7_IRQ = TLK_BIT(7),
};

enum tlk_usb_ep_auto_irq
{
    TLK_USB_CTRL_EP_AUTO_ADDR = TLK_BIT(0),
    TLK_USB_CTRL_EP_AUTO_CFG  = TLK_BIT(1),
    TLK_USB_CTRL_EP_AUTO_INTF = TLK_BIT(2),
    TLK_USB_CTRL_EP_AUTO_STA  = TLK_BIT(3),
    TLK_USB_CTRL_EP_AUTO_SYN  = TLK_BIT(4),
    TLK_USB_CTRL_EP_AUTO_DESC = TLK_BIT(5),
    TLK_USB_CTRL_EP_AUTO_FEAT = TLK_BIT(6),
    TLK_USB_CTRL_EP_AUTO_STD  = TLK_BIT(7),
};


enum tlk_usb_irq_mask
{
    USB_IRQ_RESET_MASK   = TLK_BIT(0),
    USB_IRQ_250US_MASK   = TLK_BIT(1),
    USB_IRQ_SUSPEND_MASK = TLK_BIT(2), 
    USB_IRQ_SOF_MASK     = TLK_BIT(3), /* IRQ from 3 to 7 is not supported by B92 TODO: Add check in function */
    USB_IRQ_SETUP_MASK   = TLK_BIT(4),
    USB_IRQ_DATA_MASK    = TLK_BIT(5),
    USB_IRQ_STATUS_MASK  = TLK_BIT(6),
    USB_IRQ_SETINF_MASK  = TLK_BIT(7),
};

enum tlk_usb_irq_status
{
    USB_IRQ_RESET_STATUS    = TLK_BIT(0),
    USB_IRQ_250US_STATUS    = TLK_BIT(1),
    USB_IRQ_SUSPEND_STATUS  = TLK_BIT(2),
    USB_IRQ_SOF_STATUS      = TLK_BIT(3), /* Not supported by B92 TODO: Add check */
    USB_IRQ_EP_SETUP_STATUS = TLK_BIT(4),
    USB_IRQ_EP_DATA_STATUS  = TLK_BIT(5),
    USB_IRQ_EP_STA_STATUS   = TLK_BIT(6),
    USB_IRQ_EP_INTF_STATUS  = TLK_BIT(7),
};

enum tlk_usb_ep_idx {
    TLK_USB_EP0_IDX = 0,     /* only for control transfer */
    TLK_USB_IN_EP1_IDX = 1,  /* only IN */
    TLK_USB_IN_EP2_IDX = 2,  /* only IN */
    TLK_USB_IN_EP3_IDX = 3,  /* only IN */
    TLK_USB_IN_EP4_IDX = 4,  /* only IN */
    TLK_USB_OUT_EP5_IDX = 5, /* only OUT */
    TLK_USB_OUT_EP6_IDX = 6, /* only OUT */
    TLK_USB_IN_EP7_IDX = 7,  /* only IN */
    TLK_USB_IN_EP8_IDX = 8,  /* only IN */
};

struct tlk_usb_cb {
    void (*setup_cb)(void);
    void (*data_cb)(void);
    void (*eos_cb)(void);
    void (*suspend_cb)(void);
    void (*reset_cb)(void);
    void (*sof_cb)(uint32_t); /* Not supported by B92 */
};

void tlk_usb_init(void);

void tlk_usb_deinit(void);

void tlk_usb_set_cb(struct tlk_usb_cb cb);

void tlk_usb_enable_manual_interrupt(int m);

void tlk_usb_set_irq_mask(enum tlk_usb_irq_mask mask);

void tlk_usb_clr_irq_status(enum tlk_usb_irq_status status);

void tlk_usb_pull_up(int en);

void tlk_usb_wakeup(void);

bool tlk_usb_is_wakeup_en(void);

void tlk_usb_ctrl_ep_prepare(void);

void tlk_usb_reset_ctrl_ep_ptr(void);

uint8_t tlk_usb_read_ctrl_ep_data(void);

void tlk_usb_reset_ep_ptr(enum tlk_usb_ep_idx ep);

void tlk_usb_data_ep_ack(unsigned int ep);

void tlk_usb_write_ctrl_ep_ctrl(unsigned char data);

void tlk_usb_set_eps_en(enum tlk_usb_ep_en ep);

void tlk_usb_set_eps_dis(enum tlk_usb_ep_en ep);

void tlk_usb_reset_ep(unsigned int ep);

uint8_t tlk_usb_ep_irq_get(void);

void tlk_usb_ep_irq_clear(enum tlk_usb_ep_en ep);

uint16_t tlk_usb_ctrl_ep_write(uint8_t *buffer, uint16_t total_bytes);

uint16_t tlk_usb_ctrl_ep_read(uint8_t * buffer, uint16_t total_bytes);

uint16_t tlk_usb_ep_read(enum tlk_usb_ep_idx ep_id, uint8_t * buffer, uint16_t total_bytes);

uint16_t tlk_usb_ep_write(enum tlk_usb_ep_idx ep_id, uint8_t * buffer, uint16_t total_bytes);

#endif