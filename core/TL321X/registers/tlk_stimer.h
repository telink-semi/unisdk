#ifndef TLK_STIMER_REGISTERS_H_
#define TLK_STIMER_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t current_time; // address: 0x80140200, offset: 0x0
    uint32_t irq_time; // address: 0x80140204, offset: 0x4
    union {
        struct {
            unsigned timer_irq_en : 1;
            unsigned cal_irq_en : 1;
            unsigned sync_wait_irq_en : 1;
            unsigned trig_past_en : 1;
            unsigned capt_irq_en : 1;
            unsigned capt_ov_irq_en : 1;
            unsigned reserved : 2;
        } __attribute__((packed)) irq_mask_bit;
        uint8_t irq_mask; // address: 0x80140208, offset: 0x8
    };
    union {
        struct {
            unsigned timer_irq : 1;
            unsigned cal_irq : 1;
            unsigned capt_irq : 1;
            unsigned capt_ov_irq : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) irq_status_bit;
        uint8_t irq_status; // address: 0x80140209, offset: 0x9
    };
    union {
        struct {
            unsigned write_32k : 1;
            unsigned timer_en : 1;
            unsigned timer_auto : 1;
            unsigned cal_32k_en : 1;
            unsigned cal_32k_mode : 4;
        } __attribute__((packed)) control_bit;
        uint8_t control; // address: 0x8014020a, offset: 0xa
    };
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned cmd_stop : 1;
            unsigned reserved1 : 1;
            unsigned cmd_sync : 1;
            unsigned clk_32k : 1;
            unsigned clr_read_done : 1;
            unsigned rd_busy : 1;
            unsigned cmd_set_dly_done : 1;
        } __attribute__((packed)) state_bit;
        uint8_t state; // address: 0x8014020b, offset: 0xb
    };
    uint32_t set_32k_time; // address: 0x8014020c, offset: 0xc
    uint32_t read_32k_time; // address: 0x80140210, offset: 0x10
    uint32_t cal_latch; // address: 0x80140214, offset: 0x14
    union {
        struct {
            unsigned update_upon_32k : 1;
            unsigned run_upon_nxt_32k : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) sys_timer_up_bit;
        uint8_t sys_timer_up; // address: 0x80140218, offset: 0x18
    };
    union {
        struct {
            unsigned capt_mode : 2;
            unsigned capt_en : 1;
            unsigned pem_event_en : 1;
            unsigned pem_task_0_en : 1;
            unsigned pem_task_1_en : 1;
            unsigned pem_task_2_en : 1;
            unsigned reserved : 1;
        } __attribute__((packed)) control1_bit;
        uint8_t control1; // address: 0x80140219, offset: 0x19
    };
    uint8_t reserved0[0x2]; // address: 0x8014021a, offset: 0x1a
    uint32_t capture; // address: 0x8014021c, offset: 0x1c
} tlk_stimer_reg_t;

#define TLK_STIMER_BASE_ADDR (0x80140200U)
#define tlk_stimer_reg (*(volatile tlk_stimer_reg_t *) TLK_STIMER_BASE_ADDR)

#endif

