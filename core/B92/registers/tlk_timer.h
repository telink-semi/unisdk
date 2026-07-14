#ifndef TLK_TLK_TIMER_REGISTERS_H_
#define TLK_TLK_TIMER_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_timer_control0_e {
    TLK_TIMER_CONTROL0_TMR0_EN     = TLK_BIT(2),
    TLK_TIMER_CONTROL0_TMR0_NOWRAP = TLK_BIT(3),
    TLK_TIMER_CONTROL0_TMR1_EN     = TLK_BIT(6),
    TLK_TIMER_CONTROL0_TMR1_NOWRAP = TLK_BIT(7),
} tlk_timer_control0_e;

typedef enum tlk_timer_control2_e {
    TLK_TIMER_CONTROL2_WDT_EN = TLK_BIT(7),
} tlk_timer_control2_e;

typedef enum tlk_timer_control3_e {
    TLK_TIMER_CONTROL3_TMR0_STATUS    = TLK_BIT(0),
    TLK_TIMER_CONTROL3_TMR1_STATUS    = TLK_BIT(1),
    TLK_TIMER_CONTROL3_WDT_STATUS_W1C = TLK_BIT(2),
    TLK_TIMER_CONTROL3_WDT_CNT_CLEAR  = TLK_BIT(3),
    TLK_TIMER_CONTROL3_SOFTWARE_IRQ   = TLK_BIT(7),
} tlk_timer_control3_e;

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
        struct {
            enum tlk_timer_control0_e __attribute__((packed)) control0 : 8;
        }; // address: 0x80140140, offset: 0x0
    };
    uint8_t reserved0; // address: 0x80140141, offset: 0x1
    union {
        struct {
            unsigned reserved : 7;
            unsigned wdt_en : 1;
        } __attribute__((packed)) control2_bit;
        struct {
            enum tlk_timer_control2_e __attribute__((packed)) control2 : 8;
        }; // address: 0x80140142, offset: 0x2
    };
    union {
        struct {
            unsigned tmr0_status : 1;
            unsigned tmr1_status : 1;
            unsigned wdt_status_w1c : 1;
            unsigned wdt_cnt_clear : 1;
            unsigned reserved : 3;
            unsigned software_irq : 1;
        } __attribute__((packed)) control3_bit;
        struct {
            enum tlk_timer_control3_e __attribute__((packed)) control3 : 8;
        }; // address: 0x80140143, offset: 0x3
    };
    uint32_t tmr0_capture; // address: 0x80140144, offset: 0x4
    uint32_t tmr1_capture; // address: 0x80140148, offset: 0x8
    uint32_t wdt_interval; // address: 0x8014014c, offset: 0xc
    uint32_t tmr0_tick; // address: 0x80140150, offset: 0x10
    uint32_t tmr1_tick; // address: 0x80140154, offset: 0x14
} tlk_timer_reg_t;

#define TLK_TIMER_BASE_ADDR (0x80140140U)
#define tlk_timer_reg (*(volatile tlk_timer_reg_t *) TLK_TIMER_BASE_ADDR)

#endif