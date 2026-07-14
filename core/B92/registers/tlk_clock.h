#ifndef TLK_TLK_CLOCK_REGISTERS_H_
#define TLK_TLK_CLOCK_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_clock_mspi_clk_set_e {
    TLK_CLOCK_MSPI_CLK_SET_CLKDIV_HS_RC         = TLK_BIT(6),
    TLK_CLOCK_MSPI_CLK_SET_CLKDIV_RESET_RELEASE = TLK_BIT(7),
} tlk_clock_mspi_clk_set_e;

typedef enum tlk_clock_lspi_clk_set_e {
    TLK_CLOCK_LSPI_CLK_SET_CLKDIV_HS_RC         = TLK_BIT(6),
    TLK_CLOCK_LSPI_CLK_SET_CLKDIV_RESET_RELEASE = TLK_BIT(7),
} tlk_clock_lspi_clk_set_e;

typedef enum tlk_clock_gspi_clk_set_e {
    TLK_CLOCK_GSPI_CLK_SET_CLKDIV_HS_RC         = TLK_BIT(14),
    TLK_CLOCK_GSPI_CLK_SET_CLKDIV_RESET_RELEASE = TLK_BIT(15),
} tlk_clock_gspi_clk_set_e;

typedef enum tlk_clock_hclk_pclk_e {
    TLK_CLOCK_HCLK_PCLK_HCLK_DIV = TLK_BIT(2),
} tlk_clock_hclk_pclk_e;

typedef enum tlk_clock_clock_en0_e {
    TLK_CLOCK_CLOCK_EN0_LSPI   = TLK_BIT(0),
    TLK_CLOCK_CLOCK_EN0_I2C    = TLK_BIT(1),
    TLK_CLOCK_CLOCK_EN0_UART0  = TLK_BIT(2),
    TLK_CLOCK_CLOCK_EN0_USB    = TLK_BIT(3),
    TLK_CLOCK_CLOCK_EN0_PWM    = TLK_BIT(4),
    TLK_CLOCK_CLOCK_EN0_UART1  = TLK_BIT(6),
    TLK_CLOCK_CLOCK_EN0_SWIRES = TLK_BIT(7),
} tlk_clock_clock_en0_e;

typedef enum tlk_clock_clock_en1_e {
    TLK_CLOCK_CLOCK_EN1_STIMER      = TLK_BIT(1),
    TLK_CLOCK_CLOCK_EN1_DMA         = TLK_BIT(2),
    TLK_CLOCK_CLOCK_EN1_ALGM        = TLK_BIT(3),
    TLK_CLOCK_CLOCK_EN1_PKE         = TLK_BIT(4),
    TLK_CLOCK_CLOCK_EN1_MACHINETIME = TLK_BIT(5),
    TLK_CLOCK_CLOCK_EN1_GSPI        = TLK_BIT(6),
    TLK_CLOCK_CLOCK_EN1_SPISLV      = TLK_BIT(7),
} tlk_clock_clock_en1_e;

typedef enum tlk_clock_clock_en2_e {
    TLK_CLOCK_CLOCK_EN2_TIMER = TLK_BIT(0),
    TLK_CLOCK_CLOCK_EN2_AUDIO = TLK_BIT(1),
    TLK_CLOCK_CLOCK_EN2_I2C1  = TLK_BIT(2),
    TLK_CLOCK_CLOCK_EN2_MCU   = TLK_BIT(4),
    TLK_CLOCK_CLOCK_EN2_LM    = TLK_BIT(5),
    TLK_CLOCK_CLOCK_EN2_TRNG  = TLK_BIT(6),
    TLK_CLOCK_CLOCK_EN2_DPR   = TLK_BIT(7),
} tlk_clock_clock_en2_e;

typedef enum tlk_clock_clock_en3_e {
    TLK_CLOCK_CLOCK_EN3_ZB_PCLK   = TLK_BIT(0),
    TLK_CLOCK_CLOCK_EN3_ZB_MSTCLK = TLK_BIT(1),
    TLK_CLOCK_CLOCK_EN3_ZB_LPCLK  = TLK_BIT(2),
    TLK_CLOCK_CLOCK_EN3_MSPI      = TLK_BIT(4),
    TLK_CLOCK_CLOCK_EN3_QDEC      = TLK_BIT(5),
} tlk_clock_clock_en3_e;

typedef enum tlk_clock_dmic_step_e {
    TLK_CLOCK_DMIC_STEP_DMIC_CLK_SEL = TLK_BIT(15),
} tlk_clock_dmic_step_e;

typedef enum tlk_clock_wakeupen_e {
    TLK_CLOCK_WAKEUPEN_USB_PWDN_I    = TLK_BIT(0),
    TLK_CLOCK_WAKEUPEN_GPIO_WAKEUP_I = TLK_BIT(1),
    TLK_CLOCK_WAKEUPEN_QDEC_WAKEUP_I = TLK_BIT(2),
    TLK_CLOCK_WAKEUPEN_USB_RESUME    = TLK_BIT(4),
    TLK_CLOCK_WAKEUPEN_STANDBY_EX    = TLK_BIT(5),
} tlk_clock_wakeupen_e;

typedef enum tlk_clock_pwdnen_e {
    TLK_CLOCK_PWDNEN_SUSPEND_EN_O     = TLK_BIT(0),
    TLK_CLOCK_PWDNEN_RAMCRC_CLREN_TGL = TLK_BIT(4),
    TLK_CLOCK_PWDNEN_RST_ALL          = TLK_BIT(5),
    TLK_CLOCK_PWDNEN_STALL_EN_TRG     = TLK_BIT(7),
} tlk_clock_pwdnen_e;

typedef enum tlk_clock_clk_div_e {
    TLK_CLOCK_CLK_DIV_R_7816_CLK_EN = TLK_BIT(7),
} tlk_clock_clk_div_e;

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned clkmod : 6;
            unsigned clkdiv_hs_rc : 1;
            unsigned clkdiv_reset_release : 1;
        } __attribute__((packed)) mspi_clk_set_bit;
        struct {
            enum tlk_clock_mspi_clk_set_e __attribute__((packed)) mspi_clk_set : 8;
        }; // address: 0x801401c0, offset: 0x0
    };
    union {
        struct {
            unsigned clkmod : 6;
            unsigned clkdiv_hs_rc : 1;
            unsigned clkdiv_reset_release : 1;
        } __attribute__((packed)) lspi_clk_set_bit;
        struct {
            enum tlk_clock_lspi_clk_set_e __attribute__((packed)) lspi_clk_set : 8;
        }; // address: 0x801401c1, offset: 0x1
    };
    union {
        struct {
            unsigned clkmod : 8;
            unsigned reserved : 6;
            unsigned clkdiv_hs_rc : 1;
            unsigned clkdiv_reset_release : 1;
        } __attribute__((packed)) gspi_clk_set_bit;
        struct {
            enum tlk_clock_gspi_clk_set_e __attribute__((packed)) gspi_clk_set : 16;
        }; // address: 0x801401c2, offset: 0x2
    };
    uint8_t reserved0[0xe]; // address: 0x801401c4, offset: 0x4
    union {
        struct {
            unsigned zb_mst_mod : 4;
            unsigned reserved : 4;
        } __attribute__((packed)) clkmod_bit;
        uint8_t clkmod; // address: 0x801401d2, offset: 0x12
    };
    uint8_t reserved1[0x5]; // address: 0x801401d3, offset: 0x13
    union {
        struct {
            unsigned pclk_div : 2;
            unsigned hclk_div : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) hclk_pclk_bit;
        struct {
            enum tlk_clock_hclk_pclk_e __attribute__((packed)) hclk_pclk : 8;
        }; // address: 0x801401d8, offset: 0x18
    };
    uint8_t reserved2[0xb]; // address: 0x801401d9, offset: 0x19
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
        struct {
            enum tlk_clock_clock_en0_e __attribute__((packed)) clock_en0 : 8;
        }; // address: 0x801401e4, offset: 0x24
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
        struct {
            enum tlk_clock_clock_en1_e __attribute__((packed)) clock_en1 : 8;
        }; // address: 0x801401e5, offset: 0x25
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
        struct {
            enum tlk_clock_clock_en2_e __attribute__((packed)) clock_en2 : 8;
        }; // address: 0x801401e6, offset: 0x26
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
        struct {
            enum tlk_clock_clock_en3_e __attribute__((packed)) clock_en3 : 8;
        }; // address: 0x801401e7, offset: 0x27
    };
    union {
        struct {
            unsigned sclk_div : 4;
            unsigned sclk_sel : 3;
            unsigned reserved : 1;
        } __attribute__((packed)) clock_sel0_bit;
        uint8_t clock_sel0; // address: 0x801401e8, offset: 0x28
    };
    uint8_t reserved3; // address: 0x801401e9, offset: 0x29
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
        struct {
            enum tlk_clock_dmic_step_e __attribute__((packed)) dmic_step : 16;
        }; // address: 0x801401ec, offset: 0x2c
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
        struct {
            enum tlk_clock_wakeupen_e __attribute__((packed)) wakeupen : 8;
        }; // address: 0x801401ee, offset: 0x2e
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
        struct {
            enum tlk_clock_pwdnen_e __attribute__((packed)) pwdnen : 8;
        }; // address: 0x801401ef, offset: 0x2f
    };
    union {
        struct {
            unsigned clkzb32k_sel : 3;
            unsigned reserved : 1;
            unsigned r_7816_mod : 3;
            unsigned r_7816_clk_en : 1;
        } __attribute__((packed)) clk_div_bit;
        struct {
            enum tlk_clock_clk_div_e __attribute__((packed)) clk_div : 8;
        }; // address: 0x801401f0, offset: 0x30
    };
    uint8_t reserved4[0xa]; // address: 0x801401f1, offset: 0x31
    uint8_t usb_clk; // address: 0x801401fb, offset: 0x3b
} tlk_clock_reg_t;

#define TLK_CLOCK_BASE_ADDR (0x801401c0U)
#define tlk_clock_reg (*(volatile tlk_clock_reg_t *) TLK_CLOCK_BASE_ADDR)

#endif