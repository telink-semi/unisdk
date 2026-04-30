#ifndef TLK_PWM_REGISTERS_H_
#define TLK_PWM_REGISTERS_H_

#include <stdint.h>

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
        uint8_t en0; // address: 0x80140401, offset: 0x1
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
        uint8_t mode; // address: 0x80140403, offset: 0x3
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
        uint8_t fifo_control; // address: 0x80140435, offset: 0x35
    };
    union {
        struct {
            unsigned mask_pwm : 1;
            unsigned mask_fifo : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) mask0_bit;
        uint8_t mask0; // address: 0x80140436, offset: 0x36
    };
    union {
        struct {
            unsigned mask_lvl : 1;
            unsigned mask : 7;
        } __attribute__((packed)) mask1_bit;
        uint8_t mask1; // address: 0x80140437, offset: 0x37
    };
    union {
        struct {
            unsigned pwm_irq : 1;
            unsigned fifo_done_irq : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) irq0_bit;
        uint8_t irq0; // address: 0x80140438, offset: 0x38
    };
    union {
        struct {
            unsigned irq_lvl : 1;
            unsigned irq_flag : 7;
        } __attribute__((packed)) irq1_bit;
        uint8_t irq1; // address: 0x80140439, offset: 0x39
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
        uint8_t fifo_status; // address: 0x80140455, offset: 0x55
    };
    union {
        struct {
            unsigned tx_clr : 1;
            unsigned reload_pul : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) tx_fifo_load_bit;
        uint8_t tx_fifo_load; // address: 0x80140456, offset: 0x56
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
        uint8_t pem_control; // address: 0x80140460, offset: 0x60
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

