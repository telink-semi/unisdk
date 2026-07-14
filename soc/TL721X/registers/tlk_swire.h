#ifndef TLK_TLK_SWIRE_REGISTERS_H_
#define TLK_TLK_SWIRE_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_swire_control_e {
    TLK_SWIRE_CONTROL_WRITE      = TLK_BIT(0),
    TLK_SWIRE_CONTROL_READ       = TLK_BIT(1),
    TLK_SWIRE_CONTROL_COMMAND    = TLK_BIT(2),
    TLK_SWIRE_CONTROL_ERROR_FLAG = TLK_BIT(3),
    TLK_SWIRE_CONTROL_EOP        = TLK_BIT(4),
    TLK_SWIRE_CONTROL_USB_DETECT = TLK_BIT(6),
    TLK_SWIRE_CONTROL_USB_EN     = TLK_BIT(7),
} tlk_swire_control_e;

typedef enum tlk_swire_id_e {
    TLK_SWIRE_ID_FIFO_MODE = TLK_BIT(7),
} tlk_swire_id_e;

typedef struct __attribute__((packed)) {
    uint8_t data; // address: 0x80100c00, offset: 0x0
    union {
        struct {
            unsigned write : 1;
            unsigned read : 1;
            unsigned command : 1;
            unsigned error_flag : 1;
            unsigned eop : 1;
            unsigned reserved : 1;
            unsigned usb_detect : 1;
            unsigned usb_en : 1;
        } __attribute__((packed)) control_bit;
        struct {
            enum tlk_swire_control_e __attribute__((packed)) control : 8;
        }; // address: 0x80100c01, offset: 0x1
    };
    union {
        struct {
            unsigned clk_div : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0x80100c02, offset: 0x2
    };
    union {
        struct {
            unsigned id_valid : 5;
            unsigned reserved : 2;
            unsigned fifo_mode : 1;
        } __attribute__((packed)) id_bit;
        struct {
            enum tlk_swire_id_e __attribute__((packed)) id : 8;
        }; // address: 0x80100c03, offset: 0x3
    };
} tlk_swire_reg_t;

#define TLK_SWIRE_BASE_ADDR (0x80100c00U)
#define tlk_swire_reg (*(volatile tlk_swire_reg_t *) TLK_SWIRE_BASE_ADDR)

#endif