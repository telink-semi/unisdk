#ifndef TLK_CLOCK_REGISTERS_H_
#define TLK_CLOCK_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x12]; // address: 0x801401c0, offset: 0x0
    union {
        struct {
            unsigned zb_mst_mod : 4;
            unsigned reserved : 4;
        } __attribute__((packed)) clkmod_bit;
        uint8_t clkmod; // address: 0x801401d2, offset: 0x12
    };
    uint8_t reserved1[0x11]; // address: 0x801401d3, offset: 0x13
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
        } __attribute__((packed)) clock_en0_bit;
        uint8_t clock_en0; // address: 0x801401e4, offset: 0x24
    };
    union {
        struct {
            unsigned reserved : 1;
            unsigned stimer : 1;
            unsigned dma : 1;
            unsigned algm : 1;
            unsigned pke : 1;
            unsigned machinetime : 1;
            unsigned gspi : 1;
            unsigned spislv : 1;
        } __attribute__((packed)) clock_en1_bit;
        uint8_t clock_en1; // address: 0x801401e5, offset: 0x25
    };
    union {
        struct {
            unsigned timer : 1;
            unsigned audio : 1;
            unsigned i2c1 : 1;
            unsigned reserved : 1;
            unsigned mcu : 1;
            unsigned lm : 1;
            unsigned trng : 1;
            unsigned dpr : 1;
        } __attribute__((packed)) clock_en2_bit;
        uint8_t clock_en2; // address: 0x801401e6, offset: 0x26
    };
    union {
        struct {
            unsigned zb_pclk : 1;
            unsigned zb_mstclk : 1;
            unsigned zb_lpclk : 1;
            unsigned reserved0 : 1;
            unsigned mspi : 1;
            unsigned qdec : 1;
            unsigned reserved1 : 1;
            unsigned reserved2 : 1;
        } __attribute__((packed)) clock_en3_bit;
        uint8_t clock_en3; // address: 0x801401e7, offset: 0x27
    };
    union {
        struct {
            unsigned sclk_div : 4;
            unsigned sclk_sel : 3;
            unsigned reserved : 1;
        } __attribute__((packed)) clock_sel0_bit;
        uint8_t clock_sel0; // address: 0x801401e8, offset: 0x28
    };
    uint8_t reserved2; // address: 0x801401e9, offset: 0x29
    union {
        struct {
            unsigned low : 8;
            unsigned high : 8;
        } __attribute__((packed)) i2s0_mod_bit;
        uint16_t i2s0_mod; // address: 0x801401ea, offset: 0x2a
    };
    union {
        struct {
            unsigned low : 8;
            unsigned high : 7;
            unsigned dmic_clk_sel : 1;
        } __attribute__((packed)) dmic_step_bit;
        uint16_t dmic_step; // address: 0x801401ec, offset: 0x2c
    };
    union {
        struct {
            unsigned usb_pwdn_i : 1;
            unsigned gpio_wakeup_i : 1;
            unsigned qdec_wakeup_i : 1;
            unsigned reserved0 : 1;
            unsigned usb_resume : 1;
            unsigned standby_ex : 1;
            unsigned reserved1 : 2;
        } __attribute__((packed)) wakeupen_bit;
        uint8_t wakeupen; // address: 0x801401ee, offset: 0x2e
    };
    union {
        struct {
            unsigned suspend_en_o : 1;
            unsigned reserved0 : 3;
            unsigned ramcrc_clren_tgl : 1;
            unsigned rst_all : 1;
            unsigned reserved1 : 1;
            unsigned stall_en_trg : 1;
        } __attribute__((packed)) pwdnen_bit;
        uint8_t pwdnen; // address: 0x801401ef, offset: 0x2f
    };
    union {
        struct {
            unsigned clkzb32k_sel : 3;
            unsigned reserved : 1;
            unsigned r_7816_mod : 3;
            unsigned r_7816_clk_en : 1;
        } __attribute__((packed)) clk_div_bit;
        uint8_t clk_div; // address: 0x801401f0, offset: 0x30
    };
} tlk_clock_reg_t;

#define TLK_CLOCK_BASE_ADDR (0x801401c0U)
#define tlk_clock_reg (*(volatile tlk_clock_reg_t *) TLK_CLOCK_BASE_ADDR)

#endif

