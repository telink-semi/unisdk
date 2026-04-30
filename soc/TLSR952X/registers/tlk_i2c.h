#ifndef TLK_I2C_REGISTERS_H_
#define TLK_I2C_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t clock_speed; // address: 0x80140280, offset: 0x0
    uint8_t id; // address: 0x80140281, offset: 0x1
    union {
        struct {
            unsigned busy : 1;
            unsigned packet_busy : 1;
            unsigned received_status : 1;
            unsigned state_machine : 3;
            unsigned slave_state_machine : 2;
        } __attribute__((packed)) master_bit;
        uint8_t master; // address: 0x80140282, offset: 0x2
    };
    union {
        struct {
            unsigned slave_wr_irq_en : 1;
            unsigned master_nak_irq_en : 1;
            unsigned rx_buf_irq_en : 1;
            unsigned tx_buf_irq_en : 1;
            unsigned rxdone_irq_en : 1;
            unsigned txdone_en : 1;
            unsigned rxend_en : 1;
            unsigned txend_en : 1;
        } __attribute__((packed)) irq_mask_bit;
        uint8_t irq_mask; // address: 0x80140283, offset: 0x3
    };
    union {
        struct {
            unsigned launch_id : 1;
            unsigned launch_address : 1;
            unsigned launch_data_write : 1;
            unsigned launch_data_read : 1;
            unsigned launch_start : 1;
            unsigned launch_stop : 1;
            unsigned enable_read_id : 1;
            unsigned enable_ack_in_read : 1;
        } __attribute__((packed)) sct1_bit;
        uint8_t sct1; // address: 0x80140284, offset: 0x4
    };
    union {
        struct {
            unsigned rx_irq_trig_level : 4;
            unsigned tx_irq_trig_level : 4;
        } __attribute__((packed)) trig_bit;
        uint8_t trig; // address: 0x80140285, offset: 0x5
    };
    union {
        struct {
            unsigned i2c_master_en : 1;
            unsigned clk_stretch_en : 1;
            unsigned manual_tx_stop_en : 1;
            unsigned manual_rx_stop_en : 1;
            unsigned nak_stop_en : 1;
            unsigned tx_stretch_sel : 1;
            unsigned slave_stretch_irq_en : 1;
            unsigned reserved : 1;
        } __attribute__((packed)) sct2_bit;
        uint8_t sct2; // address: 0x80140286, offset: 0x6
    };
    union {
        struct {
            unsigned slave_auto_stretch_clk_en : 1;
            unsigned reserved : 1;
            unsigned master_nak_en : 1;
            unsigned manual_sda_delay : 1;
            unsigned ndma_rxdone_en : 1;
            unsigned auto_rx_clear_en : 1;
            unsigned slave_stretch_max_release_en : 1;
            unsigned slave_stretch_min_release_en : 1;
        } __attribute__((packed)) control3_bit;
        uint8_t control3; // address: 0x80140287, offset: 0x7
    };
    uint8_t data_buff[4]; // address: 0x80140288, offset: 0x8
    union {
        struct {
            unsigned rx_buf_cnt : 4;
            unsigned tx_buf_cnt : 4;
        } __attribute__((packed)) buff_count_bit;
        uint8_t buff_count; // address: 0x8014028c, offset: 0xc
    };
    union {
        struct {
            unsigned rbcnt : 3;
            unsigned irq : 1;
            unsigned wbcnt : 3;
            unsigned rxdone : 1;
        } __attribute__((packed)) status_bit;
        uint8_t status; // address: 0x8014028d, offset: 0xd
    };
    union {
        struct {
            unsigned slave_rw_status : 1;
            unsigned slave_stretch_indication : 1;
            unsigned tx_empty : 1;
            unsigned rx_full : 1;
            unsigned reserved : 2;
            unsigned ss_scl_irq : 1;
            unsigned reserved1 : 1;
        } __attribute__((packed)) status1_bit;
        uint8_t status1; // address: 0x8014028e, offset: 0xe
    };
    union {
        struct {
            unsigned ss_rw_irq : 1;
            unsigned ms_nak_irq : 1;
            unsigned tx_buf_irq : 1;
            unsigned rx_buf_irq : 1;
            unsigned rx_done_irq : 1;
            unsigned tx_done : 1;
            unsigned rx_end : 1;
            unsigned tx_end : 1;
        } __attribute__((packed)) irq_status_bit;
        uint8_t irq_status; // address: 0x8014028f, offset: 0xf
    };
    uint8_t len[3]; // address: 0x80140290, offset: 0x10
    union {
        struct {
            unsigned id_status_flag : 1;
            unsigned address_status_flag : 1;
            unsigned dataw_status_flag : 1;
            unsigned datar_status_flag : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) mst_status_bit;
        uint8_t mst_status; // address: 0x80140293, offset: 0x13
    };
} tlk_i2c_reg_t;

#define TLK_I2C_BASE_ADDR (0x80140280U)
#define tlk_i2c_reg (*(volatile tlk_i2c_reg_t *) TLK_I2C_BASE_ADDR)

#endif


#ifndef TLK_I2C1M_REGISTERS_H_
#define TLK_I2C1M_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t clock_speed; // address: 0x80140480, offset: 0x0
    uint8_t id; // address: 0x80140481, offset: 0x1
    union {
        struct {
            unsigned master_busy : 1;
            unsigned master_packet_busy : 1;
            unsigned master_received_status : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) master_bit;
        uint8_t master; // address: 0x80140482, offset: 0x2
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned i2c1m_master_en : 1;
            unsigned reserved1 : 1;
            unsigned clk_stretch_en : 1;
            unsigned reserved2 : 4;
        } __attribute__((packed)) sct0_bit;
        uint8_t sct0; // address: 0x80140483, offset: 0x3
    };
    uint8_t address; // address: 0x80140484, offset: 0x4
    uint8_t data_buff; // address: 0x80140485, offset: 0x5
    uint8_t data_read; // address: 0x80140486, offset: 0x6
    union {
        struct {
            unsigned launch_id : 1;
            unsigned launch_address : 1;
            unsigned launch_data_write : 1;
            unsigned launch_data_read : 1;
            unsigned launch_start : 1;
            unsigned launch_stop : 1;
            unsigned enable_read_id : 1;
            unsigned enable_ack_in_read : 1;
        } __attribute__((packed)) sct1_bit;
        uint8_t sct1; // address: 0x80140487, offset: 0x7
    };
} tlk_i2c1m_reg_t;

#define TLK_I2C1M_BASE_ADDR (0x80140480U)
#define tlk_i2c1m_reg (*(volatile tlk_i2c1m_reg_t *) TLK_I2C1M_BASE_ADDR)

#endif

