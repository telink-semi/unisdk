#ifndef TLK_GSPI_REGISTERS_H_
#define TLK_GSPI_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t wr_rd_data[4]; // address: 0x8bffff00, offset: 0x0
    uint8_t command; // address: 0x8bffff04, offset: 0x4
    union {
        struct {
            unsigned rxf_overrun_int_en : 1;
            unsigned txf_underrun_int_en : 1;
            unsigned rxf_thres_int_en : 1;
            unsigned txf_thres_int_en : 1;
            unsigned trans_end_int_en : 1;
            unsigned reserved : 1;
            unsigned rx_dma_en : 1;
            unsigned tx_dma_en : 1;
        } __attribute__((packed)) control0_bit;
        uint8_t control0; // address: 0x8bffff05, offset: 0x5
    };
    uint8_t command1; // address: 0x8bffff06, offset: 0x6
    union {
        struct {
            unsigned cs2sclk : 3;
            unsigned csht_low : 5;
        } __attribute__((packed)) timing_bit;
        uint8_t timing; // address: 0x8bffff07, offset: 0x7
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
        uint8_t control1; // address: 0x8bffff08, offset: 0x8
    };
    union {
        struct {
            unsigned dummy_cnt : 4;
            unsigned transmode : 4;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0x8bffff09, offset: 0x9
    };
    union {
        struct {
            unsigned cmd1_en : 1;
            unsigned reg_token_val_sel : 1;
            unsigned reg_token_en : 1;
            unsigned reg_ddr_mode : 1;
            unsigned csht_high : 2;
            unsigned reserved : 2;
        } __attribute__((packed)) reg_control0_bit;
        uint8_t reg_control0; // address: 0x8bffff0a, offset: 0xa
    };
    uint8_t xip_write_tcem; // address: 0x8bffff0b, offset: 0xb
    uint32_t address; // address: 0x8bffff0c, offset: 0xc
    union {
        struct {
            unsigned count : 24;
            unsigned reserved : 8;
        } __attribute__((packed)) tx_count_bit;
        uint32_t tx_count; // address: 0x8bffff10, offset: 0x10
    };
    union {
        struct {
            unsigned count : 24;
            unsigned reserved : 8;
        } __attribute__((packed)) rx_count_bit;
        uint32_t rx_count; // address: 0x8bffff14, offset: 0x14
    };
    union {
        struct {
            unsigned spi_lsb : 1;
            unsigned spi_3line : 1;
            unsigned spi_mode : 2;
            unsigned spi_master : 1;
            unsigned dmatx_sof_clrtxfifo_en : 1;
            unsigned dmarx_eof_clrrxfifo_en : 1;
            unsigned auto_hready_en : 1;
        } __attribute__((packed)) control3_bit;
        uint8_t control3; // address: 0x8bffff18, offset: 0x18
    };
    union {
        struct {
            unsigned txfifo_threshold : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) txfifo_thres_bit;
        uint8_t txfifo_thres; // address: 0x8bffff19, offset: 0x19
    };
    union {
        struct {
            unsigned rxfifo_threshold : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) rxfifo_thres_bit;
        uint8_t rxfifo_thres; // address: 0x8bffff1a, offset: 0x1a
    };
    union {
        struct {
            unsigned event_en : 1;
            unsigned task_en : 1;
            unsigned pem_event_sel : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) pem_control0_bit;
        uint8_t pem_control0; // address: 0x8bffff1b, offset: 0x1b
    };
    union {
        struct {
            unsigned dma_trig_spi_en : 1;
            unsigned txdma_req_after_cmd : 1;
            unsigned xip_stop : 1;
            unsigned xip_enable : 1;
            unsigned dummy_cnt_add : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) control4_bit;
        uint8_t control4; // address: 0x8bffff1c, offset: 0x1c
    };
    uint8_t xip_page_size; // address: 0x8bffff1d, offset: 0x1d
    uint8_t xip_timeout_cnt; // address: 0x8bffff1e, offset: 0x1e
    uint8_t xip_read_tcem; // address: 0x8bffff1f, offset: 0x1f
    uint8_t reserved0[0x2]; // address: 0x8bffff20, offset: 0x20
    uint8_t xip_addr_offset; // address: 0x8bffff22, offset: 0x22
    uint8_t reserved1; // address: 0x8bffff23, offset: 0x23
    union {
        struct {
            unsigned txfifo_entries : 7;
            unsigned txfifo_full : 1;
        } __attribute__((packed)) txfifo_status_bit;
        uint8_t txfifo_status; // address: 0x8bffff24, offset: 0x24
    };
    union {
        struct {
            unsigned rxfifo_entries : 7;
            unsigned rxfifo_empty : 1;
        } __attribute__((packed)) rxfifo_status_bit;
        uint8_t rxfifo_status; // address: 0x8bffff25, offset: 0x25
    };
    uint8_t reserved2[0x2]; // address: 0x8bffff26, offset: 0x26
    union {
        struct {
            unsigned set_slave_ready : 1;
            unsigned clr_slave_ready : 1;
            unsigned spi_soft_reset : 1;
            unsigned xip_reg_arb_err : 1;
            unsigned rxfifo_clr_level : 1;
            unsigned txfifo_clr_level : 1;
            unsigned reserved : 1;
            unsigned busy : 1;
        } __attribute__((packed)) status_bit;
        uint8_t status; // address: 0x8bffff28, offset: 0x28
    };
    union {
        struct {
            unsigned mode : 4;
            unsigned reserved : 4;
        } __attribute__((packed)) slv_trans_mode_bit;
        uint8_t slv_trans_mode; // address: 0x8bffff29, offset: 0x29
    };
    union {
        struct {
            unsigned rxf_overrun_int_stus : 1;
            unsigned txf_underrun_int_stus : 1;
            unsigned rxf_thres_int_stus : 1;
            unsigned txf_thres_int_stus : 1;
            unsigned trans_end_int_stus : 1;
            unsigned slave_cmd_int_stus : 1;
            unsigned reserved : 2;
        } __attribute__((packed)) int_status_bit;
        uint8_t int_status; // address: 0x8bffff2a, offset: 0x2a
    };
    uint8_t reserved3[0x65]; // address: 0x8bffff2b, offset: 0x2b
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
            uint8_t read_fmt; // address: 0x8bffff90, offset: 0x90
        };
        union {
            struct {
                unsigned dummy_cnt : 4;
                unsigned transmode : 4;
            } __attribute__((packed)) read_transmode_bit;
            uint8_t read_transmode; // address: 0x8bffff91, offset: 0x91
        };
        union {
            struct {
                unsigned cmd1_en : 1;
                unsigned token_val_sel : 1;
                unsigned token_en : 1;
                unsigned dummy_cnt_add : 1;
                unsigned ddr_mode : 1;
                unsigned page_mode_en : 1;
                unsigned timeout_mode_en : 1;
                unsigned tcem_mode_en : 1;
            } __attribute__((packed)) read_control0_bit;
            uint8_t read_control0; // address: 0x8bffff92, offset: 0x92
        };
        uint8_t read_cmd; // address: 0x8bffff93, offset: 0x93
        union {
            struct {
                unsigned data_lane : 2;
                unsigned addr_len : 2;
                unsigned addr_fmt : 1;
                unsigned addr_en : 1;
                unsigned cmd_fmt : 1;
                unsigned cmd_en : 1;
            } __attribute__((packed)) write_fmt_bit;
            uint8_t write_fmt; // address: 0x8bffff94, offset: 0x94
        };
        union {
            struct {
                unsigned dummy_cnt : 4;
                unsigned transmode : 4;
            } __attribute__((packed)) write_transmode_bit;
            uint8_t write_transmode; // address: 0x8bffff95, offset: 0x95
        };
        union {
            struct {
                unsigned cmd1_en : 1;
                unsigned reserved : 7;
            } __attribute__((packed)) write_control0_bit;
            uint8_t write_control0; // address: 0x8bffff96, offset: 0x96
        };
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
    uint8_t reserved4[0x3]; // address: 0x8bffffb1, offset: 0xb1
    struct {
        uint8_t read_cmd1; // address: 0x8bffffb4, offset: 0xb4
        uint8_t write_cmd1; // address: 0x8bffffb5, offset: 0xb5
    } __attribute__((packed)) xip_cmd[4]; // address: 0x8bffffb4, offset: 0xb4
} tlk_gspi_reg_t;

#define TLK_GSPI_BASE_ADDR (0x8bffff00U)
#define tlk_gspi_reg (*(volatile tlk_gspi_reg_t *) TLK_GSPI_BASE_ADDR)

#endif

