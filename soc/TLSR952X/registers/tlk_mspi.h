#ifndef TLK_TLK_MSPI_REGISTERS_H_
#define TLK_TLK_MSPI_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_mspi_control0_e {
    TLK_MSPI_CONTROL0_RXFIFO_IRQ_EN = TLK_BIT(2),
    TLK_MSPI_CONTROL0_TXFIFO_IRQ_EN = TLK_BIT(3),
    TLK_MSPI_CONTROL0_END_IRQ_EN    = TLK_BIT(4),
    TLK_MSPI_CONTROL0_RX_DMA_EN     = TLK_BIT(6),
    TLK_MSPI_CONTROL0_TX_DMA_EN     = TLK_BIT(7),
} tlk_mspi_control0_e;

typedef enum tlk_mspi_control1_e {
    TLK_MSPI_CONTROL1_ADDR_FMT = TLK_BIT(4),
    TLK_MSPI_CONTROL1_ADDR_EN  = TLK_BIT(5),
    TLK_MSPI_CONTROL1_CMD_FMT  = TLK_BIT(6),
    TLK_MSPI_CONTROL1_CMD_EN   = TLK_BIT(7),
} tlk_mspi_control1_e;

typedef enum tlk_mspi_control3_e {
    TLK_MSPI_CONTROL3_SPI_LSB                = TLK_BIT(0),
    TLK_MSPI_CONTROL3_SPI_3LINE              = TLK_BIT(1),
    TLK_MSPI_CONTROL3_NO_USED                = TLK_BIT(4),
    TLK_MSPI_CONTROL3_DMATX_SOF_CLRTXFIFO_EN = TLK_BIT(5),
    TLK_MSPI_CONTROL3_DMARX_EOF_CLRRXFIFO_EN = TLK_BIT(6),
    TLK_MSPI_CONTROL3_AUTO_HREADY_EN         = TLK_BIT(7),
} tlk_mspi_control3_e;

typedef enum tlk_mspi_control4_e {
    TLK_MSPI_CONTROL4_XIP_PAGE_MODE_EN    = TLK_BIT(0),
    TLK_MSPI_CONTROL4_XIP_TIMEOUT_MODE_EN = TLK_BIT(1),
    TLK_MSPI_CONTROL4_XIP_STOP            = TLK_BIT(2),
    TLK_MSPI_CONTROL4_XIP_ENABLE          = TLK_BIT(3),
    TLK_MSPI_CONTROL4_DUMMY_CNT_ADD       = TLK_BIT(4),
} tlk_mspi_control4_e;

typedef enum tlk_mspi_txfifo_status_e {
    TLK_MSPI_TXFIFO_STATUS_TXFIFO_FULL = TLK_BIT(7),
} tlk_mspi_txfifo_status_e;

typedef enum tlk_mspi_rxfifo_status_e {
    TLK_MSPI_RXFIFO_STATUS_RXFIFO_EMPTY = TLK_BIT(7),
} tlk_mspi_rxfifo_status_e;

typedef enum tlk_mspi_status_e {
    TLK_MSPI_STATUS_SPI_SOFT_RESET   = TLK_BIT(2),
    TLK_MSPI_STATUS_XIP_REG_ARB_ERR  = TLK_BIT(3),
    TLK_MSPI_STATUS_RXFIFO_CLR_LEVEL = TLK_BIT(4),
    TLK_MSPI_STATUS_TXFIFO_CLR_LEVEL = TLK_BIT(5),
    TLK_MSPI_STATUS_BUSY             = TLK_BIT(7),
} tlk_mspi_status_e;

typedef enum tlk_mspi_irq_status0_e {
    TLK_MSPI_IRQ_STATUS0_RXF_THRES_IRQ = TLK_BIT(2),
    TLK_MSPI_IRQ_STATUS0_TXF_THRES_IRQ = TLK_BIT(3),
    TLK_MSPI_IRQ_STATUS0_TRANS_END_IRQ = TLK_BIT(4),
} tlk_mspi_irq_status0_e;

typedef enum tlk_mspi_xip_rd_fmt_e {
    TLK_MSPI_XIP_RD_FMT_XIP0_RD_DATA_DUAL = TLK_BIT(0),
    TLK_MSPI_XIP_RD_FMT_XIP0_RD_DATA_QUAD = TLK_BIT(1),
    TLK_MSPI_XIP_RD_FMT_XIP0_RD_ADDR_FMT  = TLK_BIT(4),
    TLK_MSPI_XIP_RD_FMT_XIP0_RD_ADDR_EN   = TLK_BIT(5),
    TLK_MSPI_XIP_RD_FMT_XIP0_RD_CMD_FMT   = TLK_BIT(6),
    TLK_MSPI_XIP_RD_FMT_XIP0_RD_CMD_EN    = TLK_BIT(7),
} tlk_mspi_xip_rd_fmt_e;

typedef enum tlk_mspi_xip_wr_fmt_e {
    TLK_MSPI_XIP_WR_FMT_XIP0_WR_DATA_DUAL = TLK_BIT(0),
    TLK_MSPI_XIP_WR_FMT_XIP0_WR_DATA_QUAD = TLK_BIT(1),
    TLK_MSPI_XIP_WR_FMT_XIP0_WR_ADDR_FMT  = TLK_BIT(4),
    TLK_MSPI_XIP_WR_FMT_XIP0_WR_ADDR_EN   = TLK_BIT(5),
    TLK_MSPI_XIP_WR_FMT_XIP0_WR_CMD_FMT   = TLK_BIT(6),
    TLK_MSPI_XIP_WR_FMT_XIP0_WR_CMD_EN    = TLK_BIT(7),
} tlk_mspi_xip_wr_fmt_e;

typedef struct __attribute__((packed)) {
    uint8_t wr_rd_data[4]; // address: 0xa3ffff00, offset: 0x0
    uint8_t cmd; // address: 0xa3ffff04, offset: 0x4
    union {
        struct {
            unsigned reserved0 : 2;
            unsigned rxfifo_irq_en : 1;
            unsigned txfifo_irq_en : 1;
            unsigned end_irq_en : 1;
            unsigned reserved1 : 1;
            unsigned rx_dma_en : 1;
            unsigned tx_dma_en : 1;
        } __attribute__((packed)) control0_bit;
        struct {
            enum tlk_mspi_control0_e __attribute__((packed)) control0 : 8;
        }; // address: 0xa3ffff05, offset: 0x5
    };
    uint8_t reserved0; // address: 0xa3ffff06, offset: 0x6
    union {
        struct {
            unsigned cs2sclk : 3;
            unsigned csht : 5;
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
    uint8_t reserved1[0x2]; // address: 0xa3ffff0a, offset: 0xa
    uint32_t addr; // address: 0xa3ffff0c, offset: 0xc
    uint8_t tx_cnt[3]; // address: 0xa3ffff10, offset: 0x10
    uint8_t reserved2; // address: 0xa3ffff13, offset: 0x13
    uint8_t rx_cnt[3]; // address: 0xa3ffff14, offset: 0x14
    uint8_t reserved3; // address: 0xa3ffff17, offset: 0x17
    union {
        struct {
            unsigned spi_lsb : 1;
            unsigned spi_3line : 1;
            unsigned spi_mode : 2;
            unsigned no_used : 1;
            unsigned dmatx_sof_clrtxfifo_en : 1;
            unsigned dmarx_eof_clrrxfifo_en : 1;
            unsigned auto_hready_en : 1;
        } __attribute__((packed)) control3_bit;
        struct {
            enum tlk_mspi_control3_e __attribute__((packed)) control3 : 8;
        }; // address: 0xa3ffff18, offset: 0x18
    };
    union {
        struct {
            unsigned txfifo_threshold : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) txfifo_thres_bit;
        uint8_t txfifo_thres; // address: 0xa3ffff19, offset: 0x19
    };
    union {
        struct {
            unsigned rxfifo_threshold : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) rxfifo_thres_bit;
        uint8_t rxfifo_thres; // address: 0xa3ffff1a, offset: 0x1a
    };
    uint8_t reserved4; // address: 0xa3ffff1b, offset: 0x1b
    union {
        struct {
            unsigned xip_page_mode_en : 1;
            unsigned xip_timeout_mode_en : 1;
            unsigned xip_stop : 1;
            unsigned xip_enable : 1;
            unsigned dummy_cnt_add : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) control4_bit;
        struct {
            enum tlk_mspi_control4_e __attribute__((packed)) control4 : 8;
        }; // address: 0xa3ffff1c, offset: 0x1c
    };
    uint8_t xip_page_size; // address: 0xa3ffff1d, offset: 0x1d
    uint8_t xip_timeout_cnt; // address: 0xa3ffff1e, offset: 0x1e
    uint8_t reserved5; // address: 0xa3ffff1f, offset: 0x1f
    union {
        struct {
            unsigned mspi_set_l : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) set_l_bit;
        uint8_t set_l; // address: 0xa3ffff20, offset: 0x20
    };
    union {
        struct {
            unsigned mspi_set_h : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) set_h_bit;
        uint8_t set_h; // address: 0xa3ffff21, offset: 0x21
    };
    uint8_t xip_addr_offset; // address: 0xa3ffff22, offset: 0x22
    uint8_t reserved6; // address: 0xa3ffff23, offset: 0x23
    union {
        struct {
            unsigned txfifo_entries : 7;
            unsigned txfifo_full : 1;
        } __attribute__((packed)) txfifo_status_bit;
        struct {
            enum tlk_mspi_txfifo_status_e __attribute__((packed)) txfifo_status : 8;
        }; // address: 0xa3ffff24, offset: 0x24
    };
    union {
        struct {
            unsigned rxfifo_entries : 7;
            unsigned rxfifo_empty : 1;
        } __attribute__((packed)) rxfifo_status_bit;
        struct {
            enum tlk_mspi_rxfifo_status_e __attribute__((packed)) rxfifo_status : 8;
        }; // address: 0xa3ffff25, offset: 0x25
    };
    uint8_t reserved7[0x2]; // address: 0xa3ffff26, offset: 0x26
    union {
        struct {
            unsigned reserved0 : 2;
            unsigned spi_soft_reset : 1;
            unsigned xip_reg_arb_err : 1;
            unsigned rxfifo_clr_level : 1;
            unsigned txfifo_clr_level : 1;
            unsigned reserved1 : 1;
            unsigned busy : 1;
        } __attribute__((packed)) status_bit;
        struct {
            enum tlk_mspi_status_e __attribute__((packed)) status : 8;
        }; // address: 0xa3ffff28, offset: 0x28
    };
    uint8_t reserved8; // address: 0xa3ffff29, offset: 0x29
    union {
        struct {
            unsigned reserved0 : 2;
            unsigned rxf_thres_irq : 1;
            unsigned txf_thres_irq : 1;
            unsigned trans_end_irq : 1;
            unsigned reserved1 : 3;
        } __attribute__((packed)) irq_status0_bit;
        struct {
            enum tlk_mspi_irq_status0_e __attribute__((packed)) irq_status0 : 8;
        }; // address: 0xa3ffff2a, offset: 0x2a
    };
    uint8_t reserved9[0x65]; // address: 0xa3ffff2b, offset: 0x2b
    union {
        struct {
            unsigned xip0_rd_data_dual : 1;
            unsigned xip0_rd_data_quad : 1;
            unsigned xip0_rd_addr_len : 2;
            unsigned xip0_rd_addr_fmt : 1;
            unsigned xip0_rd_addr_en : 1;
            unsigned xip0_rd_cmd_fmt : 1;
            unsigned xip0_rd_cmd_en : 1;
        } __attribute__((packed)) xip_rd_fmt_bit;
        struct {
            enum tlk_mspi_xip_rd_fmt_e __attribute__((packed)) xip_rd_fmt : 8;
        }; // address: 0xa3ffff90, offset: 0x90
    };
    union {
        struct {
            unsigned xip0_rd_dummy_cnt : 4;
            unsigned xip0_rd_transmode : 4;
        } __attribute__((packed)) xip_rd_transmode_bit;
        uint8_t xip_rd_transmode; // address: 0xa3ffff91, offset: 0x91
    };
    uint8_t reserved10; // address: 0xa3ffff92, offset: 0x92
    uint8_t xip_rd_cmd; // address: 0xa3ffff93, offset: 0x93
    union {
        struct {
            unsigned xip0_wr_data_dual : 1;
            unsigned xip0_wr_data_quad : 1;
            unsigned xip0_wr_addr_len : 2;
            unsigned xip0_wr_addr_fmt : 1;
            unsigned xip0_wr_addr_en : 1;
            unsigned xip0_wr_cmd_fmt : 1;
            unsigned xip0_wr_cmd_en : 1;
        } __attribute__((packed)) xip_wr_fmt_bit;
        struct {
            enum tlk_mspi_xip_wr_fmt_e __attribute__((packed)) xip_wr_fmt : 8;
        }; // address: 0xa3ffff94, offset: 0x94
    };
    union {
        struct {
            unsigned reserved : 4;
            unsigned xip0_wr_transmode : 4;
        } __attribute__((packed)) xip_wr_transmode_bit;
        uint8_t xip_wr_transmode; // address: 0xa3ffff95, offset: 0x95
    };
    uint8_t reserved11; // address: 0xa3ffff96, offset: 0x96
    uint8_t xip_wr_cmd; // address: 0xa3ffff97, offset: 0x97
} tlk_mspi_reg_t;

#define TLK_MSPI_BASE_ADDR (0xa3ffff00U)
#define tlk_mspi_reg (*(volatile tlk_mspi_reg_t *) TLK_MSPI_BASE_ADDR)

#endif