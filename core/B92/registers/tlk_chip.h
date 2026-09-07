#ifndef TLK_TLK_CHIP_REGISTERS_H_
#define TLK_TLK_CHIP_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_reset_reset0_e {
    TLK_RESET_RESET0_LSPI   = TLK_BIT(0),
    TLK_RESET_RESET0_I2C    = TLK_BIT(1),
    TLK_RESET_RESET0_UART0  = TLK_BIT(2),
    TLK_RESET_RESET0_USB    = TLK_BIT(3),
    TLK_RESET_RESET0_PWM    = TLK_BIT(4),
    TLK_RESET_RESET0_UART1  = TLK_BIT(6),
    TLK_RESET_RESET0_SWIRES = TLK_BIT(7),
} tlk_reset_reset0_e;

typedef enum tlk_reset_reset1_e {
    TLK_RESET_RESET1_STIMER = TLK_BIT(1),
    TLK_RESET_RESET1_DMA    = TLK_BIT(2),
    TLK_RESET_RESET1_ALGM   = TLK_BIT(3),
    TLK_RESET_RESET1_PKE    = TLK_BIT(4),
    TLK_RESET_RESET1_GSPI   = TLK_BIT(6),
    TLK_RESET_RESET1_SPISLV = TLK_BIT(7),
} tlk_reset_reset1_e;

typedef enum tlk_reset_reset2_e {
    TLK_RESET_RESET2_TIMER       = TLK_BIT(0),
    TLK_RESET_RESET2_AUDIO       = TLK_BIT(1),
    TLK_RESET_RESET2_I2C1        = TLK_BIT(2),
    TLK_RESET_RESET2_MCU_RST_DIS = TLK_BIT(3),
    TLK_RESET_RESET2_MCU_RST_EN  = TLK_BIT(4),
    TLK_RESET_RESET2_LM          = TLK_BIT(5),
    TLK_RESET_RESET2_TRNG        = TLK_BIT(6),
    TLK_RESET_RESET2_DPR         = TLK_BIT(7),
} tlk_reset_reset2_e;

typedef enum tlk_reset_reset3_e {
    TLK_RESET_RESET3_ZB        = TLK_BIT(0),
    TLK_RESET_RESET3_ZB_MSTCLK = TLK_BIT(1),
    TLK_RESET_RESET3_ZB_LPCLK  = TLK_BIT(2),
    TLK_RESET_RESET3_ZB_CRYPT  = TLK_BIT(3),
    TLK_RESET_RESET3_MSPI      = TLK_BIT(4),
    TLK_RESET_RESET3_QDEC      = TLK_BIT(5),
    TLK_RESET_RESET3_SARADC    = TLK_BIT(6),
    TLK_RESET_RESET3_ALG       = TLK_BIT(7),
} tlk_reset_reset3_e;

typedef enum tlk_reset_pwdnen_e {
    TLK_RESET_PWDNEN_SUSPEND_EN       = TLK_BIT(0),
    TLK_RESET_PWDNEN_RAMCRC_CLREN_TGL = TLK_BIT(4),
    TLK_RESET_PWDNEN_RST_ALL          = TLK_BIT(5),
    TLK_RESET_PWDNEN_STALL_EN_TRG     = TLK_BIT(7),
} tlk_reset_pwdnen_e;

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
        struct {
            enum tlk_reset_reset0_e __attribute__((packed)) reset0 : 8;
        }; // address: 0x801401e0, offset: 0x20
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
        struct {
            enum tlk_reset_reset1_e __attribute__((packed)) reset1 : 8;
        }; // address: 0x801401e1, offset: 0x21
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
        struct {
            enum tlk_reset_reset2_e __attribute__((packed)) reset2 : 8;
        }; // address: 0x801401e2, offset: 0x22
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
        struct {
            enum tlk_reset_reset3_e __attribute__((packed)) reset3 : 8;
        }; // address: 0x801401e3, offset: 0x23
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
        struct {
            enum tlk_reset_pwdnen_e __attribute__((packed)) pwdnen : 8;
        }; // address: 0x801401ef, offset: 0x2f
    };
} tlk_reset_reg_t;

#define TLK_RESET_BASE_ADDR (0x801401c0U)
#define tlk_reset_reg (*(volatile tlk_reset_reg_t *) TLK_RESET_BASE_ADDR)

#endif