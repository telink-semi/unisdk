#ifndef TLK_TLK_CHIP_REGISTERS_H_
#define TLK_TLK_CHIP_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_reset_reset0_e {
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
    TLK_RESET_RESET2_TIMER              = TLK_BIT(0),
    TLK_RESET_RESET2_AUDIO              = TLK_BIT(1),
    TLK_RESET_RESET2_I2C1               = TLK_BIT(2),
    TLK_RESET_RESET2_MCU_RST            = TLK_BIT(3),
    TLK_RESET_RESET2_MCU_DURING_SUSPEND = TLK_BIT(4),
    TLK_RESET_RESET2_LM                 = TLK_BIT(5),
    TLK_RESET_RESET2_TRNG               = TLK_BIT(6),
    TLK_RESET_RESET2_DPR                = TLK_BIT(7),
} tlk_reset_reset2_e;

typedef enum tlk_reset_reset3_e {
    TLK_RESET_RESET3_TRACE  = TLK_BIT(1),
    TLK_RESET_RESET3_BROM   = TLK_BIT(2),
    TLK_RESET_RESET3_MSPI   = TLK_BIT(4),
    TLK_RESET_RESET3_QDEC   = TLK_BIT(5),
    TLK_RESET_RESET3_SARADC = TLK_BIT(6),
    TLK_RESET_RESET3_ALG    = TLK_BIT(7),
} tlk_reset_reset3_e;

typedef enum tlk_reset_pwdnen_e {
    TLK_RESET_PWDNEN_SUSPEND_EN       = TLK_BIT(0),
    TLK_RESET_PWDNEN_RAMCRC_CLREN_TGL = TLK_BIT(4),
    TLK_RESET_PWDNEN_RST_ALL          = TLK_BIT(5),
    TLK_RESET_PWDNEN_STALL_EN_TRG     = TLK_BIT(7),
} tlk_reset_pwdnen_e;

typedef enum tlk_reset_t_reset4_e {
    TLK_RESET_T_RESET4_SKE  = TLK_BIT(4),
    TLK_RESET_T_RESET4_HASH = TLK_BIT(5),
    TLK_RESET_T_RESET4_ZB   = TLK_BIT(7),
} tlk_reset_t_reset4_e;

typedef enum tlk_reset_t_reset5_e {
    TLK_RESET_T_RESET5_UART2    = TLK_BIT(1),
    TLK_RESET_T_RESET5_IR_LEARN = TLK_BIT(4),
    TLK_RESET_T_RESET5_KEY_SCAN = TLK_BIT(5),
    TLK_RESET_T_RESET5_PEM      = TLK_BIT(6),
} tlk_reset_t_reset5_e;

typedef enum tlk_reset_t_reset6_e {
    TLK_RESET_T_RESET6_RZ = TLK_BIT(0),
} tlk_reset_t_reset6_e;

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
        struct {
            enum tlk_reset_reset0_e __attribute__((packed)) reset0 : 8;
        }; // address: 0x80140820, offset: 0x20
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
        }; // address: 0x80140821, offset: 0x21
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
        struct {
            enum tlk_reset_reset2_e __attribute__((packed)) reset2 : 8;
        }; // address: 0x80140822, offset: 0x22
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
        struct {
            enum tlk_reset_reset3_e __attribute__((packed)) reset3 : 8;
        }; // address: 0x80140823, offset: 0x23
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
        struct {
            enum tlk_reset_pwdnen_e __attribute__((packed)) pwdnen : 8;
        }; // address: 0x8014082f, offset: 0x2f
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
        struct {
            enum tlk_reset_t_reset4_e __attribute__((packed)) t_reset4 : 8;
        }; // address: 0x80140840, offset: 0x40
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
        struct {
            enum tlk_reset_t_reset5_e __attribute__((packed)) t_reset5 : 8;
        }; // address: 0x80140841, offset: 0x41
    };
    union {
        struct {
            unsigned rz : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) t_reset6_bit;
        struct {
            enum tlk_reset_t_reset6_e __attribute__((packed)) t_reset6 : 8;
        }; // address: 0x80140842, offset: 0x42
    };
} tlk_reset_reg_t;

#define TLK_RESET_BASE_ADDR (0x80140800U)
#define tlk_reset_reg (*(volatile tlk_reset_reg_t *) TLK_RESET_BASE_ADDR)

#endif