#ifndef TLK_TLK_I2C_REGISTERS_H_
#define TLK_TLK_I2C_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_i2c0_master_e {
    TLK_I2C0_MASTER_BUSY        = TLK_BIT(0),
    TLK_I2C0_MASTER_PACKET_BUSY = TLK_BIT(1),
    TLK_I2C0_MASTER_ACK_IN      = TLK_BIT(2),
} tlk_i2c0_master_e;

typedef enum tlk_i2c0_irq_mask_e {
    TLK_I2C0_IRQ_MASK_SLAVE_WR   = TLK_BIT(0),
    TLK_I2C0_IRQ_MASK_MASTER_NAK = TLK_BIT(1),
    TLK_I2C0_IRQ_MASK_RX_BUF     = TLK_BIT(2),
    TLK_I2C0_IRQ_MASK_TX_BUF     = TLK_BIT(3),
    TLK_I2C0_IRQ_MASK_RX_DONE    = TLK_BIT(4),
    TLK_I2C0_IRQ_MASK_TX_DONE    = TLK_BIT(5),
    TLK_I2C0_IRQ_MASK_RX_END     = TLK_BIT(6),
    TLK_I2C0_IRQ_MASK_TX_END     = TLK_BIT(7),
} tlk_i2c0_irq_mask_e;

typedef enum tlk_i2c0_command_e {
    TLK_I2C0_COMMAND_LAUNCH_ID     = TLK_BIT(0),
    TLK_I2C0_COMMAND_LAUNCH_ADDR   = TLK_BIT(1),
    TLK_I2C0_COMMAND_LAUNCH_DATA_W = TLK_BIT(2),
    TLK_I2C0_COMMAND_LAUNCH_DATA_R = TLK_BIT(3),
    TLK_I2C0_COMMAND_LAUNCH_START  = TLK_BIT(4),
    TLK_I2C0_COMMAND_LAUNCH_STOP   = TLK_BIT(5),
    TLK_I2C0_COMMAND_EN_READ_ID    = TLK_BIT(6),
    TLK_I2C0_COMMAND_EN_ACK_READ   = TLK_BIT(7),
} tlk_i2c0_command_e;

typedef enum tlk_i2c0_control2_e {
    TLK_I2C0_CONTROL2_MASTER_EN            = TLK_BIT(0),
    TLK_I2C0_CONTROL2_CLK_STRETCH_EN       = TLK_BIT(1),
    TLK_I2C0_CONTROL2_MANUAL_TX_STOP_EN    = TLK_BIT(2),
    TLK_I2C0_CONTROL2_MANUAL_RX_STOP_EN    = TLK_BIT(3),
    TLK_I2C0_CONTROL2_NAK_STOP_EN          = TLK_BIT(4),
    TLK_I2C0_CONTROL2_TX_STRETCH_SEL       = TLK_BIT(5),
    TLK_I2C0_CONTROL2_SLAVE_STRETCH_IRQ_EN = TLK_BIT(6),
} tlk_i2c0_control2_e;

typedef enum tlk_i2c0_control3_e {
    TLK_I2C0_CONTROL3_SLAVE_CLK_STRETCH_EN         = TLK_BIT(0),
    TLK_I2C0_CONTROL3_MASTER_NAK_EN                = TLK_BIT(2),
    TLK_I2C0_CONTROL3_MANUAL_SDA_DELAY             = TLK_BIT(3),
    TLK_I2C0_CONTROL3_NDMA_RXDONE_EN               = TLK_BIT(4),
    TLK_I2C0_CONTROL3_AUTO_RX_CLEAR_EN             = TLK_BIT(5),
    TLK_I2C0_CONTROL3_SLAVE_STRETCH_MAX_RELEASE_EN = TLK_BIT(6),
    TLK_I2C0_CONTROL3_SLAVE_STRETCH_MIN_RELEASE_EN = TLK_BIT(7),
} tlk_i2c0_control3_e;

typedef enum tlk_i2c0_status_e {
    TLK_I2C0_STATUS_IRQ    = TLK_BIT(3),
    TLK_I2C0_STATUS_RXDONE = TLK_BIT(7),
} tlk_i2c0_status_e;

typedef enum tlk_i2c0_status1_e {
    TLK_I2C0_STATUS1_SLAVE_RW_STATUS          = TLK_BIT(0),
    TLK_I2C0_STATUS1_SLAVE_STRETCH_INDICATION = TLK_BIT(1),
    TLK_I2C0_STATUS1_TX_EMPTY                 = TLK_BIT(2),
    TLK_I2C0_STATUS1_RX_FULL                  = TLK_BIT(3),
    TLK_I2C0_STATUS1_SS_SCL_IRQ               = TLK_BIT(6),
} tlk_i2c0_status1_e;

typedef enum tlk_i2c0_irq_status_e {
    TLK_I2C0_IRQ_STATUS_SS_RW   = TLK_BIT(0),
    TLK_I2C0_IRQ_STATUS_MS_NAK  = TLK_BIT(1),
    TLK_I2C0_IRQ_STATUS_RX_BUF  = TLK_BIT(2),
    TLK_I2C0_IRQ_STATUS_TX_BUF  = TLK_BIT(3),
    TLK_I2C0_IRQ_STATUS_RX_DONE = TLK_BIT(4),
    TLK_I2C0_IRQ_STATUS_TX_DONE = TLK_BIT(5),
    TLK_I2C0_IRQ_STATUS_RX_END  = TLK_BIT(6),
    TLK_I2C0_IRQ_STATUS_TX_END  = TLK_BIT(7),
} tlk_i2c0_irq_status_e;

typedef enum tlk_i2c0_mst_status_e {
    TLK_I2C0_MST_STATUS_ID_STATUS_FLAG      = TLK_BIT(0),
    TLK_I2C0_MST_STATUS_ADDRESS_STATUS_FLAG = TLK_BIT(1),
    TLK_I2C0_MST_STATUS_DATAW_STATUS_FLAG   = TLK_BIT(2),
    TLK_I2C0_MST_STATUS_DATAR_STATUS_FLAG   = TLK_BIT(3),
} tlk_i2c0_mst_status_e;

typedef struct __attribute__((packed)) {
    uint8_t clock_speed; // address: 0x80140280, offset: 0x0
    uint8_t id; // address: 0x80140281, offset: 0x1
    union {
        struct {
            unsigned busy : 1;
            unsigned packet_busy : 1;
            unsigned ack_in : 1;
            unsigned state_machine : 3;
            unsigned slave_state_machine : 2;
        } __attribute__((packed)) master_bit;
        struct {
            enum tlk_i2c0_master_e __attribute__((packed)) master : 8;
        }; // address: 0x80140282, offset: 0x2
    };
    union {
        struct {
            unsigned slave_wr : 1;
            unsigned master_nak : 1;
            unsigned rx_buf : 1;
            unsigned tx_buf : 1;
            unsigned rx_done : 1;
            unsigned tx_done : 1;
            unsigned rx_end : 1;
            unsigned tx_end : 1;
        } __attribute__((packed)) irq_mask_bit;
        struct {
            enum tlk_i2c0_irq_mask_e __attribute__((packed)) irq_mask : 8;
        }; // address: 0x80140283, offset: 0x3
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
        } __attribute__((packed)) command_bit;
        struct {
            enum tlk_i2c0_command_e __attribute__((packed)) command : 8;
        }; // address: 0x80140284, offset: 0x4
    };
    union {
        struct {
            unsigned rx_irq_trig_lvl : 4;
            unsigned tx_irq_trig_lvl : 4;
        } __attribute__((packed)) trig_bit;
        uint8_t trig; // address: 0x80140285, offset: 0x5
    };
    union {
        struct {
            unsigned master_en : 1;
            unsigned clk_stretch_en : 1;
            unsigned manual_tx_stop_en : 1;
            unsigned manual_rx_stop_en : 1;
            unsigned nak_stop_en : 1;
            unsigned tx_stretch_sel : 1;
            unsigned slave_stretch_irq_en : 1;
            unsigned reserved : 1;
        } __attribute__((packed)) control2_bit;
        struct {
            enum tlk_i2c0_control2_e __attribute__((packed)) control2 : 8;
        }; // address: 0x80140286, offset: 0x6
    };
    union {
        struct {
            unsigned slave_clk_stretch_en : 1;
            unsigned reserved : 1;
            unsigned master_nak_en : 1;
            unsigned manual_sda_delay : 1;
            unsigned ndma_rxdone_en : 1;
            unsigned auto_rx_clear_en : 1;
            unsigned slave_stretch_max_release_en : 1;
            unsigned slave_stretch_min_release_en : 1;
        } __attribute__((packed)) control3_bit;
        struct {
            enum tlk_i2c0_control3_e __attribute__((packed)) control3 : 8;
        }; // address: 0x80140287, offset: 0x7
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
        struct {
            enum tlk_i2c0_status_e __attribute__((packed)) status : 8;
        }; // address: 0x8014028d, offset: 0xd
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
        struct {
            enum tlk_i2c0_status1_e __attribute__((packed)) status1 : 8;
        }; // address: 0x8014028e, offset: 0xe
    };
    union {
        struct {
            unsigned ss_rw : 1;
            unsigned ms_nak : 1;
            unsigned rx_buf : 1;
            unsigned tx_buf : 1;
            unsigned rx_done : 1;
            unsigned tx_done : 1;
            unsigned rx_end : 1;
            unsigned tx_end : 1;
        } __attribute__((packed)) irq_status_bit;
        struct {
            enum tlk_i2c0_irq_status_e __attribute__((packed)) irq_status : 8;
        }; // address: 0x8014028f, offset: 0xf
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
        struct {
            enum tlk_i2c0_mst_status_e __attribute__((packed)) mst_status : 8;
        }; // address: 0x80140293, offset: 0x13
    };
} tlk_i2c0_reg_t;

#define TLK_I2C0_BASE_ADDR (0x80140280U)
#define tlk_i2c0_reg (*(volatile tlk_i2c0_reg_t *) TLK_I2C0_BASE_ADDR)



typedef enum tlk_i2c1_id_e {
    TLK_I2C1_ID_WRITE_READ_BIT = TLK_BIT(0),
} tlk_i2c1_id_e;

typedef enum tlk_i2c1_master_e {
    TLK_I2C1_MASTER_BUSY               = TLK_BIT(0),
    TLK_I2C1_MASTER_MASTER_PACKET_BUSY = TLK_BIT(1),
    TLK_I2C1_MASTER_ACK_IN             = TLK_BIT(2),
} tlk_i2c1_master_e;

typedef enum tlk_i2c1_control_e {
    TLK_I2C1_CONTROL_MASTER_EN      = TLK_BIT(1),
    TLK_I2C1_CONTROL_CLK_STRETCH_EN = TLK_BIT(3),
} tlk_i2c1_control_e;

typedef enum tlk_i2c1_command_e {
    TLK_I2C1_COMMAND_LAUNCH_ID     = TLK_BIT(0),
    TLK_I2C1_COMMAND_LAUNCH_ADDR   = TLK_BIT(1),
    TLK_I2C1_COMMAND_LAUNCH_DATA_W = TLK_BIT(2),
    TLK_I2C1_COMMAND_LAUNCH_DATA_R = TLK_BIT(3),
    TLK_I2C1_COMMAND_LAUNCH_START  = TLK_BIT(4),
    TLK_I2C1_COMMAND_LAUNCH_STOP   = TLK_BIT(5),
    TLK_I2C1_COMMAND_EN_READ_ID    = TLK_BIT(6),
    TLK_I2C1_COMMAND_EN_ACK_READ   = TLK_BIT(7),
} tlk_i2c1_command_e;

typedef struct __attribute__((packed)) {
    uint8_t clock_speed; // address: 0x80140480, offset: 0x0
    union {
        struct {
            unsigned write_read_bit : 1;
            unsigned id : 7;
        } __attribute__((packed)) id_bit;
        struct {
            enum tlk_i2c1_id_e __attribute__((packed)) id : 8;
        }; // address: 0x80140481, offset: 0x1
    };
    union {
        struct {
            unsigned busy : 1;
            unsigned master_packet_busy : 1;
            unsigned ack_in : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) master_bit;
        struct {
            enum tlk_i2c1_master_e __attribute__((packed)) master : 8;
        }; // address: 0x80140482, offset: 0x2
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned master_en : 1;
            unsigned reserved1 : 1;
            unsigned clk_stretch_en : 1;
            unsigned reserved2 : 4;
        } __attribute__((packed)) control_bit;
        struct {
            enum tlk_i2c1_control_e __attribute__((packed)) control : 8;
        }; // address: 0x80140483, offset: 0x3
    };
    uint8_t address; // address: 0x80140484, offset: 0x4
    uint8_t data_write; // address: 0x80140485, offset: 0x5
    uint8_t data_read; // address: 0x80140486, offset: 0x6
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
        } __attribute__((packed)) command_bit;
        struct {
            enum tlk_i2c1_command_e __attribute__((packed)) command : 8;
        }; // address: 0x80140487, offset: 0x7
    };
} tlk_i2c1_reg_t;

#define TLK_I2C1_BASE_ADDR (0x80140480U)
#define tlk_i2c1_reg (*(volatile tlk_i2c1_reg_t *) TLK_I2C1_BASE_ADDR)

#endif