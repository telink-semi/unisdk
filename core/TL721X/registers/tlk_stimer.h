#ifndef TLK_TLK_STIMER_REGISTERS_H_
#define TLK_TLK_STIMER_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_stimer_irq_mask_e {
    TLK_STIMER_IRQ_MASK_TIMER_IRQ_EN     = TLK_BIT(0),
    TLK_STIMER_IRQ_MASK_CAL_IRQ_EN       = TLK_BIT(1),
    TLK_STIMER_IRQ_MASK_SYNC_WAIT_IRQ_EN = TLK_BIT(2),
    TLK_STIMER_IRQ_MASK_TRIG_PAST_EN     = TLK_BIT(3),
    TLK_STIMER_IRQ_MASK_CAPT_IRQ_EN      = TLK_BIT(4),
    TLK_STIMER_IRQ_MASK_CAPT_OV_IRQ_EN   = TLK_BIT(5),
} tlk_stimer_irq_mask_e;

typedef enum tlk_stimer_irq_status_e {
    TLK_STIMER_IRQ_STATUS_TIMER_IRQ   = TLK_BIT(0),
    TLK_STIMER_IRQ_STATUS_CAL_IRQ     = TLK_BIT(1),
    TLK_STIMER_IRQ_STATUS_CAPT_IRQ    = TLK_BIT(2),
    TLK_STIMER_IRQ_STATUS_CAPT_OV_IRQ = TLK_BIT(3),
} tlk_stimer_irq_status_e;

typedef enum tlk_stimer_control_e {
    TLK_STIMER_CONTROL_WRITE_32K  = TLK_BIT(0),
    TLK_STIMER_CONTROL_TIMER_EN   = TLK_BIT(1),
    TLK_STIMER_CONTROL_TIMER_AUTO = TLK_BIT(2),
    TLK_STIMER_CONTROL_CAL_32K_EN = TLK_BIT(3),
} tlk_stimer_control_e;

typedef enum tlk_stimer_state_e {
    TLK_STIMER_STATE_CMD_STOP         = TLK_BIT(1),
    TLK_STIMER_STATE_CMD_SYNC         = TLK_BIT(3),
    TLK_STIMER_STATE_CLK_32K          = TLK_BIT(4),
    TLK_STIMER_STATE_CLR_READ_DONE    = TLK_BIT(5),
    TLK_STIMER_STATE_RD_BUSY          = TLK_BIT(6),
    TLK_STIMER_STATE_CMD_SET_DLY_DONE = TLK_BIT(7),
} tlk_stimer_state_e;

typedef enum tlk_stimer_sys_timer_up_e {
    TLK_STIMER_SYS_TIMER_UP_UPDATE_UPON_32K  = TLK_BIT(0),
    TLK_STIMER_SYS_TIMER_UP_RUN_UPON_NXT_32K = TLK_BIT(1),
} tlk_stimer_sys_timer_up_e;

typedef enum tlk_stimer_control1_e {
    TLK_STIMER_CONTROL1_CAPT_EN       = TLK_BIT(2),
    TLK_STIMER_CONTROL1_PEM_EVENT_EN  = TLK_BIT(3),
    TLK_STIMER_CONTROL1_PEM_TASK_0_EN = TLK_BIT(4),
    TLK_STIMER_CONTROL1_PEM_TASK_1_EN = TLK_BIT(5),
    TLK_STIMER_CONTROL1_PEM_TASK_2_EN = TLK_BIT(6),
} tlk_stimer_control1_e;

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
        struct {
            enum tlk_stimer_irq_mask_e __attribute__((packed)) irq_mask : 8;
        }; // address: 0x80140208, offset: 0x8
    };
    union {
        struct {
            unsigned timer_irq : 1;
            unsigned cal_irq : 1;
            unsigned capt_irq : 1;
            unsigned capt_ov_irq : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) irq_status_bit;
        struct {
            enum tlk_stimer_irq_status_e __attribute__((packed)) irq_status : 8;
        }; // address: 0x80140209, offset: 0x9
    };
    union {
        struct {
            unsigned write_32k : 1;
            unsigned timer_en : 1;
            unsigned timer_auto : 1;
            unsigned cal_32k_en : 1;
            unsigned cal_32k_mode : 4;
        } __attribute__((packed)) control_bit;
        struct {
            enum tlk_stimer_control_e __attribute__((packed)) control : 8;
        }; // address: 0x8014020a, offset: 0xa
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
        struct {
            enum tlk_stimer_state_e __attribute__((packed)) state : 8;
        }; // address: 0x8014020b, offset: 0xb
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
        struct {
            enum tlk_stimer_sys_timer_up_e __attribute__((packed)) sys_timer_up : 8;
        }; // address: 0x80140218, offset: 0x18
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
        struct {
            enum tlk_stimer_control1_e __attribute__((packed)) control1 : 8;
        }; // address: 0x80140219, offset: 0x19
    };
    uint8_t reserved0[0x2]; // address: 0x8014021a, offset: 0x1a
    uint32_t capture; // address: 0x8014021c, offset: 0x1c
} tlk_stimer_reg_t;

#define TLK_STIMER_BASE_ADDR (0x80140200U)
#define tlk_stimer_reg (*(volatile tlk_stimer_reg_t *) TLK_STIMER_BASE_ADDR)

#endif