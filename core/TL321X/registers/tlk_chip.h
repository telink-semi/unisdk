#ifndef TLK_RESET_REGISTERS_H_
#define TLK_RESET_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x20]; // address: 0x80140800, offset: 0x0
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned i2c : 1;
            unsigned uart0 : 1;
            unsigned usb : 1;
            unsigned pwm : 1;
            unsigned reserved1 : 1;
            unsigned uart1 : 1;
            unsigned swires : 1;
        } __attribute__((packed)) reset0_bit;
        uint8_t reset0; // address: 0x80140820, offset: 0x20
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
        uint8_t reset1; // address: 0x80140821, offset: 0x21
    };
    union {
        struct {
            unsigned timer : 1;
            unsigned audio : 1;
            unsigned i2c1 : 1;
            unsigned mcu_rst : 1;
            unsigned mcu_during_suspend : 1;
            unsigned lm : 1;
            unsigned trng : 1;
            unsigned dpr : 1;
        } __attribute__((packed)) reset2_bit;
        uint8_t reset2; // address: 0x80140822, offset: 0x22
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned trace : 1;
            unsigned brom : 1;
            unsigned reserved1 : 1;
            unsigned mspi : 1;
            unsigned qdec : 1;
            unsigned saradc : 1;
            unsigned alg : 1;
        } __attribute__((packed)) reset3_bit;
        uint8_t reset3; // address: 0x80140823, offset: 0x23
    };
    uint8_t reserved1[0xb]; // address: 0x80140824, offset: 0x24
    union {
        struct {
            unsigned suspend_en : 1;
            unsigned reserved0 : 3;
            unsigned ramcrc_clren_tgl : 1;
            unsigned rst_all : 1;
            unsigned reserved1 : 1;
            unsigned stall_en_trg : 1;
        } __attribute__((packed)) pwdnen_bit;
        uint8_t pwdnen; // address: 0x8014082f, offset: 0x2f
    };
    uint8_t reserved2[0x10]; // address: 0x80140830, offset: 0x30
    union {
        struct {
            unsigned reserved0 : 4;
            unsigned ske : 1;
            unsigned hash : 1;
            unsigned reserved1 : 1;
            unsigned zb : 1;
        } __attribute__((packed)) t_reset4_bit;
        uint8_t t_reset4; // address: 0x80140840, offset: 0x40
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned uart2 : 1;
            unsigned reserved1 : 2;
            unsigned ir_learn : 1;
            unsigned key_scan : 1;
            unsigned pem : 1;
            unsigned reserved2 : 1;
        } __attribute__((packed)) t_reset5_bit;
        uint8_t t_reset5; // address: 0x80140841, offset: 0x41
    };
    union {
        struct {
            unsigned rz : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) t_reset6_bit;
        uint8_t t_reset6; // address: 0x80140842, offset: 0x42
    };
} tlk_reset_reg_t;

#define TLK_RESET_BASE_ADDR (0x80140800U)
#define tlk_reset_reg (*(volatile tlk_reset_reg_t *) TLK_RESET_BASE_ADDR)

#endif

