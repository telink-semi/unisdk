#ifndef TLK_TIMER_REGISTERS_H_
#define TLK_TIMER_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned tmr0_m_sel : 2;
            unsigned tmr0_en : 1;
            unsigned tmr0_nowrap : 1;
            unsigned tmr1_m_sel : 2;
            unsigned tmr1_en : 1;
            unsigned tmr1_nowrap : 1;
        } __attribute__((packed)) control0_bit;
        uint8_t control0; // address: 0x80140140, offset: 0x0
    };
    union {
        struct {
            unsigned tmr0_capt_mode : 2;
            unsigned reserved0 : 2;
            unsigned tmr1_capt_mode : 2;
            unsigned reserved1 : 2;
        } __attribute__((packed)) control1_bit;
        uint8_t control1; // address: 0x80140141, offset: 0x1
    };
    union {
        struct {
            unsigned reserved0 : 4;
            unsigned tmr0_capt_en : 1;
            unsigned reserved1 : 1;
            unsigned tmr1_capt_en : 1;
            unsigned reserved2 : 1;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0x80140142, offset: 0x2
    };
    union {
        struct {
            unsigned tmr0_mode_irq_mask : 1;
            unsigned tmr0_capt_irq_mask : 1;
            unsigned reserved0 : 1;
            unsigned tmr1_mode_irq_mask : 1;
            unsigned tmr1_capt_irq_mask : 1;
            unsigned reserved1 : 1;
            unsigned tmr0_ov_irq_mask : 1;
            unsigned tmr1_ov_irq_mask : 1;
        } __attribute__((packed)) control3_bit;
        uint8_t control3; // address: 0x80140143, offset: 0x3
    };
    uint32_t compare0; // address: 0x80140144, offset: 0x4
    uint32_t compare1; // address: 0x80140148, offset: 0x8
    uint32_t watchdog_target; // address: 0x8014014c, offset: 0xc
    uint32_t tick0; // address: 0x80140150, offset: 0x10
    uint32_t tick1; // address: 0x80140154, offset: 0x14
    uint32_t capture0; // address: 0x80140158, offset: 0x18
    uint32_t capture1; // address: 0x8014015c, offset: 0x1c
    union {
        struct {
            unsigned watchdog_status : 1;
            unsigned watchdog_cnt_clear : 1;
            unsigned reserved0 : 5;
            unsigned software_irq : 1;
        } __attribute__((packed)) status0_bit;
        uint8_t status0; // address: 0x80140160, offset: 0x20
    };
    union {
        struct {
            unsigned tmr0_mode_irq : 1;
            unsigned tmr0_capt_irq : 1;
            unsigned reserved0 : 1;
            unsigned tmr1_mode_irq : 1;
            unsigned tmr1_capt_irq : 1;
            unsigned reserved1 : 1;
            unsigned tmr0_capt_ov_irq : 1;
            unsigned tmr1_capt_ov_irq : 1;
        } __attribute__((packed)) status1_bit;
        uint8_t status1; // address: 0x80140161, offset: 0x21
    };
    union {
        struct {
            unsigned watchdog_en : 1;
            unsigned pem_event_en : 1;
            unsigned reserved0 : 6;
        } __attribute__((packed)) watchdog_en_bit;
        uint8_t watchdog_en; // address: 0x80140162, offset: 0x22
    };
    uint8_t pem_task_en; // address: 0x80140163, offset: 0x23
} tlk_timer_reg_t;

#define TLK_TIMER_BASE_ADDR (0x80140140U)
#define tlk_timer_reg (*(volatile tlk_timer_reg_t *) TLK_TIMER_BASE_ADDR)

#endif

