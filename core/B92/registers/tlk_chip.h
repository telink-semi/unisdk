#ifndef TLK_RESET_REGISTERS_H_
#define TLK_RESET_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x20]; // address: 0x801401c0, offset: 0x0
    union {
        struct {
            unsigned lspi : 1;
            unsigned i2c : 1;
            unsigned uart0 : 1;
            unsigned usb : 1;
            unsigned pwm : 1;
            unsigned reserved : 1;
            unsigned uart1 : 1;
            unsigned swires : 1;
        } __attribute__((packed)) reset0_bit;
        uint8_t reset0; // address: 0x801401e0, offset: 0x20
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned stimer : 1;
            unsigned dma : 1;
            unsigned algm : 1;
            unsigned pke : 1;
            unsigned reserved1 : 1;
            unsigned gspi : 1;
            unsigned spislv : 1;
        } __attribute__((packed)) reset1_bit;
        uint8_t reset1; // address: 0x801401e1, offset: 0x21
    };
    union {
        struct {
            unsigned timer : 1;
            unsigned audio : 1;
            unsigned i2c1 : 1;
            unsigned mcu_rst_dis : 1;
            unsigned mcu_rst_en : 1;
            unsigned lm : 1;
            unsigned trng : 1;
            unsigned dpr : 1;
        } __attribute__((packed)) reset2_bit;
        uint8_t reset2; // address: 0x801401e2, offset: 0x22
    };
    union {
        struct {
            unsigned zb : 1;
            unsigned zb_mstclk : 1;
            unsigned zb_lpclk : 1;
            unsigned zb_crypt : 1;
            unsigned mspi : 1;
            unsigned qdec : 1;
            unsigned saradc : 1;
            unsigned alg : 1;
        } __attribute__((packed)) reset3_bit;
        uint8_t reset3; // address: 0x801401e3, offset: 0x23
    };
    uint8_t reserved1[0xb]; // address: 0x801401e4, offset: 0x24
    union {
        struct {
            unsigned suspend_en : 1;
            unsigned reserved0 : 3;
            unsigned ramcrc_clren_tgl : 1;
            unsigned rst_all : 1;
            unsigned reserved1 : 1;
            unsigned stall_en_trg : 1;
        } __attribute__((packed)) pwdnen_bit;
        uint8_t pwdnen; // address: 0x801401ef, offset: 0x2f
    };
} tlk_reset_reg_t;

#define TLK_RESET_BASE_ADDR (0x801401c0U)
#define tlk_reset_reg (*(volatile tlk_reset_reg_t *) TLK_RESET_BASE_ADDR)

#endif

