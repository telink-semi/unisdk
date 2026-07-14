#ifndef TLK_TLK_MSPI_REGISTERS_H_
#define TLK_TLK_MSPI_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_mspi_control0_e {
    TLK_MSPI_CONTROL0_RX_FIFO_IRQ_EN = TLK_BIT(2),
    TLK_MSPI_CONTROL0_TX_FIFO_IRQ_EN = TLK_BIT(3),
    TLK_MSPI_CONTROL0_END_IRQ_EN     = TLK_BIT(4),
    TLK_MSPI_CONTROL0_RX_DMA_EN      = TLK_BIT(6),
    TLK_MSPI_CONTROL0_TX_DMA_EN      = TLK_BIT(7),
} tlk_mspi_control0_e;

typedef enum tlk_mspi_control1_e {
    TLK_MSPI_CONTROL1_ADDR_FMT = TLK_BIT(4),
    TLK_MSPI_CONTROL1_ADDR_EN  = TLK_BIT(5),
    TLK_MSPI_CONTROL1_CMD_FMT  = TLK_BIT(6),
    TLK_MSPI_CONTROL1_CMD_EN   = TLK_BIT(7),
} tlk_mspi_control1_e;

typedef enum tlk_mspi_reg_control0_e {
    TLK_MSPI_REG_CONTROL0_CMD1_EN       = TLK_BIT(0),
    TLK_MSPI_REG_CONTROL0_TOKEN_VAL_SEL = TLK_BIT(1),
    TLK_MSPI_REG_CONTROL0_TOKEN_EN      = TLK_BIT(2),
    TLK_MSPI_REG_CONTROL0_DDR_MODE      = TLK_BIT(3),
} tlk_mspi_reg_control0_e;

typedef enum tlk_mspi_control3_e {
    TLK_MSPI_CONTROL3_SPI_LSB                   = TLK_BIT(0),
    TLK_MSPI_CONTROL3_SPI_3LINE                 = TLK_BIT(1),
    TLK_MSPI_CONTROL3_DMA_TX_SOF_CLR_TX_FIFO_EN = TLK_BIT(5),
    TLK_MSPI_CONTROL3_DMA_RX_EOF_CLR_RX_FIFO_EN = TLK_BIT(6),
    TLK_MSPI_CONTROL3_AUTO_HREADY_EN            = TLK_BIT(7),
} tlk_mspi_control3_e;

typedef enum tlk_mspi_pem_control0_e {
    TLK_MSPI_PEM_CONTROL0_EVENT_EN      = TLK_BIT(0),
    TLK_MSPI_PEM_CONTROL0_TASK_EN       = TLK_BIT(1),
    TLK_MSPI_PEM_CONTROL0_PEM_EVENT_SEL = TLK_BIT(2),
} tlk_mspi_pem_control0_e;

typedef enum tlk_mspi_control4_e {
    TLK_MSPI_CONTROL4_DMA_TRIG_SPI_EN      = TLK_BIT(0),
    TLK_MSPI_CONTROL4_TX_DMA_REQ_AFTER_CMD = TLK_BIT(1),
    TLK_MSPI_CONTROL4_XIP_STOP             = TLK_BIT(2),
    TLK_MSPI_CONTROL4_XIP_EN               = TLK_BIT(3),
    TLK_MSPI_CONTROL4_DUMMY_CNT_ADD        = TLK_BIT(4),
} tlk_mspi_control4_e;

typedef enum tlk_mspi_tx_fifo_status_e {
    TLK_MSPI_TX_FIFO_STATUS_TX_FIFO_FULL = TLK_BIT(7),
} tlk_mspi_tx_fifo_status_e;

typedef enum tlk_mspi_rx_fifo_status_e {
    TLK_MSPI_RX_FIFO_STATUS_RX_FIFO_EMPTY = TLK_BIT(7),
} tlk_mspi_rx_fifo_status_e;

typedef enum tlk_mspi_status_e {
    TLK_MSPI_STATUS_SPI_SOFT_RESET    = TLK_BIT(2),
    TLK_MSPI_STATUS_XIP_ARB_ERR       = TLK_BIT(3),
    TLK_MSPI_STATUS_RX_FIFO_CLR_LEVEL = TLK_BIT(4),
    TLK_MSPI_STATUS_TX_FIFO_CLR_LEVEL = TLK_BIT(5),
    TLK_MSPI_STATUS_BUSY              = TLK_BIT(7),
} tlk_mspi_status_e;

typedef enum tlk_mspi_irq_status0_e {
    TLK_MSPI_IRQ_STATUS0_RX_FIFO_THRES_IRQ_STATUS = TLK_BIT(2),
    TLK_MSPI_IRQ_STATUS0_TX_FIFO_THRES_IRQ_STATUS = TLK_BIT(3),
    TLK_MSPI_IRQ_STATUS0_TRANS_END_IRQ_STATUS     = TLK_BIT(4),
} tlk_mspi_irq_status0_e;

typedef enum tlk_mspi_cipher_control_e {
    TLK_MSPI_CIPHER_CONTROL_CIPHER_READ_EN  = TLK_BIT(0),
    TLK_MSPI_CIPHER_CONTROL_CIPHER_WRITE_EN = TLK_BIT(1),
} tlk_mspi_cipher_control_e;

typedef enum tlk_mspi_xip_read_format_e {
    TLK_MSPI_XIP_READ_FORMAT_READ_ADDR_FMT = TLK_BIT(4),
    TLK_MSPI_XIP_READ_FORMAT_READ_ADDR_EN  = TLK_BIT(5),
    TLK_MSPI_XIP_READ_FORMAT_READ_CMD_FMT  = TLK_BIT(6),
    TLK_MSPI_XIP_READ_FORMAT_READ_CMD_EN   = TLK_BIT(7),
} tlk_mspi_xip_read_format_e;

typedef enum tlk_mspi_xip_read_control0_e {
    TLK_MSPI_XIP_READ_CONTROL0_CMD_EN             = TLK_BIT(0),
    TLK_MSPI_XIP_READ_CONTROL0_TOKEN_VAL_SEL      = TLK_BIT(1),
    TLK_MSPI_XIP_READ_CONTROL0_TOKEN_EN           = TLK_BIT(2),
    TLK_MSPI_XIP_READ_CONTROL0_READ_DUMMY_CNT_ADD = TLK_BIT(3),
    TLK_MSPI_XIP_READ_CONTROL0_DDR_MODE           = TLK_BIT(4),
    TLK_MSPI_XIP_READ_CONTROL0_PAGE_MODE_EN       = TLK_BIT(5),
    TLK_MSPI_XIP_READ_CONTROL0_TIMEOUT_MODE_EN    = TLK_BIT(6),
    TLK_MSPI_XIP_READ_CONTROL0_TCEM_MODE_EN       = TLK_BIT(7),
} tlk_mspi_xip_read_control0_e;

typedef enum tlk_mspi_xip_write_format_e {
    TLK_MSPI_XIP_WRITE_FORMAT_WRITE_ADDR_FMT = TLK_BIT(4),
    TLK_MSPI_XIP_WRITE_FORMAT_WRITE_ADDR_EN  = TLK_BIT(5),
    TLK_MSPI_XIP_WRITE_FORMAT_WRITE_CMD_FMT  = TLK_BIT(6),
    TLK_MSPI_XIP_WRITE_FORMAT_WRITE_CMD_EN   = TLK_BIT(7),
} tlk_mspi_xip_write_format_e;

typedef enum tlk_mspi_xip_write_control0_e {
    TLK_MSPI_XIP_WRITE_CONTROL0_CMD_EN = TLK_BIT(0),
} tlk_mspi_xip_write_control0_e;

typedef struct __attribute__((packed)) {
    uint8_t write_read_data[4]; // address: 0xa3ffff00, offset: 0x0
    uint8_t command; // address: 0xa3ffff04, offset: 0x4
    union {
        struct {
            unsigned reserved0 : 2;
            unsigned rx_fifo_irq_en : 1;
            unsigned tx_fifo_irq_en : 1;
            unsigned end_irq_en : 1;
            unsigned reserved1 : 1;
            unsigned rx_dma_en : 1;
            unsigned tx_dma_en : 1;
        } __attribute__((packed)) control0_bit;
        struct {
            enum tlk_mspi_control0_e __attribute__((packed)) control0 : 8;
        }; // address: 0xa3ffff05, offset: 0x5
    };
    uint8_t command1; // address: 0xa3ffff06, offset: 0x6
    union {
        struct {
            unsigned cs2sclk : 3;
            unsigned csht_low : 5;
        } __attribute__((packed)) timing_bit;
        uint8_t timing; // address: 0xa3ffff07, offset: 0x7
    };
    union {
        struct {
            unsigned data_lane : 2;
            unsigned addr_len : 2;
            unsigned addr_fmt : 1;
            unsigned addr_en : 1;
            unsigned cmd_fmt : 1;
            unsigned cmd_en : 1;
        } __attribute__((packed)) control1_bit;
        struct {
            enum tlk_mspi_control1_e __attribute__((packed)) control1 : 8;
        }; // address: 0xa3ffff08, offset: 0x8
    };
    union {
        struct {
            unsigned dummy_cnt : 4;
            unsigned transmode : 4;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0xa3ffff09, offset: 0x9
    };
    union {
        struct {
            unsigned cmd1_en : 1;
            unsigned token_val_sel : 1;
            unsigned token_en : 1;
            unsigned ddr_mode : 1;
            unsigned csht_high : 2;
            unsigned reserved : 2;
        } __attribute__((packed)) reg_control0_bit;
        struct {
            enum tlk_mspi_reg_control0_e __attribute__((packed)) reg_control0 : 8;
        }; // address: 0xa3ffff0a, offset: 0xa
    };
    uint8_t xip_write_tcem_set; // address: 0xa3ffff0b, offset: 0xb
    uint8_t address[4]; // address: 0xa3ffff0c, offset: 0xc
    uint8_t tx_count[3]; // address: 0xa3ffff10, offset: 0x10
    uint8_t reserved0; // address: 0xa3ffff13, offset: 0x13
    uint8_t rx_count[3]; // address: 0xa3ffff14, offset: 0x14
    uint8_t reserved1; // address: 0xa3ffff17, offset: 0x17
    union {
        struct {
            unsigned spi_lsb : 1;
            unsigned spi_3line : 1;
            unsigned spi_mode : 2;
            unsigned reserved : 1;
            unsigned dma_tx_sof_clr_tx_fifo_en : 1;
            unsigned dma_rx_eof_clr_rx_fifo_en : 1;
            unsigned auto_hready_en : 1;
        } __attribute__((packed)) control3_bit;
        struct {
            enum tlk_mspi_control3_e __attribute__((packed)) control3 : 8;
        }; // address: 0xa3ffff18, offset: 0x18
    };
    union {
        struct {
            unsigned value : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) tx_fifo_threshold_bit;
        uint8_t tx_fifo_threshold; // address: 0xa3ffff19, offset: 0x19
    };
    union {
        struct {
            unsigned value : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) rx_fifo_threshold_bit;
        uint8_t rx_fifo_threshold; // address: 0xa3ffff1a, offset: 0x1a
    };
    union {
        struct {
            unsigned event_en : 1;
            unsigned task_en : 1;
            unsigned pem_event_sel : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) pem_control0_bit;
        struct {
            enum tlk_mspi_pem_control0_e __attribute__((packed)) pem_control0 : 8;
        }; // address: 0xa3ffff1b, offset: 0x1b
    };
    union {
        struct {
            unsigned dma_trig_spi_en : 1;
            unsigned tx_dma_req_after_cmd : 1;
            unsigned xip_stop : 1;
            unsigned xip_en : 1;
            unsigned dummy_cnt_add : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) control4_bit;
        struct {
            enum tlk_mspi_control4_e __attribute__((packed)) control4 : 8;
        }; // address: 0xa3ffff1c, offset: 0x1c
    };
    uint8_t xip_page_size; // address: 0xa3ffff1d, offset: 0x1d
    uint8_t xip_timeout_count; // address: 0xa3ffff1e, offset: 0x1e
    uint8_t xip_read_tcem_set; // address: 0xa3ffff1f, offset: 0x1f
    uint8_t reserved2[0x2]; // address: 0xa3ffff20, offset: 0x20
    uint8_t xip_address_offset; // address: 0xa3ffff22, offset: 0x22
    uint8_t reserved3; // address: 0xa3ffff23, offset: 0x23
    union {
        struct {
            unsigned tx_fifo_entries : 7;
            unsigned tx_fifo_full : 1;
        } __attribute__((packed)) tx_fifo_status_bit;
        struct {
            enum tlk_mspi_tx_fifo_status_e __attribute__((packed)) tx_fifo_status : 8;
        }; // address: 0xa3ffff24, offset: 0x24
    };
    union {
        struct {
            unsigned rx_fifo_entries : 7;
            unsigned rx_fifo_empty : 1;
        } __attribute__((packed)) rx_fifo_status_bit;
        struct {
            enum tlk_mspi_rx_fifo_status_e __attribute__((packed)) rx_fifo_status : 8;
        }; // address: 0xa3ffff25, offset: 0x25
    };
    uint8_t reserved4[0x2]; // address: 0xa3ffff26, offset: 0x26
    union {
        struct {
            unsigned reserved0 : 2;
            unsigned spi_soft_reset : 1;
            unsigned xip_arb_err : 1;
            unsigned rx_fifo_clr_level : 1;
            unsigned tx_fifo_clr_level : 1;
            unsigned reserved1 : 1;
            unsigned busy : 1;
        } __attribute__((packed)) status_bit;
        struct {
            enum tlk_mspi_status_e __attribute__((packed)) status : 8;
        }; // address: 0xa3ffff28, offset: 0x28
    };
    uint8_t reserved5; // address: 0xa3ffff29, offset: 0x29
    union {
        struct {
            unsigned reserved0 : 2;
            unsigned rx_fifo_thres_irq_status : 1;
            unsigned tx_fifo_thres_irq_status : 1;
            unsigned trans_end_irq_status : 1;
            unsigned reserved1 : 3;
        } __attribute__((packed)) irq_status0_bit;
        struct {
            enum tlk_mspi_irq_status0_e __attribute__((packed)) irq_status0 : 8;
        }; // address: 0xa3ffff2a, offset: 0x2a
    };
    uint8_t reserved6[0x5a]; // address: 0xa3ffff2b, offset: 0x2b
    union {
        struct {
            unsigned cipher_read_en : 1;
            unsigned cipher_write_en : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) cipher_control_bit;
        struct {
            enum tlk_mspi_cipher_control_e __attribute__((packed)) cipher_control : 8;
        }; // address: 0xa3ffff85, offset: 0x85
    };
    uint8_t reserved7[0xa]; // address: 0xa3ffff86, offset: 0x86
    struct {
        union {
            struct {
                unsigned read_data_lane : 2;
                unsigned read_addr_len : 2;
                unsigned read_addr_fmt : 1;
                unsigned read_addr_en : 1;
                unsigned read_cmd_fmt : 1;
                unsigned read_cmd_en : 1;
            } __attribute__((packed)) read_format_bit;
            struct {
                enum tlk_mspi_xip_read_format_e __attribute__((packed)) read_format : 8;
            }; // address: 0xa3ffff90, offset: 0x90
        };
        union {
            struct {
                unsigned read_dummy_cnt : 4;
                unsigned read_transmode : 4;
            } __attribute__((packed)) read_transmode_bit;
            uint8_t read_transmode; // address: 0xa3ffff91, offset: 0x91
        };
        union {
            struct {
                unsigned cmd_en : 1;
                unsigned token_val_sel : 1;
                unsigned token_en : 1;
                unsigned read_dummy_cnt_add : 1;
                unsigned ddr_mode : 1;
                unsigned page_mode_en : 1;
                unsigned timeout_mode_en : 1;
                unsigned tcem_mode_en : 1;
            } __attribute__((packed)) read_control0_bit;
            struct {
                enum tlk_mspi_xip_read_control0_e __attribute__((packed)) read_control0 : 8;
            }; // address: 0xa3ffff92, offset: 0x92
        };
        uint8_t read_command; // address: 0xa3ffff93, offset: 0x93
        union {
            struct {
                unsigned write_data_lane : 2;
                unsigned write_addr_len : 2;
                unsigned write_addr_fmt : 1;
                unsigned write_addr_en : 1;
                unsigned write_cmd_fmt : 1;
                unsigned write_cmd_en : 1;
            } __attribute__((packed)) write_format_bit;
            struct {
                enum tlk_mspi_xip_write_format_e __attribute__((packed)) write_format : 8;
            }; // address: 0xa3ffff94, offset: 0x94
        };
        union {
            struct {
                unsigned write_dummy_cnt : 4;
                unsigned write_transmode : 4;
            } __attribute__((packed)) write_transmode_bit;
            uint8_t write_transmode; // address: 0xa3ffff95, offset: 0x95
        };
        union {
            struct {
                unsigned cmd_en : 1;
                unsigned reserved : 7;
            } __attribute__((packed)) write_control0_bit;
            struct {
                enum tlk_mspi_xip_write_control0_e __attribute__((packed)) write_control0 : 8;
            }; // address: 0xa3ffff96, offset: 0x96
        };
        uint8_t write_command; // address: 0xa3ffff97, offset: 0x97
    } __attribute__((packed)) xip[4]; // address: 0xa3ffff90, offset: 0x90
    union {
        struct {
            unsigned xip_psram0_end_addr : 2;
            unsigned xip_psram1_end_addr : 2;
            unsigned xip_psram2_end_addr : 2;
            unsigned xip_psram3_end_addr : 2;
        } __attribute__((packed)) xip_size_set_bit;
        uint8_t xip_size_set; // address: 0xa3ffffb0, offset: 0xb0
    };
    uint8_t reserved8[0x3]; // address: 0xa3ffffb1, offset: 0xb1
    struct {
        uint8_t read; // address: 0xa3ffffb4, offset: 0xb4
        uint8_t write; // address: 0xa3ffffb5, offset: 0xb5
    } __attribute__((packed)) xip_command[4]; // address: 0xa3ffffb4, offset: 0xb4
    uint8_t reserved9[0x4]; // address: 0xa3ffffbc, offset: 0xbc
    struct {
        uint16_t start; // address: 0xa3ffffc0, offset: 0xc0
        uint16_t size; // address: 0xa3ffffc2, offset: 0xc2
        uint16_t offset_value; // address: 0xa3ffffc4, offset: 0xc4
        uint16_t reserved; // address: 0xa3ffffc6, offset: 0xc6
    } __attribute__((packed)) xip_core[3]; // address: 0xa3ffffc0, offset: 0xc0
} tlk_mspi_reg_t;

#define TLK_MSPI_BASE_ADDR (0xa3ffff00U)
#define tlk_mspi_reg (*(volatile tlk_mspi_reg_t *) TLK_MSPI_BASE_ADDR)

#endif