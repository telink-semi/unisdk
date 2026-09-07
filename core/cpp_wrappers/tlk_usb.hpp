#ifndef USB_DRIVER_HPP
#define USB_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_usb.h"
#ifdef __cplusplus
}
#endif

class UsbDriver
{
  public:
    UsbDriver() = delete;

    static void init()
    {
        tlk_usb_init();
    }

    static void deinit()
    {
        tlk_usb_deinit();
    }

    static void setCb(tlk_usb_cb cb)
    {
        tlk_usb_set_cb(cb);
    }

    static void enableManualInterrupt(int m)
    {
        tlk_usb_enable_manual_interrupt(m);
    }

    static void setIrqMask(tlk_usb_irq_mask mask)
    {
        tlk_usb_set_irq_mask(mask);
    }

    static void clrIrqStatus(tlk_usb_irq_status status)
    {
        tlk_usb_clr_irq_status(status);
    }

    static void pullUp(int en)
    {
        tlk_usb_pull_up(en);
    }

    static void wakeup()
    {
        tlk_usb_wakeup();
    }

    static bool isWakeupEn()
    {
        return tlk_usb_is_wakeup_en();
    }

    static void ctrlEpPrepare()
    {
        tlk_usb_ctrl_ep_prepare();
    }

    static void resetCtrlEpPtr()
    {
        tlk_usb_reset_ctrl_ep_ptr();
    }

    static uint8_t readCtrlEpData()
    {
        return tlk_usb_read_ctrl_ep_data();
    }

    static void resetEpPtr(tlk_usb_ep_idx ep)
    {
        tlk_usb_reset_ep_ptr(ep);
    }

    static void dataEpAck(unsigned int ep)
    {
        tlk_usb_data_ep_ack(ep);
    }

    static void writeCtrlEpCtrl(unsigned char data)
    {
        tlk_usb_write_ctrl_ep_ctrl(data);
    }

    static void setEpsEn(tlk_usb_ep_en ep)
    {
        tlk_usb_set_eps_en(ep);
    }

    static void setEpsDis(tlk_usb_ep_en ep)
    {
        tlk_usb_set_eps_dis(ep);
    }

    static void resetEp(unsigned int ep)
    {
        tlk_usb_reset_ep(ep);
    }

    static uint8_t epIrqGet()
    {
        return tlk_usb_ep_irq_get();
    }

    static void epIrqClear(tlk_usb_ep_en ep)
    {
        tlk_usb_ep_irq_clear(ep);
    }

    static uint16_t ctrlEpWrite(uint8_t* buffer, uint16_t total_bytes)
    {
        return tlk_usb_ctrl_ep_write(buffer, total_bytes);
    }

    static uint16_t ctrlEpRead(uint8_t* buffer, uint16_t total_bytes)
    {
        return tlk_usb_ctrl_ep_read(buffer, total_bytes);
    }

    static uint16_t epRead(tlk_usb_ep_idx ep_id, uint8_t* buffer, uint16_t total_bytes)
    {
        return tlk_usb_ep_read(ep_id, buffer, total_bytes);
    }

    static uint16_t epWrite(tlk_usb_ep_idx ep_id, uint8_t* buffer, uint16_t total_bytes)
    {
        return tlk_usb_ep_write(ep_id, buffer, total_bytes);
    }
};

#endif
