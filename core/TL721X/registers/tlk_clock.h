#ifndef TLK_TLK_CLOCK_REGISTERS_H_
#define TLK_TLK_CLOCK_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_clock_mspi_mode_mspi_div_in_sel_e {
    TLK_CLOCK_MSPI_MODE_MSPI_DIV_IN_SEL_24M_RC   = 0x0U,
    TLK_CLOCK_MSPI_MODE_MSPI_DIV_IN_SEL_24M_XTAL = 0x1U,
    TLK_CLOCK_MSPI_MODE_MSPI_DIV_IN_SEL_PLL      = 0x2U,
} tlk_clock_mspi_mode_mspi_div_in_sel_e;

typedef enum tlk_clock_lspi_mode_lspi_div_in_sel_e {
    TLK_CLOCK_LSPI_MODE_LSPI_DIV_IN_SEL_24M_RC   = 0x0U,
    TLK_CLOCK_LSPI_MODE_LSPI_DIV_IN_SEL_24M_XTAL = 0x1U,
    TLK_CLOCK_LSPI_MODE_LSPI_DIV_IN_SEL_PLL      = 0x2U,
} tlk_clock_lspi_mode_lspi_div_in_sel_e;

typedef enum tlk_clock_gspi_mode_gspi_div_in_sel_e {
    TLK_CLOCK_GSPI_MODE_GSPI_DIV_IN_SEL_24M_RC   = 0x0U,
    TLK_CLOCK_GSPI_MODE_GSPI_DIV_IN_SEL_24M_XTAL = 0x1U,
    TLK_CLOCK_GSPI_MODE_GSPI_DIV_IN_SEL_PLL      = 0x2U,
} tlk_clock_gspi_mode_gspi_div_in_sel_e;

typedef enum tlk_clock_i2s0_step_e {
    TLK_CLOCK_I2S0_STEP_CLK_EN = TLK_BIT(15),
} tlk_clock_i2s0_step_e;

typedef enum tlk_clock_i2s2_step_e {
    TLK_CLOCK_I2S2_STEP_CLK_EN = TLK_BIT(15),
} tlk_clock_i2s2_step_e;

typedef enum tlk_clock_cache_init_e {
    TLK_CLOCK_CACHE_INIT_ICACHE_DISABLE_INIT = TLK_BIT(6),
    TLK_CLOCK_CACHE_INIT_DCACHE_DISABLE_INIT = TLK_BIT(7),
} tlk_clock_cache_init_e;

typedef enum tlk_clock_mcu_ctrl0_e {
    TLK_CLOCK_MCU_CTRL0_MCU_REBOOT = TLK_BIT(7),
} tlk_clock_mcu_ctrl0_e;

typedef enum tlk_clock_i2s1_step_e {
    TLK_CLOCK_I2S1_STEP_CLK_EN = TLK_BIT(15),
} tlk_clock_i2s1_step_e;

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
    TLK_CLOCK_CLOCK_EN1_STIMER = TLK_BIT(1),
    TLK_CLOCK_CLOCK_EN1_DMA    = TLK_BIT(2),
    TLK_CLOCK_CLOCK_EN1_ALGM   = TLK_BIT(3),
    TLK_CLOCK_CLOCK_EN1_PKE    = TLK_BIT(4),
    TLK_CLOCK_CLOCK_EN1_PLMT   = TLK_BIT(5),
    TLK_CLOCK_CLOCK_EN1_GSPI   = TLK_BIT(6),
    TLK_CLOCK_CLOCK_EN1_SPISLV = TLK_BIT(7),
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
    TLK_CLOCK_CLOCK_EN3_TRACE      = TLK_BIT(1),
    TLK_CLOCK_CLOCK_EN3_BROM       = TLK_BIT(2),
    TLK_CLOCK_CLOCK_EN3_MSPI       = TLK_BIT(4),
    TLK_CLOCK_CLOCK_EN3_QDEC       = TLK_BIT(5),
    TLK_CLOCK_CLOCK_EN3_SARADC_DIG = TLK_BIT(6),
} tlk_clock_clock_en3_e;

typedef enum tlk_clock_dmic_step_e {
    TLK_CLOCK_DMIC_STEP_CLK_SEL = TLK_BIT(15),
} tlk_clock_dmic_step_e;

typedef enum tlk_clock_wakeupen_e {
    TLK_CLOCK_WAKEUPEN_USB_PWDN_I    = TLK_BIT(0),
    TLK_CLOCK_WAKEUPEN_GPIO_WAKEUP_I = TLK_BIT(1),
    TLK_CLOCK_WAKEUPEN_QDEC_WAKEUP_I = TLK_BIT(2),
    TLK_CLOCK_WAKEUPEN_USB_RESUME    = TLK_BIT(4),
    TLK_CLOCK_WAKEUPEN_STANDBY_EX    = TLK_BIT(5),
} tlk_clock_wakeupen_e;

typedef enum tlk_clock_pwdnen_e {
    TLK_CLOCK_PWDNEN_SUSPEND_EN       = TLK_BIT(0),
    TLK_CLOCK_PWDNEN_RAMCRC_CLREN_TGL = TLK_BIT(4),
    TLK_CLOCK_PWDNEN_RST_ALL          = TLK_BIT(5),
    TLK_CLOCK_PWDNEN_STALL_EN_TRG     = TLK_BIT(7),
} tlk_clock_pwdnen_e;

typedef enum tlk_clock_clk_div_e {
    TLK_CLOCK_CLK_DIV_R_7816_CLK_EN = TLK_BIT(7),
} tlk_clock_clk_div_e;

typedef enum tlk_clock_ram_crc_e {
    TLK_CLOCK_RAM_CRC_RAM_CRC_ERR         = TLK_BIT(0),
    TLK_CLOCK_RAM_CRC_WATCHDOG_RST_STATUS = TLK_BIT(2),
    TLK_CLOCK_RAM_CRC_JTAG_RST_STATUS     = TLK_BIT(3),
} tlk_clock_ram_crc_e;

typedef enum tlk_clock_jtag_sel_e {
    TLK_CLOCK_JTAG_SEL_VALUE = TLK_BIT(0),
} tlk_clock_jtag_sel_e;

typedef enum tlk_clock_t_clock_en4_e {
    TLK_CLOCK_T_CLOCK_EN4_SKE  = TLK_BIT(4),
    TLK_CLOCK_T_CLOCK_EN4_HASH = TLK_BIT(5),
    TLK_CLOCK_T_CLOCK_EN4_CCLK = TLK_BIT(6),
    TLK_CLOCK_T_CLOCK_EN4_ZB   = TLK_BIT(7),
} tlk_clock_t_clock_en4_e;

typedef enum tlk_clock_t_clock_en5_e {
    TLK_CLOCK_T_CLOCK_EN5_UART2    = TLK_BIT(1),
    TLK_CLOCK_T_CLOCK_EN5_IR_LEARN = TLK_BIT(4),
    TLK_CLOCK_T_CLOCK_EN5_PEM      = TLK_BIT(6),
    TLK_CLOCK_T_CLOCK_EN5_CHACHA20 = TLK_BIT(7),
} tlk_clock_t_clock_en5_e;

typedef enum tlk_clock_t_clock_en6_e {
    TLK_CLOCK_T_CLOCK_EN6_RZ = TLK_BIT(0),
} tlk_clock_t_clock_en6_e;

typedef enum tlk_clock_t_clock_en7_e {
    TLK_CLOCK_T_CLOCK_EN7_RZ = TLK_BIT(0),
} tlk_clock_t_clock_en7_e;

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned mspi_mod : 4;
            enum tlk_clock_mspi_mode_mspi_div_in_sel_e __attribute__((packed)) mspi_div_in_sel : 2;
            unsigned reserved : 2;
        } __attribute__((packed)) mspi_mode_bit;
        uint8_t mspi_mode; // address: 0x80140800, offset: 0x0
    };
    union {
        struct {
            unsigned lspi_mod : 4;
            enum tlk_clock_lspi_mode_lspi_div_in_sel_e __attribute__((packed)) lspi_div_in_sel : 2;
            unsigned reserved : 2;
        } __attribute__((packed)) lspi_mode_bit;
        uint8_t lspi_mode; // address: 0x80140801, offset: 0x1
    };
    union {
        struct {
            unsigned gspi_mod : 8;
            enum tlk_clock_gspi_mode_gspi_div_in_sel_e __attribute__((packed)) gspi_div_in_sel : 2;
            unsigned reserved : 6;
        } __attribute__((packed)) gspi_mode_bit;
        uint16_t gspi_mode; // address: 0x80140802, offset: 0x2
    };
    uint8_t reserved0[0x2]; // address: 0x80140804, offset: 0x4
    union {
        struct {
            unsigned low : 8;
            unsigned high : 7;
            unsigned clk_en : 1;
        } __attribute__((packed)) i2s0_step_bit;
        struct {
            enum tlk_clock_i2s0_step_e __attribute__((packed)) i2s0_step : 16;
        }; // address: 0x80140806, offset: 0x6
    };
    union {
        struct {
            unsigned low : 8;
            unsigned high : 7;
            unsigned clk_en : 1;
        } __attribute__((packed)) i2s2_step_bit;
        struct {
            enum tlk_clock_i2s2_step_e __attribute__((packed)) i2s2_step : 16;
        }; // address: 0x80140808, offset: 0x8
    };
    uint16_t i2s2_mod; // address: 0x8014080a, offset: 0xa
    uint8_t reserved1[0x4]; // address: 0x8014080c, offset: 0xc
    union {
        struct {
            unsigned reserved : 6;
            unsigned icache_disable_init : 1;
            unsigned dcache_disable_init : 1;
        } __attribute__((packed)) cache_init_bit;
        struct {
            enum tlk_clock_cache_init_e __attribute__((packed)) cache_init : 8;
        }; // address: 0x80140810, offset: 0x10
    };
    uint8_t eoc_ts_value; // address: 0x80140811, offset: 0x11
    uint8_t reserved2[0x2]; // address: 0x80140812, offset: 0x12
    union {
        struct {
            unsigned reserved : 7;
            unsigned mcu_reboot : 1;
        } __attribute__((packed)) mcu_ctrl0_bit;
        struct {
            enum tlk_clock_mcu_ctrl0_e __attribute__((packed)) mcu_ctrl0 : 8;
        }; // address: 0x80140814, offset: 0x14
    };
    uint8_t mcu_reset_vector[3]; // address: 0x80140815, offset: 0x15
    union {
        struct {
            unsigned value : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) busclk_ratio_bit;
        uint8_t busclk_ratio; // address: 0x80140818, offset: 0x18
    };
    uint8_t reserved3; // address: 0x80140819, offset: 0x19
    union {
        struct {
            unsigned value : 5;
            unsigned reserved : 3;
        } __attribute__((packed)) probe_clk_sel_bit;
        uint8_t probe_clk_sel; // address: 0x8014081a, offset: 0x1a
    };
    uint8_t reserved4; // address: 0x8014081b, offset: 0x1b
    union {
        struct {
            unsigned low : 8;
            unsigned high : 7;
            unsigned clk_en : 1;
        } __attribute__((packed)) i2s1_step_bit;
        struct {
            enum tlk_clock_i2s1_step_e __attribute__((packed)) i2s1_step : 16;
        }; // address: 0x8014081c, offset: 0x1c
    };
    uint16_t i2s1_mod; // address: 0x8014081e, offset: 0x1e
    uint8_t reserved5[0x4]; // address: 0x80140820, offset: 0x20
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
        }; // address: 0x80140824, offset: 0x24
    };
    union {
        struct {
            unsigned reserved : 1;
            unsigned stimer : 1;
            unsigned dma : 1;
            unsigned algm : 1;
            unsigned pke : 1;
            unsigned plmt : 1;
            unsigned gspi : 1;
            unsigned spislv : 1;
        } __attribute__((packed)) clock_en1_bit;
        struct {
            enum tlk_clock_clock_en1_e __attribute__((packed)) clock_en1 : 8;
        }; // address: 0x80140825, offset: 0x25
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
        }; // address: 0x80140826, offset: 0x26
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned trace : 1;
            unsigned brom : 1;
            unsigned reserved1 : 1;
            unsigned mspi : 1;
            unsigned qdec : 1;
            unsigned saradc_dig : 1;
            unsigned reserved2 : 1;
        } __attribute__((packed)) clock_en3_bit;
        struct {
            enum tlk_clock_clock_en3_e __attribute__((packed)) clock_en3 : 8;
        }; // address: 0x80140827, offset: 0x27
    };
    union {
        struct {
            unsigned cclk_div : 4;
            unsigned cclk_sel : 2;
            unsigned reserved : 2;
        } __attribute__((packed)) cclk_set_bit;
        uint8_t cclk_set; // address: 0x80140828, offset: 0x28
    };
    uint8_t reserved6; // address: 0x80140829, offset: 0x29
    uint16_t i2s0_mod; // address: 0x8014082a, offset: 0x2a
    union {
        struct {
            unsigned low : 8;
            unsigned high : 7;
            unsigned clk_sel : 1;
        } __attribute__((packed)) dmic_step_bit;
        struct {
            enum tlk_clock_dmic_step_e __attribute__((packed)) dmic_step : 16;
        }; // address: 0x8014082c, offset: 0x2c
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
        }; // address: 0x8014082e, offset: 0x2e
    };
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
            enum tlk_clock_pwdnen_e __attribute__((packed)) pwdnen : 8;
        }; // address: 0x8014082f, offset: 0x2f
    };
    union {
        struct {
            unsigned reserved : 4;
            unsigned r_7816_mod : 3;
            unsigned r_7816_clk_en : 1;
        } __attribute__((packed)) clk_div_bit;
        struct {
            enum tlk_clock_clk_div_e __attribute__((packed)) clk_div : 8;
        }; // address: 0x80140830, offset: 0x30
    };
    uint8_t reserved7; // address: 0x80140831, offset: 0x31
    union {
        struct {
            unsigned ram_crc_err : 1;
            unsigned reserved : 1;
            unsigned watchdog_rst_status : 1;
            unsigned jtag_rst_status : 1;
            unsigned reserved2 : 4;
        } __attribute__((packed)) ram_crc_bit;
        struct {
            enum tlk_clock_ram_crc_e __attribute__((packed)) ram_crc : 8;
        }; // address: 0x80140832, offset: 0x32
    };
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) jtag_sel_bit;
        struct {
            enum tlk_clock_jtag_sel_e __attribute__((packed)) jtag_sel : 8;
        }; // address: 0x80140833, offset: 0x33
    };
    uint8_t reserved8[0x2]; // address: 0x80140834, offset: 0x34
    uint16_t dmic_mod; // address: 0x80140836, offset: 0x36
    uint8_t reserved9[0x3]; // address: 0x80140838, offset: 0x38
    union {
        struct {
            unsigned value : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) usb_div_bit;
        uint8_t usb_div; // address: 0x8014083b, offset: 0x3b
    };
    uint8_t reserved10[0x8]; // address: 0x8014083c, offset: 0x3c
    union {
        struct {
            unsigned reserved : 4;
            unsigned ske : 1;
            unsigned hash : 1;
            unsigned cclk : 1;
            unsigned zb : 1;
        } __attribute__((packed)) t_clock_en4_bit;
        struct {
            enum tlk_clock_t_clock_en4_e __attribute__((packed)) t_clock_en4 : 8;
        }; // address: 0x80140844, offset: 0x44
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned uart2 : 1;
            unsigned reserved1 : 2;
            unsigned ir_learn : 1;
            unsigned reserved2 : 1;
            unsigned pem : 1;
            unsigned chacha20 : 1;
        } __attribute__((packed)) t_clock_en5_bit;
        struct {
            enum tlk_clock_t_clock_en5_e __attribute__((packed)) t_clock_en5 : 8;
        }; // address: 0x80140845, offset: 0x45
    };
    union {
        struct {
            unsigned rz : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) t_clock_en6_bit;
        struct {
            enum tlk_clock_t_clock_en6_e __attribute__((packed)) t_clock_en6 : 8;
        }; // address: 0x80140846, offset: 0x46
    };
    union {
        struct {
            unsigned rz : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) t_clock_en7_bit;
        struct {
            enum tlk_clock_t_clock_en7_e __attribute__((packed)) t_clock_en7 : 8;
        }; // address: 0x80140847, offset: 0x47
    };
} tlk_clock_reg_t;

#define TLK_CLOCK_BASE_ADDR (0x80140800U)
#define tlk_clock_reg (*(volatile tlk_clock_reg_t *) TLK_CLOCK_BASE_ADDR)

#endif