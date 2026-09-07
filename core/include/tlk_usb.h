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
    USB_IRQ_SOF_MASK =
        TLK_BIT(3), /* IRQ from 3 to 7 is not supported by B92 TODO: Add check in function */
    USB_IRQ_SETUP_MASK  = TLK_BIT(4),
    USB_IRQ_DATA_MASK   = TLK_BIT(5),
    USB_IRQ_STATUS_MASK = TLK_BIT(6),
    USB_IRQ_SETINF_MASK = TLK_BIT(7),
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

enum tlk_usb_ep_idx
{
    TLK_USB_EP0_IDX     = 0, /* only for control transfer */
    TLK_USB_IN_EP1_IDX  = 1, /* only IN */
    TLK_USB_IN_EP2_IDX  = 2, /* only IN */
    TLK_USB_IN_EP3_IDX  = 3, /* only IN */
    TLK_USB_IN_EP4_IDX  = 4, /* only IN */
    TLK_USB_OUT_EP5_IDX = 5, /* only OUT */
    TLK_USB_OUT_EP6_IDX = 6, /* only OUT */
    TLK_USB_IN_EP7_IDX  = 7, /* only IN */
    TLK_USB_IN_EP8_IDX  = 8, /* only IN */
};

struct tlk_usb_cb
{
    void (*setup_cb)(void);
    void (*data_cb)(void);
    void (*eos_cb)(void);
    void (*suspend_cb)(void);
    void (*reset_cb)(void);
    void (*sof_cb)(uint32_t); /* Not supported by B92 */
};

/**
 * @brief Initialize the USB module.
 *
 * @return None.
 */
void tlk_usb_init(void);

/**
 * @brief Deinitialize the USB module.
 *
 * @return None.
 */
void tlk_usb_deinit(void);

/**
 * @brief Set USB event callbacks.
 *
 * @param cb  Callback structure containing function pointers for USB events.
 *
 * @return None.
 */
void tlk_usb_set_cb(struct tlk_usb_cb cb);

/**
 * @brief Enable or disable manual interrupt handling.
 *
 * @param m  Non-zero to enable manual interrupt mode, zero to disable.
 *
 * @return None.
 */
void tlk_usb_enable_manual_interrupt(int m);

/**
 * @brief Set the USB interrupt mask.
 *
 * @param mask  Interrupt mask to apply.
 *
 * @return None.
 */
void tlk_usb_irq_set_mask(enum tlk_usb_irq_mask mask);

/**
 * @brief Clear the USB interrupt status flags.
 *
 * @param status  Interrupt status flags to clear.
 *
 * @return None.
 */
void tlk_usb_irq_clr_status(enum tlk_usb_irq_status status);

/**
 * @brief Control the USB D+ pull-up resistor (device connection).
 *
 * @param en  Non-zero to pull up (connect), zero to release (disconnect).
 *
 * @return None.
 */
void tlk_usb_pull_up(int en);

/**
 * @brief Wake up the USB module from suspend state.
 *
 * @return None.
 */
void tlk_usb_wakeup(void);

/**
 * @brief Check if USB wake-up is enabled.
 *
 * @return true if wake-up is enabled, false otherwise.
 */
bool tlk_usb_is_wakeup_en(void);

/**
 * @brief Prepare the control endpoint for a setup packet.
 *
 * @return None.
 */
void tlk_usb_ctrl_ep_prepare(void);

/**
 * @brief Reset the control endpoint data pointer.
 *
 * @return None.
 */
void tlk_usb_ctrl_ep_reset_ptr(void);

/**
 * @brief Read a byte from the control endpoint.
 *
 * @return The byte read from the control endpoint.
 */
uint8_t tlk_usb_ctrl_ep_read_data(void);

/**
 * @brief Reset the data pointer for a specified endpoint.
 *
 * @param ep  Endpoint index.
 *
 * @return None.
 */
void tlk_usb_ep_reset_ptr(enum tlk_usb_ep_idx ep);

/**
 * @brief Acknowledge a data endpoint interrupt.
 *
 * @param ep  Endpoint number to acknowledge.
 *
 * @return None.
 */
void tlk_usb_ep_ack_data(unsigned int ep);

/**
 * @brief Write a control byte to the control endpoint control register.
 *
 * @param data  Control byte to write.
 *
 * @return None.
 */
void tlk_usb_ctrl_ep_write_ctrl(unsigned char data);

/**
 * @brief Enable specified USB endpoints.
 *
 * @param ep  Bitmask of endpoints to enable.
 *
 * @return None.
 */
void tlk_usb_set_eps_en(enum tlk_usb_ep_en ep);

/**
 * @brief Disable specified USB endpoints.
 *
 * @param ep  Bitmask of endpoints to disable.
 *
 * @return None.
 */
void tlk_usb_set_eps_dis(enum tlk_usb_ep_en ep);

/**
 * @brief Reset a specified USB endpoint.
 *
 * @param ep  Endpoint number to reset.
 *
 * @return None.
 */
void tlk_usb_ep_reset(unsigned int ep);

/**
 * @brief Get the pending endpoint interrupt status.
 *
 * @return Bitmask of endpoints with pending interrupts.
 */
uint8_t tlk_usb_ep_irq_get(void);

/**
 * @brief Clear the interrupt flag for a specified endpoint.
 *
 * @param ep  Bitmask of endpoints to clear interrupts for.
 *
 * @return None.
 */
void tlk_usb_ep_irq_clear(enum tlk_usb_ep_en ep);

/**
 * @brief Write data to the control endpoint.
 *
 * @param buffer      Pointer to the data buffer to send.
 * @param total_bytes Number of bytes to write.
 *
 * @return Number of bytes actually written.
 */
uint16_t tlk_usb_ctrl_ep_write(uint8_t* buffer, uint16_t total_bytes);

/**
 * @brief Read data from the control endpoint.
 *
 * @param buffer      Pointer to the buffer to store received data.
 * @param total_bytes Number of bytes to read.
 *
 * @return Number of bytes actually read.
 */
uint16_t tlk_usb_ctrl_ep_read(uint8_t* buffer, uint16_t total_bytes);

/**
 * @brief Read data from a specified USB endpoint.
 *
 * @param ep_id       Endpoint index to read from.
 * @param buffer      Pointer to the buffer to store received data.
 * @param total_bytes Number of bytes to read.
 *
 * @return Number of bytes actually read.
 */
uint16_t tlk_usb_ep_read(enum tlk_usb_ep_idx ep_id, uint8_t* buffer, uint16_t total_bytes);

/**
 * @brief Write data to a specified USB endpoint.
 *
 * @param ep_id       Endpoint index to write to.
 * @param buffer      Pointer to the data buffer to send.
 * @param total_bytes Number of bytes to write.
 *
 * @return Number of bytes actually written.
 */
uint16_t tlk_usb_ep_write(enum tlk_usb_ep_idx ep_id, uint8_t* buffer, uint16_t total_bytes);

#endif
