/*
 * tusb_config.h
 *
 * TinyUSB Configuration File
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_
#include "properties/tlk_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUSB_MCU
  #define CFG_TUSB_MCU    OPT_MCU_NONE 
#endif

#define TUP_DCD_ENDPOINT_MAX UNISDK_USB_EP_NUM
#define BOARD_TUD_RHPORT 0

/* Select Р RTOS
 * OPT_OS_NONE, OPT_OS_FREERTOS, OPT_OS_ZEPHYR etc.
 */
#define CFG_TUSB_OS       OPT_OS_NONE


/* CFG_TUSB_DEBUG for debugging
 * 0 : no debug
 * 1 : print error
 * 2 : print warning
 * 3 : print info
 */
#define CFG_TUSB_DEBUG    CONFIG_TLK_USB_LOG_LEVEL
#ifdef CFG_TUSB_DEBUG
#include <stdio.h> /* Fix snprintf missed declaration */
#endif
#define CFG_TUSB_DEBUG_PRINTF _tlk_critical_printf


//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_ENABLED   1     // Enable Device mode
#define CFG_TUH_ENABLED   0     // Disable Host mode

// Byffer size Endpoint 0 (Control endpoint)
// 64 — default Full Speed / High Speed USB for Buteo and Tarcel
#define CFG_TUD_ENDPOINT0_SIZE  UNISDK_USB_CTRL_EP_SIZE

//--------------------------------------------------------------------
// CLASS CONFIGURATION
//--------------------------------------------------------------------

#define CFG_TUD_CDC       CONFIG_TLK_USB_CLASS_CDC     // Communication Device Class (Serial Port)

//--------------------------------------------------------------------
// CDC CONFIGURATION
//--------------------------------------------------------------------
#if CFG_TUD_CDC

  #define CFG_TUD_CDC_RX_BUFSIZE   CONFIG_TLK_USB_CDC_RX_BUF_SIZE
  #define CFG_TUD_CDC_TX_BUFSIZE   CONFIG_TLK_USB_CDC_TX_BUF_SIZE

#endif

//--------------------------------------------------------------------
// MEMORY & COMPILER ATTRIBUTES
//--------------------------------------------------------------------

#ifndef CFG_TUSB_MEM_SECTION
  #define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
  #define CFG_TUSB_MEM_ALIGN      __attribute__ ((aligned(4)))
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */