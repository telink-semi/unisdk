#ifndef TLK_I2C_REGISTERS_H_
#define TLK_I2C_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t clock_speed; // address: 0x80140280, offset: 0x0
    uint8_t id; // address: 0x80140281, offset: 0x1
    union {
        struct {
            unsigned master_busy : 1;
            unsigned master_scs_n : 1;
            unsigned master_ack_in : 1;
            unsigned master_p : 3;
            unsigned ss : 2;
        } __attribute__((packed)) master_bit;
        uint8_t master; // address: 0x80140282, offset: 0x2
    };
    union {
        struct {
            unsigned mask_slave_wr : 1;
            unsigned mask_master_nak : 1;
            unsigned rx_irq_en : 1;
            unsigned tx_irq_en : 1;
            unsigned mask_txdone : 1;
            unsigned mask_rxdone : 1;
            unsigned mask_rxend : 1;
            unsigned mask_txend : 1;
        } __attribute__((packed)) sct0_bit;
        uint8_t sct0; // address: 0x80140283, offset: 0x3
    };
    union {
        struct {
            unsigned launch_id : 1;
            unsigned launch_addr : 1;
            unsigned launch_data_w : 1;
            unsigned launch_data_r : 1;
            unsigned launch_start : 1;
            unsigned launch_stop : 1;
            unsigned en_read_id : 1;
            unsigned en_ack_read : 1;
        } __attribute__((packed)) sct1_bit;
        uint8_t sct1; // address: 0x80140284, offset: 0x4
    };
    union {
        struct {
            unsigned rx_irq_trig : 4;
            unsigned tx_irq_trig : 4;
        } __attribute__((packed)) trig_bit;
        uint8_t trig; // address: 0x80140285, offset: 0x5
    };
    union {
        struct {
            unsigned master_en : 1;
            unsigned r_clk_stretch_en : 1;
            unsigned manual_tx_stop_hit : 1;
            unsigned manual_rx_stop_hit : 1;
            unsigned nak_stop_en : 1;
            unsigned tx_stretch_sel : 1;
            unsigned mask_stretch : 1;
            unsigned r_stretch_pos_sel : 1;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0x80140286, offset: 0x6
    };
    union {
        struct {
            unsigned r_clk_stretch_sen : 1;
            unsigned r_id_nmatch_stop_en : 1;
            unsigned r_ms_nak_en : 1;
            unsigned manual_sda_delay : 1;
            unsigned ndma_rxdone_en : 1;
            unsigned auto_rxclr_en : 1;
            unsigned r_hs_mode : 1;
            unsigned r_fast_mode : 1;
        } __attribute__((packed)) control3_bit;
        uint8_t control3; // address: 0x80140287, offset: 0x7
    };
    uint8_t data_buff[4]; // address: 0x80140288, offset: 0x8
    union {
        struct {
            unsigned rx_buf_cnt : 4;
            unsigned tx_buf_cnt : 4;
        } __attribute__((packed)) bufcnt_bit;
        uint8_t bufcnt; // address: 0x8014028c, offset: 0xc
    };
    union {
        struct {
            unsigned rbcnt_or_clr : 3;
            unsigned i2c_irq_o : 1;
            unsigned wbcnt : 3;
            unsigned rxdone : 1;
        } __attribute__((packed)) status_bit;
        uint8_t status; // address: 0x8014028d, offset: 0xd
    };
    union {
        struct {
            unsigned ss_read : 1;
            unsigned ss_scl : 1;
            unsigned tx_empty : 1;
            unsigned rx_full : 1;
            unsigned reserved0 : 2;
            unsigned ss_scl_irq : 1;
            unsigned reserved1 : 1;
        } __attribute__((packed)) status1_bit;
        uint8_t status1; // address: 0x8014028e, offset: 0xe
    };
    union {
        struct {
            unsigned ss_rw_clr : 1;
            unsigned ms_nak_clr : 1;
            unsigned rx_clr : 1;
            unsigned tx_clr : 1;
            unsigned rxdone_irq_clr : 1;
            unsigned txdone_clr : 1;
            unsigned rx_end_clr : 1;
            unsigned tx_end_clr : 1;
        } __attribute__((packed)) clear_bit;
        uint8_t clear; // address: 0x8014028f, offset: 0xf
    };
    uint8_t len[3]; // address: 0x80140290, offset: 0x10
    union {
        struct {
            unsigned pem_event_en : 1;
            unsigned reserved0 : 1;
            unsigned pem_event_sel : 1;
            unsigned reserved1 : 1;
            unsigned mask_trx_start : 1;
            unsigned mask_trx_stop : 1;
            unsigned r_stretch_auto_clr_dis : 1;
            unsigned reserved2 : 1;
        } __attribute__((packed)) control1_bit;
        uint8_t control1; // address: 0x80140293, offset: 0x13
    };
    union {
        struct {
            unsigned ss_busy : 1;
            unsigned ss_id : 1;
            unsigned ss_dataw : 1;
            unsigned ss_datar : 1;
            unsigned id_busy : 1;
            unsigned addr_busy : 1;
            unsigned dataw_busy : 1;
            unsigned datar_busy : 1;
        } __attribute__((packed)) master_status_bit;
        uint8_t master_status; // address: 0x80140294, offset: 0x14
    };
    union {
        struct {
            unsigned pem_event_en : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) control2_pem_bit;
        uint8_t control2_pem; // address: 0x80140295, offset: 0x15
    };
} tlk_i2c_reg_t;

#define TLK_I2C_BASE_ADDR (0x80140280U)
#define tlk_i2c_reg (*(volatile tlk_i2c_reg_t *) TLK_I2C_BASE_ADDR)

#endif


#ifndef TLK_I2C1_M_REGISTERS_H_
#define TLK_I2C1_M_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t clock_speed; // address: 0x80140480, offset: 0x0
    union {
        struct {
            unsigned write_read_bit : 1;
            unsigned id : 7;
        } __attribute__((packed)) id_bit;
        uint8_t id; // address: 0x80140481, offset: 0x1
    };
    union {
        struct {
            unsigned master_busy : 1;
            unsigned master_scs_n : 1;
            unsigned mat_ack_in : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) master_bit;
        uint8_t master; // address: 0x80140482, offset: 0x2
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned master : 1;
            unsigned reserved1 : 1;
            unsigned r_clk_stretch_en : 1;
            unsigned reserved2 : 4;
        } __attribute__((packed)) sct0_bit;
        uint8_t sct0; // address: 0x80140483, offset: 0x3
    };
    uint8_t address; // address: 0x80140484, offset: 0x4
    uint8_t data_write; // address: 0x80140485, offset: 0x5
    uint8_t data_read; // address: 0x80140486, offset: 0x6
    union {
        struct {
            unsigned ls_id : 1;
            unsigned ls_addr : 1;
            unsigned ls_dataw : 1;
            unsigned ls_datar : 1;
            unsigned ls_start : 1;
            unsigned ls_stop : 1;
            unsigned ls_id_r : 1;
            unsigned ls_ack : 1;
        } __attribute__((packed)) control_bit;
        uint8_t control; // address: 0x80140487, offset: 0x7
    };
} tlk_i2c1_m_reg_t;

#define TLK_I2C1_M_BASE_ADDR (0x80140480U)
#define tlk_i2c1_m_reg (*(volatile tlk_i2c1_m_reg_t *) TLK_I2C1_M_BASE_ADDR)

#endif

