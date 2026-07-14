#ifndef TLK_TLK_PWM_REGISTERS_H_
#define TLK_TLK_PWM_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_pwm_en0_e {
    TLK_PWM_EN0_PWM0 = TLK_BIT(0),
} tlk_pwm_en0_e;

typedef enum tlk_pwm_mode_e {
    TLK_PWM_MODE_CRUN_O       = TLK_BIT(0),
    TLK_PWM_MODE_CATCH_O      = TLK_BIT(1),
    TLK_PWM_MODE_FIFO_MODE_EN = TLK_BIT(2),
    TLK_PWM_MODE_TXDMA_EN     = TLK_BIT(3),
    TLK_PWM_MODE_OUT2ANA_EN   = TLK_BIT(4),
} tlk_pwm_mode_e;

typedef enum tlk_pwm_fifo_control_e {
    TLK_PWM_FIFO_CONTROL_AUTO_TXCLR_OFF = TLK_BIT(0),
    TLK_PWM_FIFO_CONTROL_TXF_NEMPTY_EN  = TLK_BIT(1),
} tlk_pwm_fifo_control_e;

typedef enum tlk_pwm_mask0_e {
    TLK_PWM_MASK0_MASK_PWM  = TLK_BIT(0),
    TLK_PWM_MASK0_MASK_FIFO = TLK_BIT(1),
} tlk_pwm_mask0_e;

typedef enum tlk_pwm_mask1_e {
    TLK_PWM_MASK1_MASK_LVL = TLK_BIT(0),
} tlk_pwm_mask1_e;

typedef enum tlk_pwm_irq0_e {
    TLK_PWM_IRQ0_PWM_IRQ       = TLK_BIT(0),
    TLK_PWM_IRQ0_FIFO_DONE_IRQ = TLK_BIT(1),
} tlk_pwm_irq0_e;

typedef enum tlk_pwm_irq1_e {
    TLK_PWM_IRQ1_IRQ_LVL = TLK_BIT(0),
} tlk_pwm_irq1_e;

typedef enum tlk_pwm_fifo_status_e {
    TLK_PWM_FIFO_STATUS_TX_EMPTY = TLK_BIT(4),
    TLK_PWM_FIFO_STATUS_TX_FULL  = TLK_BIT(5),
} tlk_pwm_fifo_status_e;

typedef enum tlk_pwm_tx_fifo_load_e {
    TLK_PWM_TX_FIFO_LOAD_TX_CLR     = TLK_BIT(0),
    TLK_PWM_TX_FIFO_LOAD_RELOAD_PUL = TLK_BIT(1),
} tlk_pwm_tx_fifo_load_e;

typedef enum tlk_pwm_pem_control_e {
    TLK_PWM_PEM_CONTROL_PEM_EVENT1_SEL = TLK_BIT(0),
    TLK_PWM_PEM_CONTROL_PEM_EVENT1_EN  = TLK_BIT(1),
    TLK_PWM_PEM_CONTROL_PEM_EVENT0_EN  = TLK_BIT(2),
    TLK_PWM_PEM_CONTROL_PEM_TASK1_EN   = TLK_BIT(3),
    TLK_PWM_PEM_CONTROL_PEM_TASK0_EN   = TLK_BIT(4),
} tlk_pwm_pem_control_e;

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned pwm : 6;
            unsigned reserved1 : 1;
        } __attribute__((packed)) en_bit;
        uint8_t en; // address: 0x80140400, offset: 0x0
    };
    union {
        struct {
            unsigned pwm0 : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) en0_bit;
        struct {
            enum tlk_pwm_en0_e __attribute__((packed)) en0 : 8;
        }; // address: 0x80140401, offset: 0x1
    };
    uint8_t clkdiv; // address: 0x80140402, offset: 0x2
    union {
        struct {
            unsigned crun_o : 1;
            unsigned catch_o : 1;
            unsigned fifo_mode_en : 1;
            unsigned txdma_en : 1;
            unsigned out2ana_en : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) mode_bit;
        struct {
            enum tlk_pwm_mode_e __attribute__((packed)) mode : 8;
        }; // address: 0x80140403, offset: 0x3
    };
    union {
        struct {
            unsigned inv : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) cc0_bit;
        uint8_t cc0; // address: 0x80140404, offset: 0x4
    };
    union {
        struct {
            unsigned pos : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) cc1_bit;
        uint8_t cc1; // address: 0x80140405, offset: 0x5
    };
    union {
        struct {
            unsigned pola : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) cc2_bit;
        uint8_t cc2; // address: 0x80140406, offset: 0x6
    };
    union {
        struct {
            unsigned value : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) mode32k_bit;
        uint8_t mode32k; // address: 0x80140407, offset: 0x7
    };
    struct {
        uint16_t compare; // address: 0x80140408, offset: 0x8
        uint16_t max; // address: 0x8014040a, offset: 0xa
    } __attribute__((packed)) time_control[7]; // address: 0x80140408, offset: 0x8
    uint16_t phase[7]; // address: 0x80140424, offset: 0x24
    union {
        struct {
            unsigned pnumb : 14;
            unsigned reserved : 2;
        } __attribute__((packed)) pnum_bit;
        uint16_t pnum; // address: 0x80140432, offset: 0x32
    };
    union {
        struct {
            unsigned center_align_o : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) center_bit;
        uint8_t center; // address: 0x80140434, offset: 0x34
    };
    union {
        struct {
            unsigned auto_txclr_off : 1;
            unsigned txf_nempty_en : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) fifo_control_bit;
        struct {
            enum tlk_pwm_fifo_control_e __attribute__((packed)) fifo_control : 8;
        }; // address: 0x80140435, offset: 0x35
    };
    union {
        struct {
            unsigned mask_pwm : 1;
            unsigned mask_fifo : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) mask0_bit;
        struct {
            enum tlk_pwm_mask0_e __attribute__((packed)) mask0 : 8;
        }; // address: 0x80140436, offset: 0x36
    };
    union {
        struct {
            unsigned mask_lvl : 1;
            unsigned mask : 7;
        } __attribute__((packed)) mask1_bit;
        struct {
            enum tlk_pwm_mask1_e __attribute__((packed)) mask1 : 8;
        }; // address: 0x80140437, offset: 0x37
    };
    union {
        struct {
            unsigned pwm_irq : 1;
            unsigned fifo_done_irq : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) irq0_bit;
        struct {
            enum tlk_pwm_irq0_e __attribute__((packed)) irq0 : 8;
        }; // address: 0x80140438, offset: 0x38
    };
    union {
        struct {
            unsigned irq_lvl : 1;
            unsigned irq_flag : 7;
        } __attribute__((packed)) irq1_bit;
        struct {
            enum tlk_pwm_irq1_e __attribute__((packed)) irq1 : 8;
        }; // address: 0x80140439, offset: 0x39
    };
    uint16_t cnt[7]; // address: 0x8014043a, offset: 0x3a
    uint16_t data0; // address: 0x80140448, offset: 0x48
    uint16_t data1; // address: 0x8014044a, offset: 0x4a
    uint16_t tcmp_fsk; // address: 0x8014044c, offset: 0x4c
    uint16_t tmax_fsk; // address: 0x8014044e, offset: 0x4e
    uint16_t phase_fsk; // address: 0x80140450, offset: 0x50
    union {
        struct {
            unsigned numcnt_i : 14;
            unsigned reserved : 2;
        } __attribute__((packed)) ncnt_bit;
        uint16_t ncnt; // address: 0x80140452, offset: 0x52
    };
    union {
        struct {
            unsigned fifo_lvl : 4;
            unsigned reserved : 4;
        } __attribute__((packed)) fifo_lvl_bit;
        uint8_t fifo_lvl; // address: 0x80140454, offset: 0x54
    };
    union {
        struct {
            unsigned tx_buf_cnt : 4;
            unsigned tx_empty : 1;
            unsigned tx_full : 1;
            unsigned reserved : 2;
        } __attribute__((packed)) fifo_status_bit;
        struct {
            enum tlk_pwm_fifo_status_e __attribute__((packed)) fifo_status : 8;
        }; // address: 0x80140455, offset: 0x55
    };
    union {
        struct {
            unsigned tx_clr : 1;
            unsigned reload_pul : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) tx_fifo_load_bit;
        struct {
            enum tlk_pwm_tx_fifo_load_e __attribute__((packed)) tx_fifo_load : 8;
        }; // address: 0x80140456, offset: 0x56
    };
    union {
        struct {
            unsigned value : 5;
            unsigned reserved : 3;
        } __attribute__((packed)) resol_bit[7];
        uint8_t resol[7]; // address: 0x80140457, offset: 0x57
    };
    union {
        struct {
            unsigned sync_en : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) sync_en_bit;
        uint8_t sync_en; // address: 0x8014045e, offset: 0x5e
    };
    union {
        struct {
            unsigned sync_edge : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) sync_edge_bit;
        uint8_t sync_edge; // address: 0x8014045f, offset: 0x5f
    };
    union {
        struct {
            unsigned pem_event1_sel : 1;
            unsigned pem_event1_en : 1;
            unsigned pem_event0_en : 1;
            unsigned pem_task1_en : 1;
            unsigned pem_task0_en : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) pem_control_bit;
        struct {
            enum tlk_pwm_pem_control_e __attribute__((packed)) pem_control : 8;
        }; // address: 0x80140460, offset: 0x60
    };
    union {
        struct {
            unsigned phase_mode : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) phase_mode_bit;
        uint8_t phase_mode; // address: 0x80140461, offset: 0x61
    };
    uint16_t dead_time[7]; // address: 0x80140462, offset: 0x62
} tlk_pwm_reg_t;

#define TLK_PWM_BASE_ADDR (0x80140400U)
#define tlk_pwm_reg (*(volatile tlk_pwm_reg_t *) TLK_PWM_BASE_ADDR)

#endif