#ifndef TLK_PWM_REGISTERS_H_
#define TLK_PWM_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned pwm1_5_en : 5;
            unsigned reserved1 : 2;
        } __attribute__((packed)) en_bit;
        uint8_t en; // address: 0x80140400, offset: 0x0
    };
    union {
        struct {
            unsigned pwm0_en : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) en0_bit;
        uint8_t en0; // address: 0x80140401, offset: 0x1
    };
    uint8_t clkdiv; // address: 0x80140402, offset: 0x2
    union {
        struct {
            unsigned crun : 1;
            unsigned catch : 1;
            unsigned fifio_mode_en : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) mode_bit;
        uint8_t mode; // address: 0x80140403, offset: 0x3
    };
    uint8_t cc0[3]; // address: 0x80140404, offset: 0x4
    uint8_t mode32k; // address: 0x80140407, offset: 0x7
    uint8_t reserved0[0xc]; // address: 0x80140408, offset: 0x8
    struct {
        uint16_t compare; // address: 0x80140414, offset: 0x14
        uint16_t max; // address: 0x80140416, offset: 0x16
    } __attribute__((packed)) time_control[6]; // address: 0x80140414, offset: 0x14
    uint16_t pnum; // address: 0x8014042c, offset: 0x2c
    union {
        struct {
            unsigned center_align : 6;
            unsigned auto_txclr_off : 1;
            unsigned txf_nempty_en : 1;
        } __attribute__((packed)) center_bit;
        uint8_t center; // address: 0x8014042e, offset: 0x2e
    };
    uint8_t reserved1; // address: 0x8014042f, offset: 0x2f
    union {
        struct {
            unsigned mask_pwm : 1;
            unsigned mask_fifo : 1;
            unsigned mask : 6;
        } __attribute__((packed)) mask_bit;
        uint8_t mask; // address: 0x80140430, offset: 0x30
    };
    union {
        struct {
            unsigned pwm_irq : 1;
            unsigned fifo_done_irq : 1;
            unsigned irq_flag : 6;
        } __attribute__((packed)) irq_status_bit;
        uint8_t irq_status; // address: 0x80140431, offset: 0x31
    };
    uint8_t mask_lvl; // address: 0x80140432, offset: 0x32
    uint8_t irq_lvl; // address: 0x80140433, offset: 0x33
    uint16_t count[6]; // address: 0x80140434, offset: 0x34
    uint16_t n_count; // address: 0x80140440, offset: 0x40
    uint8_t reserved2[0x2]; // address: 0x80140442, offset: 0x42
    uint16_t tcmp_fsk; // address: 0x80140444, offset: 0x44
    uint16_t tmax_fsk; // address: 0x80140446, offset: 0x46
    uint16_t data[2]; // address: 0x80140448, offset: 0x48
    uint8_t fifo_level; // address: 0x8014044c, offset: 0x4c
    uint8_t tx_control; // address: 0x8014044d, offset: 0x4d
    uint8_t clear_txfifo; // address: 0x8014044e, offset: 0x4e
} tlk_pwm_reg_t;

#define TLK_PWM_BASE_ADDR (0x80140400U)
#define tlk_pwm_reg (*(volatile tlk_pwm_reg_t *) TLK_PWM_BASE_ADDR)

#endif

