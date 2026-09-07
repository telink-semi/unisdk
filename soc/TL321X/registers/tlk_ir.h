#ifndef TLK_TLK_IR_REGISTERS_H_
#define TLK_TLK_IR_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_ir_control0_e {
    TLK_IR_CONTROL0_EN            = TLK_BIT(0),
    TLK_IR_CONTROL0_RX_FIFO_CLEAR = TLK_BIT(1),
} tlk_ir_control0_e;

typedef enum tlk_ir_control1_e {
    TLK_IR_CONTROL1_IR_MODE         = TLK_BIT(1),
    TLK_IR_CONTROL1_IR_INVERT       = TLK_BIT(2),
    TLK_IR_CONTROL1_HIGH_WRITE_EN   = TLK_BIT(3),
    TLK_IR_CONTROL1_CYCLE_WRITE_EN  = TLK_BIT(4),
    TLK_IR_CONTROL1_DATA_MODE       = TLK_BIT(5),
    TLK_IR_CONTROL1_TIMEOUT_DISABLE = TLK_BIT(6),
    TLK_IR_CONTROL1_IR_SELECT       = TLK_BIT(7),
} tlk_ir_control1_e;

typedef enum tlk_ir_irq_mask_e {
    TLK_IR_IRQ_MASK_HIGH_IRQ_EN      = TLK_BIT(0),
    TLK_IR_IRQ_MASK_CYCLE_IRQ_EN     = TLK_BIT(1),
    TLK_IR_IRQ_MASK_TIMEOUT_IRQ_EN   = TLK_BIT(2),
    TLK_IR_IRQ_MASK_RX_BUFFER_IRQ_EN = TLK_BIT(3),
} tlk_ir_irq_mask_e;

typedef enum tlk_ir_irq_status_e {
    TLK_IR_IRQ_STATUS_HIGH_IRQ      = TLK_BIT(0),
    TLK_IR_IRQ_STATUS_CYCLE_IRQ     = TLK_BIT(1),
    TLK_IR_IRQ_STATUS_TIMEOUT_IRQ   = TLK_BIT(2),
    TLK_IR_IRQ_STATUS_RX_BUFFER_IRQ = TLK_BIT(3),
} tlk_ir_irq_status_e;

typedef enum tlk_ir_fifo_control_e {
    TLK_IR_FIFO_CONTROL_AUTO_RX_CLEAR_EN = TLK_BIT(3),
    TLK_IR_FIFO_CONTROL_PEM_EVENT_EN     = TLK_BIT(4),
    TLK_IR_FIFO_CONTROL_PEM_TASK0_EN     = TLK_BIT(5),
    TLK_IR_FIFO_CONTROL_PEM_TASK1_EN     = TLK_BIT(6),
} tlk_ir_fifo_control_e;

typedef enum tlk_ir_ir_status_e {
    TLK_IR_IR_STATUS_RX_FULL  = TLK_BIT(4),
    TLK_IR_IR_STATUS_RX_EMPTY = TLK_BIT(5),
    TLK_IR_IR_STATUS_RX_DONE  = TLK_BIT(6),
} tlk_ir_ir_status_e;

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned en : 1;
            unsigned rx_fifo_clear : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) control0_bit;
        struct {
            enum tlk_ir_control0_e __attribute__((packed)) control0 : 8;
        }; // address: 0x801404c0, offset: 0x0
    };
    union {
        struct {
            unsigned reserved : 1;
            unsigned ir_mode : 1;
            unsigned ir_invert : 1;
            unsigned high_write_en : 1;
            unsigned cycle_write_en : 1;
            unsigned data_mode : 1;
            unsigned timeout_disable : 1;
            unsigned ir_select : 1;
        } __attribute__((packed)) control1_bit;
        struct {
            enum tlk_ir_control1_e __attribute__((packed)) control1 : 8;
        }; // address: 0x801404c1, offset: 0x1
    };
    uint8_t timeout[3]; // address: 0x801404c2, offset: 0x2
    union {
        struct {
            unsigned high_irq_en : 1;
            unsigned cycle_irq_en : 1;
            unsigned timeout_irq_en : 1;
            unsigned rx_buffer_irq_en : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) irq_mask_bit;
        struct {
            enum tlk_ir_irq_mask_e __attribute__((packed)) irq_mask : 8;
        }; // address: 0x801404c5, offset: 0x5
    };
    union {
        struct {
            unsigned high_irq : 1;
            unsigned cycle_irq : 1;
            unsigned timeout_irq : 1;
            unsigned rx_buffer_irq : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) irq_status_bit;
        struct {
            enum tlk_ir_irq_status_e __attribute__((packed)) irq_status : 8;
        }; // address: 0x801404c6, offset: 0x6
    };
    union {
        struct {
            unsigned fifo_level : 3;
            unsigned auto_rx_clear_en : 1;
            unsigned pem_event_en : 1;
            unsigned pem_task0_en : 1;
            unsigned pem_task1_en : 1;
            unsigned reserved : 1;
        } __attribute__((packed)) fifo_control_bit;
        struct {
            enum tlk_ir_fifo_control_e __attribute__((packed)) fifo_control : 8;
        }; // address: 0x801404c7, offset: 0x7
    };
    union {
        struct {
            unsigned rx_buffer_count : 4;
            unsigned rx_full : 1;
            unsigned rx_empty : 1;
            unsigned rx_done : 1;
            unsigned reserved : 1;
        } __attribute__((packed)) ir_status_bit;
        struct {
            enum tlk_ir_ir_status_e __attribute__((packed)) ir_status : 8;
        }; // address: 0x801404c8, offset: 0x8
    };
    uint8_t reserved0[0x3]; // address: 0x801404c9, offset: 0x9
    uint32_t read_data; // address: 0x801404cc, offset: 0xc
} tlk_ir_reg_t;

#define TLK_IR_BASE_ADDR (0x801404c0U)
#define tlk_ir_reg (*(volatile tlk_ir_reg_t *) TLK_IR_BASE_ADDR)

#endif