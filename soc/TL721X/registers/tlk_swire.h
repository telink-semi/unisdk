#ifndef TLK_SWIRE_REGISTERS_H_
#define TLK_SWIRE_REGISTERS_H_

#include <stdint.h>

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
        uint8_t control; // address: 0x80100c01, offset: 0x1
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
        uint8_t id; // address: 0x80100c03, offset: 0x3
    };
} tlk_swire_reg_t;

#define TLK_SWIRE_BASE_ADDR (0x80100c00U)
#define tlk_swire_reg (*(volatile tlk_swire_reg_t *) TLK_SWIRE_BASE_ADDR)

#endif

