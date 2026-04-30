#ifndef TLK_PWM_REGISTERS_H_
#define TLK_PWM_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned pwm : 5;
            unsigned reserved1 : 2;
        } __attribute__((packed)) en_bit;
        uint8_t en; // address: 0x80140400, offset: 0x0
    };
    union {
        struct {
            unsigned pwm0 : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) en0_bit;
        uint8_t en0; // address: 0x80140401, offset: 0x1
    };
    uint8_t clkdiv; // address: 0x80140402, offset: 0x2
    union {
        struct {
            unsigned crun_o : 1;
            unsigned catch_o : 1;
            unsigned fifo_mode_en : 1;
            unsigned tx_dma_en : 1;
            unsigned out_sel : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) mode_bit;
        uint8_t mode; // address: 0x80140403, offset: 0x3
    };
    union {
        struct {
            unsigned inv : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) cc0_bit;
        uint8_t cc0; // address: 0x80140404, offset: 0x4
    };
    union {
        struct {
            unsigned pos : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) cc1_bit;
        uint8_t cc1; // address: 0x80140405, offset: 0x5
    };
    union {
        struct {
            unsigned pola_o : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) cc2_bit;
        uint8_t cc2; // address: 0x80140406, offset: 0x6
    };
    union {
        struct {
            unsigned mode32k : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) mode32k_bit;
        uint8_t mode32k; // address: 0x80140407, offset: 0x7
    };
    uint16_t phase[6]; // address: 0x80140408, offset: 0x8
    struct {
        uint16_t compare; // address: 0x80140414, offset: 0x14
        uint16_t max; // address: 0x80140416, offset: 0x16
    } __attribute__((packed)) time_control[6]; // address: 0x80140414, offset: 0x14
    union {
        struct {
            unsigned value : 14;
            unsigned reserved : 2;
        } __attribute__((packed)) pnum_bit;
        uint16_t pnum; // address: 0x8014042c, offset: 0x2c
    };
    union {
        struct {
            unsigned center_align : 6;
            unsigned auto_txclr_off : 1;
            unsigned txf_nempty_en : 1;
        } __attribute__((packed)) center_bit;
        uint8_t center; // address: 0x8014042e, offset: 0x2e
    };
    uint8_t reserved0; // address: 0x8014042f, offset: 0x2f
    union {
        struct {
            unsigned mask_pwm : 1;
            unsigned mask_fifo : 1;
            unsigned mask : 6;
        } __attribute__((packed)) mask0_bit;
        uint8_t mask0; // address: 0x80140430, offset: 0x30
    };
    union {
        struct {
            unsigned mask_lvl : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) mask1_bit;
        uint8_t mask1; // address: 0x80140431, offset: 0x31
    };
    union {
        struct {
            unsigned count_model_irq : 1;
            unsigned fifo_done_irq : 1;
            unsigned pwm_irq : 6;
        } __attribute__((packed)) irq_bit;
        uint8_t irq; // address: 0x80140432, offset: 0x32
    };
    union {
        struct {
            unsigned irq_lvl : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) irq_lvl_bit;
        uint8_t irq_lvl; // address: 0x80140433, offset: 0x33
    };
    uint16_t cnt[6]; // address: 0x80140434, offset: 0x34
    union {
        struct {
            unsigned numcnt_i : 14;
            unsigned reserved : 2;
        } __attribute__((packed)) ncnt_bit;
        uint16_t ncnt; // address: 0x80140440, offset: 0x40
    };
    union {
        struct {
            unsigned load_32k_config : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) control_32k_bit;
        uint8_t control_32k; // address: 0x80140442, offset: 0x42
    };
    uint8_t reserved1; // address: 0x80140443, offset: 0x43
    uint16_t tcmp_fsk; // address: 0x80140444, offset: 0x44
    uint16_t tmax_fsk; // address: 0x80140446, offset: 0x46
    union {
        struct {
            unsigned tx_data_num0 : 14;
            unsigned fsk_sel0 : 1;
            unsigned carryb0 : 1;
        } __attribute__((packed)) read_data0_bit;
        uint16_t read_data0; // address: 0x80140448, offset: 0x48
    };
    union {
        struct {
            unsigned tx_data_num1 : 14;
            unsigned fsk_sel1 : 1;
            unsigned carryb1 : 1;
        } __attribute__((packed)) read_data1_bit;
        uint16_t read_data1; // address: 0x8014044a, offset: 0x4a
    };
    union {
        struct {
            unsigned fifo_level : 4;
            unsigned reserved : 4;
        } __attribute__((packed)) fifo_level_bit;
        uint8_t fifo_level; // address: 0x8014044c, offset: 0x4c
    };
    union {
        struct {
            unsigned tx_buf_cnt : 4;
            unsigned tx_empty : 1;
            unsigned tx_full : 1;
            unsigned reserved : 2;
        } __attribute__((packed)) tx_control_bit;
        uint8_t tx_control; // address: 0x8014044d, offset: 0x4d
    };
    union {
        struct {
            unsigned clear : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) tx_fifo_clear_bit;
        uint8_t tx_fifo_clear; // address: 0x8014044e, offset: 0x4e
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
        uint8_t pem_control; // address: 0x8014044f, offset: 0x4f
    };
    uint16_t dead_time[6]; // address: 0x80140450, offset: 0x50
} tlk_pwm_reg_t;

#define TLK_PWM_BASE_ADDR (0x80140400U)
#define tlk_pwm_reg (*(volatile tlk_pwm_reg_t *) TLK_PWM_BASE_ADDR)

#endif

