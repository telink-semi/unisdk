#ifndef TLK_TLK_SPI_REGISTERS_H_
#define TLK_TLK_SPI_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_spi_control0_e {
    TLK_SPI_CONTROL0_RXF_OVERRUN_INT_EN  = TLK_BIT(0),
    TLK_SPI_CONTROL0_TXF_UNDERRUN_INT_EN = TLK_BIT(1),
    TLK_SPI_CONTROL0_RXF_THRES_INT_EN    = TLK_BIT(2),
    TLK_SPI_CONTROL0_TXF_THRES_INT_EN    = TLK_BIT(3),
    TLK_SPI_CONTROL0_TRANS_END_INT_EN    = TLK_BIT(4),
    TLK_SPI_CONTROL0_SLAVE_CMD_INT_EN    = TLK_BIT(5),
    TLK_SPI_CONTROL0_RX_DMA_EN           = TLK_BIT(6),
    TLK_SPI_CONTROL0_TX_DMA_EN           = TLK_BIT(7),
} tlk_spi_control0_e;

typedef enum tlk_spi_control1_e {
    TLK_SPI_CONTROL1_ADDR_FMT = TLK_BIT(4),
    TLK_SPI_CONTROL1_ADDR_EN  = TLK_BIT(5),
    TLK_SPI_CONTROL1_CMD_FMT  = TLK_BIT(6),
    TLK_SPI_CONTROL1_CMD_EN   = TLK_BIT(7),
} tlk_spi_control1_e;

typedef enum tlk_spi_control1_data_lane_io_mode_e {
    TLK_SPI_CONTROL1_DATA_LANE_IO_MODE_SINGLE = 0x0U,
    TLK_SPI_CONTROL1_DATA_LANE_IO_MODE_DUAL   = 0x1U,
    TLK_SPI_CONTROL1_DATA_LANE_IO_MODE_QUAD   = 0x2U,
    TLK_SPI_CONTROL1_DATA_LANE_IO_MODE_QUAD1  = 0x3U,
} tlk_spi_control1_data_lane_io_mode_e;

typedef enum tlk_spi_control1_addr_len_e {
    TLK_SPI_CONTROL1_ADDR_LEN_1BYTE  = 0x0U,
    TLK_SPI_CONTROL1_ADDR_LEN_2BYTES = 0x1U,
    TLK_SPI_CONTROL1_ADDR_LEN_3BYTES = 0x2U,
    TLK_SPI_CONTROL1_ADDR_LEN_4BYTES = 0x3U,
} tlk_spi_control1_addr_len_e;

typedef enum tlk_spi_control2_transmode_e {
    TLK_SPI_CONTROL2_TRANSMODE_WRITE_AND_READ   = 0x0U,
    TLK_SPI_CONTROL2_TRANSMODE_WRITE_ONLY       = 0x1U,
    TLK_SPI_CONTROL2_TRANSMODE_READ_ONLY        = 0x2U,
    TLK_SPI_CONTROL2_TRANSMODE_WRITE_THEN_READ  = 0x3U,
    TLK_SPI_CONTROL2_TRANSMODE_READ_THEN_WRITE  = 0x4U,
    TLK_SPI_CONTROL2_TRANSMODE_WRITE_DUMMY_READ = 0x5U,
    TLK_SPI_CONTROL2_TRANSMODE_READ_DUMMY_WRITE = 0x6U,
    TLK_SPI_CONTROL2_TRANSMODE_NONE_DATA        = 0x7U,
    TLK_SPI_CONTROL2_TRANSMODE_DUMMY_WRITE      = 0x8U,
    TLK_SPI_CONTROL2_TRANSMODE_DUMMY_READ       = 0x9U,
} tlk_spi_control2_transmode_e;

typedef enum tlk_spi_control3_e {
    TLK_SPI_CONTROL3_SPI_LSB                = TLK_BIT(0),
    TLK_SPI_CONTROL3_SPI_3LINE              = TLK_BIT(1),
    TLK_SPI_CONTROL3_SPI_MASTER             = TLK_BIT(4),
    TLK_SPI_CONTROL3_DMATX_SOF_CLRTXFIFO_EN = TLK_BIT(5),
    TLK_SPI_CONTROL3_DMARX_EOF_CLRRXFIFO_EN = TLK_BIT(6),
    TLK_SPI_CONTROL3_AUTO_HREADY_EN         = TLK_BIT(7),
} tlk_spi_control3_e;

typedef enum tlk_spi_control3_spi_mode_e {
    TLK_SPI_CONTROL3_SPI_MODE_MODE0 = 0x0U,
    TLK_SPI_CONTROL3_SPI_MODE_MODE1 = 0x1U,
    TLK_SPI_CONTROL3_SPI_MODE_MODE2 = 0x2U,
    TLK_SPI_CONTROL3_SPI_MODE_MODE3 = 0x3U,
} tlk_spi_control3_spi_mode_e;

typedef enum tlk_spi_control4_e {
    TLK_SPI_CONTROL4_XIP_PAGE_MODE_EN    = TLK_BIT(0),
    TLK_SPI_CONTROL4_XIP_TIMEOUT_MODE_EN = TLK_BIT(1),
    TLK_SPI_CONTROL4_XIP_STOP            = TLK_BIT(2),
    TLK_SPI_CONTROL4_XIP_ENABLE          = TLK_BIT(3),
    TLK_SPI_CONTROL4_DUMMY_CNT_ADD       = TLK_BIT(4),
} tlk_spi_control4_e;

typedef enum tlk_spi_txfifo_status_e {
    TLK_SPI_TXFIFO_STATUS_TXFIFO_FULL = TLK_BIT(7),
} tlk_spi_txfifo_status_e;

typedef enum tlk_spi_rxfifo_status_e {
    TLK_SPI_RXFIFO_STATUS_RXFIFO_EMPTY = TLK_BIT(7),
} tlk_spi_rxfifo_status_e;

typedef enum tlk_spi_status_e {
    TLK_SPI_STATUS_SET_SLAVE_READY  = TLK_BIT(0),
    TLK_SPI_STATUS_CLR_SLAVE_READY  = TLK_BIT(1),
    TLK_SPI_STATUS_SPI_SOFT_RESET   = TLK_BIT(2),
    TLK_SPI_STATUS_XIP_REG_ARB_ERR  = TLK_BIT(3),
    TLK_SPI_STATUS_RXFIFO_CLR_LEVEL = TLK_BIT(4),
    TLK_SPI_STATUS_TXFIFO_CLR_LEVEL = TLK_BIT(5),
    TLK_SPI_STATUS_OSD_AHBMST_BUSY  = TLK_BIT(6),
    TLK_SPI_STATUS_BUSY             = TLK_BIT(7),
} tlk_spi_status_e;

typedef enum tlk_spi_int_status0_e {
    TLK_SPI_INT_STATUS0_RXF_OVERRUN_INT_STUS  = TLK_BIT(0),
    TLK_SPI_INT_STATUS0_TXF_UNDERRUN_INT_STUS = TLK_BIT(1),
    TLK_SPI_INT_STATUS0_RXF_THRES_INT_STUS    = TLK_BIT(2),
    TLK_SPI_INT_STATUS0_TXF_THRES_INT_STUS    = TLK_BIT(3),
    TLK_SPI_INT_STATUS0_TRANS_END_INT_STUS    = TLK_BIT(4),
    TLK_SPI_INT_STATUS0_SLAVE_CMD_INT_STUS    = TLK_BIT(5),
} tlk_spi_int_status0_e;

typedef struct __attribute__((packed)) {
    uint8_t wr_rd_data[4]; // address: 0x87ffff00, offset: 0x0
    uint8_t command; // address: 0x87ffff04, offset: 0x4
    union {
        struct {
            unsigned rxf_overrun_int_en : 1;
            unsigned txf_underrun_int_en : 1;
            unsigned rxf_thres_int_en : 1;
            unsigned txf_thres_int_en : 1;
            unsigned trans_end_int_en : 1;
            unsigned slave_cmd_int_en : 1;
            unsigned rx_dma_en : 1;
            unsigned tx_dma_en : 1;
        } __attribute__((packed)) control0_bit;
        struct {
            enum tlk_spi_control0_e __attribute__((packed)) control0 : 8;
        }; // address: 0x87ffff05, offset: 0x5
    };
    uint8_t reserved0; // address: 0x87ffff06, offset: 0x6
    union {
        struct {
            unsigned cs2sclk : 3;
            unsigned csht : 5;
        } __attribute__((packed)) timing_bit;
        uint8_t timing; // address: 0x87ffff07, offset: 0x7
    };
    union {
        struct {
            enum tlk_spi_control1_data_lane_io_mode_e __attribute__((packed)) data_lane_io_mode : 2;
            enum tlk_spi_control1_addr_len_e __attribute__((packed)) addr_len : 2;
            unsigned addr_fmt : 1;
            unsigned addr_en : 1;
            unsigned cmd_fmt : 1;
            unsigned cmd_en : 1;
        } __attribute__((packed)) control1_bit;
        struct {
            enum tlk_spi_control1_e __attribute__((packed)) control1 : 8;
        }; // address: 0x87ffff08, offset: 0x8
    };
    union {
        struct {
            unsigned dummy_cnt : 4;
            enum tlk_spi_control2_transmode_e __attribute__((packed)) transmode : 4;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0x87ffff09, offset: 0x9
    };
    uint8_t reserved1[0x2]; // address: 0x87ffff0a, offset: 0xa
    uint32_t address; // address: 0x87ffff0c, offset: 0xc
    union {
        struct {
            unsigned count : 24;
            unsigned reserved : 8;
        } __attribute__((packed)) tx_count_bit;
        uint32_t tx_count; // address: 0x87ffff10, offset: 0x10
    };
    union {
        struct {
            unsigned count : 24;
            unsigned reserved : 8;
        } __attribute__((packed)) rx_count_bit;
        uint32_t rx_count; // address: 0x87ffff14, offset: 0x14
    };
    union {
        struct {
            unsigned spi_lsb : 1;
            unsigned spi_3line : 1;
            enum tlk_spi_control3_spi_mode_e __attribute__((packed)) spi_mode : 2;
            unsigned spi_master : 1;
            unsigned dmatx_sof_clrtxfifo_en : 1;
            unsigned dmarx_eof_clrrxfifo_en : 1;
            unsigned auto_hready_en : 1;
        } __attribute__((packed)) control3_bit;
        struct {
            enum tlk_spi_control3_e __attribute__((packed)) control3 : 8;
        }; // address: 0x87ffff18, offset: 0x18
    };
    union {
        struct {
            unsigned txfifo_threshold : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) txfifo_thres_bit;
        uint8_t txfifo_thres; // address: 0x87ffff19, offset: 0x19
    };
    union {
        struct {
            unsigned rxfifo_threshold : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) rxfifo_thres_bit;
        uint8_t rxfifo_thres; // address: 0x87ffff1a, offset: 0x1a
    };
    uint8_t reserved2; // address: 0x87ffff1b, offset: 0x1b
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
            enum tlk_spi_control4_e __attribute__((packed)) control4 : 8;
        }; // address: 0x87ffff1c, offset: 0x1c
    };
    uint8_t xip_page_size; // address: 0x87ffff1d, offset: 0x1d
    uint8_t xip_timeout_cnt; // address: 0x87ffff1e, offset: 0x1e
    uint8_t reserved3[0x3]; // address: 0x87ffff1f, offset: 0x1f
    uint8_t xip_addr_offset; // address: 0x87ffff22, offset: 0x22
    uint8_t reserved4; // address: 0x87ffff23, offset: 0x23
    union {
        struct {
            unsigned txfifo_entries : 7;
            unsigned txfifo_full : 1;
        } __attribute__((packed)) txfifo_status_bit;
        struct {
            enum tlk_spi_txfifo_status_e __attribute__((packed)) txfifo_status : 8;
        }; // address: 0x87ffff24, offset: 0x24
    };
    union {
        struct {
            unsigned rxfifo_entries : 7;
            unsigned rxfifo_empty : 1;
        } __attribute__((packed)) rxfifo_status_bit;
        struct {
            enum tlk_spi_rxfifo_status_e __attribute__((packed)) rxfifo_status : 8;
        }; // address: 0x87ffff25, offset: 0x25
    };
    uint8_t reserved5[0x2]; // address: 0x87ffff26, offset: 0x26
    union {
        struct {
            unsigned set_slave_ready : 1;
            unsigned clr_slave_ready : 1;
            unsigned spi_soft_reset : 1;
            unsigned xip_reg_arb_err : 1;
            unsigned rxfifo_clr_level : 1;
            unsigned txfifo_clr_level : 1;
            unsigned osd_ahbmst_busy : 1;
            unsigned busy : 1;
        } __attribute__((packed)) status_bit;
        struct {
            enum tlk_spi_status_e __attribute__((packed)) status : 8;
        }; // address: 0x87ffff28, offset: 0x28
    };
    uint8_t reserved6; // address: 0x87ffff29, offset: 0x29
    union {
        struct {
            unsigned rxf_overrun_int_stus : 1;
            unsigned txf_underrun_int_stus : 1;
            unsigned rxf_thres_int_stus : 1;
            unsigned txf_thres_int_stus : 1;
            unsigned trans_end_int_stus : 1;
            unsigned slave_cmd_int_stus : 1;
            unsigned reserved : 2;
        } __attribute__((packed)) int_status0_bit;
        struct {
            enum tlk_spi_int_status0_e __attribute__((packed)) int_status0 : 8;
        }; // address: 0x87ffff2a, offset: 0x2a
    };
} tlk_spi_reg_t;

#define TLK_SPI_BASE_ADDR (0x87ffff00U)
#define TLK_SPI0_ADDR (0x87ffff00U)
#define TLK_SPI1_ADDR (0x8bffff00U)
#define tlk_spi_reg(i) (*(volatile tlk_spi_reg_t *) ( \
    ((i) == 0) ? TLK_SPI0_ADDR : \
    ((i) == 1) ? TLK_SPI1_ADDR : \
    0 ))



typedef enum tlk_lspi_xip_xip_read_fmt_e {
    TLK_LSPI_XIP_XIP_READ_FMT_DATA_DUAL = TLK_BIT(0),
    TLK_LSPI_XIP_XIP_READ_FMT_DATA_QUAD = TLK_BIT(1),
    TLK_LSPI_XIP_XIP_READ_FMT_ADDR_FMT  = TLK_BIT(4),
    TLK_LSPI_XIP_XIP_READ_FMT_ADDR_EN   = TLK_BIT(5),
    TLK_LSPI_XIP_XIP_READ_FMT_CMD_FMT   = TLK_BIT(6),
    TLK_LSPI_XIP_XIP_READ_FMT_CMD_EN    = TLK_BIT(7),
} tlk_lspi_xip_xip_read_fmt_e;

typedef enum tlk_lspi_xip_xip_write_fmt_e {
    TLK_LSPI_XIP_XIP_WRITE_FMT_DATA_DUAL = TLK_BIT(0),
    TLK_LSPI_XIP_XIP_WRITE_FMT_DATA_QUAD = TLK_BIT(1),
    TLK_LSPI_XIP_XIP_WRITE_FMT_ADDR_FMT  = TLK_BIT(4),
    TLK_LSPI_XIP_XIP_WRITE_FMT_ADDR_EN   = TLK_BIT(5),
    TLK_LSPI_XIP_XIP_WRITE_FMT_CMD_FMT   = TLK_BIT(6),
    TLK_LSPI_XIP_XIP_WRITE_FMT_CMD_EN    = TLK_BIT(7),
} tlk_lspi_xip_xip_write_fmt_e;

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x90]; // address: 0x87ffff00, offset: 0x0
    union {
        struct {
            unsigned data_dual : 1;
            unsigned data_quad : 1;
            unsigned addr_len : 2;
            unsigned addr_fmt : 1;
            unsigned addr_en : 1;
            unsigned cmd_fmt : 1;
            unsigned cmd_en : 1;
        } __attribute__((packed)) xip_read_fmt_bit;
        struct {
            enum tlk_lspi_xip_xip_read_fmt_e __attribute__((packed)) xip_read_fmt : 8;
        }; // address: 0x87ffff90, offset: 0x90
    };
    union {
        struct {
            unsigned dummy_cnt : 4;
            unsigned transmode : 4;
        } __attribute__((packed)) xip_read_transmode_bit;
        uint8_t xip_read_transmode; // address: 0x87ffff91, offset: 0x91
    };
    uint8_t reserved1; // address: 0x87ffff92, offset: 0x92
    uint8_t xip_read_cmd; // address: 0x87ffff93, offset: 0x93
    union {
        struct {
            unsigned data_dual : 1;
            unsigned data_quad : 1;
            unsigned addr_len : 2;
            unsigned addr_fmt : 1;
            unsigned addr_en : 1;
            unsigned cmd_fmt : 1;
            unsigned cmd_en : 1;
        } __attribute__((packed)) xip_write_fmt_bit;
        struct {
            enum tlk_lspi_xip_xip_write_fmt_e __attribute__((packed)) xip_write_fmt : 8;
        }; // address: 0x87ffff94, offset: 0x94
    };
    union {
        struct {
            unsigned reserved : 4;
            unsigned transmode : 4;
        } __attribute__((packed)) xip_write_transmode_bit;
        uint8_t xip_write_transmode; // address: 0x87ffff95, offset: 0x95
    };
    uint8_t reserved2; // address: 0x87ffff96, offset: 0x96
    uint8_t xip_write_cmd; // address: 0x87ffff97, offset: 0x97
} tlk_lspi_xip_reg_t;

#define TLK_LSPI_XIP_BASE_ADDR (0x87ffff00U)
#define tlk_lspi_xip_reg (*(volatile tlk_lspi_xip_reg_t *) TLK_LSPI_XIP_BASE_ADDR)



typedef enum tlk_lspi_lcd_int_status1_e {
    TLK_LSPI_LCD_INT_STATUS1_LCD_LINE_INT_STUS  = TLK_BIT(0),
    TLK_LSPI_LCD_INT_STATUS1_LCD_LVL_INT_STUS   = TLK_BIT(1),
    TLK_LSPI_LCD_INT_STATUS1_LCD_FRAME_INT_STUS = TLK_BIT(2),
} tlk_lspi_lcd_int_status1_e;

typedef enum tlk_lspi_lcd_lcd_control2_e {
    TLK_LSPI_LCD_LCD_CONTROL2_SINGLE_COLOR_MODE   = TLK_BIT(0),
    TLK_LSPI_LCD_LCD_CONTROL2_RGB_BIG_ENDIAN_MODE = TLK_BIT(1),
    TLK_LSPI_LCD_LCD_CONTROL2_RAM_4BIT_MODE       = TLK_BIT(2),
} tlk_lspi_lcd_lcd_control2_e;

typedef enum tlk_lspi_lcd_lcd_control_e {
    TLK_LSPI_LCD_LCD_CONTROL_SCAN_EN      = TLK_BIT(0),
    TLK_LSPI_LCD_LCD_CONTROL_LCD_2LANE_EN = TLK_BIT(3),
    TLK_LSPI_LCD_LCD_CONTROL_LINE3_DCX_EN = TLK_BIT(6),
    TLK_LSPI_LCD_LCD_CONTROL_DCX          = TLK_BIT(7),
} tlk_lspi_lcd_lcd_control_e;

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x2b]; // address: 0x87ffff00, offset: 0x0
    union {
        struct {
            unsigned lcd_line_int_stus : 1;
            unsigned lcd_lvl_int_stus : 1;
            unsigned lcd_frame_int_stus : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) int_status1_bit;
        struct {
            enum tlk_lspi_lcd_int_status1_e __attribute__((packed)) int_status1 : 8;
        }; // address: 0x87ffff2b, offset: 0x2b
    };
    uint8_t reserved1[0x3]; // address: 0x87ffff2c, offset: 0x2c
    union {
        struct {
            unsigned single_color_mode : 1;
            unsigned rgb_big_endian_mode : 1;
            unsigned ram_4bit_mode : 1;
            unsigned int_mask : 3;
            unsigned reserved : 2;
        } __attribute__((packed)) lcd_control2_bit;
        struct {
            enum tlk_lspi_lcd_lcd_control2_e __attribute__((packed)) lcd_control2 : 8;
        }; // address: 0x87ffff2f, offset: 0x2f
    };
    union {
        struct {
            unsigned scan_en : 1;
            unsigned rgb_mode : 2;
            unsigned lcd_2lane_en : 1;
            unsigned reserved : 2;
            unsigned line3_dcx_en : 1;
            unsigned dcx : 1;
        } __attribute__((packed)) lcd_control_bit;
        struct {
            enum tlk_lspi_lcd_lcd_control_e __attribute__((packed)) lcd_control : 8;
        }; // address: 0x87ffff30, offset: 0x30
    };
    uint8_t lcd_vbp_cnt; // address: 0x87ffff31, offset: 0x31
    uint8_t lcd_vfp_cnt; // address: 0x87ffff32, offset: 0x32
    uint8_t lcd_line_lvl; // address: 0x87ffff33, offset: 0x33
    uint32_t lcd_bimage_addr; // address: 0x87ffff34, offset: 0x34
    uint32_t lcd_fimage_addr; // address: 0x87ffff38, offset: 0x38
    uint8_t reserved2[0x2]; // address: 0x87ffff3c, offset: 0x3c
    union {
        struct {
            unsigned cnt : 10;
            unsigned reserved : 6;
        } __attribute__((packed)) lcd_line_cnt_bit;
        uint16_t lcd_line_cnt; // address: 0x87ffff3e, offset: 0x3e
    };
    uint32_t lcd_lut_data[16]; // address: 0x87ffff40, offset: 0x40
} tlk_lspi_lcd_reg_t;

#define TLK_LSPI_LCD_BASE_ADDR (0x87ffff00U)
#define tlk_lspi_lcd_reg (*(volatile tlk_lspi_lcd_reg_t *) TLK_LSPI_LCD_BASE_ADDR)



typedef enum tlk_gspi_xip_xip_read_fmt_e {
    TLK_GSPI_XIP_XIP_READ_FMT_DATA_DUAL = TLK_BIT(0),
    TLK_GSPI_XIP_XIP_READ_FMT_DATA_QUAD = TLK_BIT(1),
    TLK_GSPI_XIP_XIP_READ_FMT_ADDR_FMT  = TLK_BIT(4),
    TLK_GSPI_XIP_XIP_READ_FMT_ADDR_EN   = TLK_BIT(5),
    TLK_GSPI_XIP_XIP_READ_FMT_CMD_FMT   = TLK_BIT(6),
    TLK_GSPI_XIP_XIP_READ_FMT_CMD_EN    = TLK_BIT(7),
} tlk_gspi_xip_xip_read_fmt_e;

typedef enum tlk_gspi_xip_xip_write_fmt_e {
    TLK_GSPI_XIP_XIP_WRITE_FMT_DATA_DUAL = TLK_BIT(0),
    TLK_GSPI_XIP_XIP_WRITE_FMT_DATA_QUAD = TLK_BIT(1),
    TLK_GSPI_XIP_XIP_WRITE_FMT_ADDR_FMT  = TLK_BIT(4),
    TLK_GSPI_XIP_XIP_WRITE_FMT_ADDR_EN   = TLK_BIT(5),
    TLK_GSPI_XIP_XIP_WRITE_FMT_CMD_FMT   = TLK_BIT(6),
    TLK_GSPI_XIP_XIP_WRITE_FMT_CMD_EN    = TLK_BIT(7),
} tlk_gspi_xip_xip_write_fmt_e;

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x90]; // address: 0x8bffff00, offset: 0x0
    struct {
        union {
            struct {
                unsigned data_dual : 1;
                unsigned data_quad : 1;
                unsigned addr_len : 2;
                unsigned addr_fmt : 1;
                unsigned addr_en : 1;
                unsigned cmd_fmt : 1;
                unsigned cmd_en : 1;
            } __attribute__((packed)) read_fmt_bit;
            struct {
                enum tlk_gspi_xip_xip_read_fmt_e __attribute__((packed)) read_fmt : 8;
            }; // address: 0x8bffff90, offset: 0x90
        };
        union {
            struct {
                unsigned dummy_cnt : 4;
                unsigned transmode : 4;
            } __attribute__((packed)) read_transmode_bit;
            uint8_t read_transmode; // address: 0x8bffff91, offset: 0x91
        };
        uint8_t reserved1; // address: 0x8bffff92, offset: 0x92
        uint8_t read_cmd; // address: 0x8bffff93, offset: 0x93
        union {
            struct {
                unsigned data_dual : 1;
                unsigned data_quad : 1;
                unsigned addr_len : 2;
                unsigned addr_fmt : 1;
                unsigned addr_en : 1;
                unsigned cmd_fmt : 1;
                unsigned cmd_en : 1;
            } __attribute__((packed)) write_fmt_bit;
            struct {
                enum tlk_gspi_xip_xip_write_fmt_e __attribute__((packed)) write_fmt : 8;
            }; // address: 0x8bffff94, offset: 0x94
        };
        union {
            struct {
                unsigned reserved : 4;
                unsigned transmode : 4;
            } __attribute__((packed)) write_transmode_bit;
            uint8_t write_transmode; // address: 0x8bffff95, offset: 0x95
        };
        uint8_t reserved2; // address: 0x8bffff96, offset: 0x96
        uint8_t write_cmd; // address: 0x8bffff97, offset: 0x97
    } __attribute__((packed)) xip[4]; // address: 0x8bffff90, offset: 0x90
    union {
        struct {
            unsigned psram0_end_addr : 2;
            unsigned psram1_end_addr : 2;
            unsigned psram2_end_addr : 2;
            unsigned psram3_end_addr : 2;
        } __attribute__((packed)) xip_size_set_bit;
        uint8_t xip_size_set; // address: 0x8bffffb0, offset: 0xb0
    };
} tlk_gspi_xip_reg_t;

#define TLK_GSPI_XIP_BASE_ADDR (0x8bffff00U)
#define tlk_gspi_xip_reg (*(volatile tlk_gspi_xip_reg_t *) TLK_GSPI_XIP_BASE_ADDR)

#endif