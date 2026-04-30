#ifndef TLK_IR_REGISTERS_H_
#define TLK_IR_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned en : 1;
            unsigned rx_fifo_clear : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) control0_bit;
        uint8_t control0; // address: 0x801404c0, offset: 0x0
    };
    union {
        struct {
            unsigned ir_mode : 2;
            unsigned ir_invert : 1;
            unsigned high_write_en : 1;
            unsigned cycle_write_en : 1;
            unsigned data_mode : 1;
            unsigned timeout_disable : 1;
            unsigned ir_select : 1;
        } __attribute__((packed)) control1_bit;
        uint8_t control1; // address: 0x801404c1, offset: 0x1
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
        uint8_t irq_mask; // address: 0x801404c5, offset: 0x5
    };
    union {
        struct {
            unsigned high_irq : 1;
            unsigned cycle_irq : 1;
            unsigned timeout_irq : 1;
            unsigned rx_buffer_irq : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) irq_status_bit;
        uint8_t irq_status; // address: 0x801404c6, offset: 0x6
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
        uint8_t fifo_control; // address: 0x801404c7, offset: 0x7
    };
    union {
        struct {
            unsigned rx_buffer_count : 4;
            unsigned rx_full : 1;
            unsigned rx_empty : 1;
            unsigned rx_done : 1;
            unsigned reserved : 1;
        } __attribute__((packed)) ir_status_bit;
        uint8_t ir_status; // address: 0x801404c8, offset: 0x8
    };
    uint8_t reserved0[0x3]; // address: 0x801404c9, offset: 0x9
    uint32_t read_data; // address: 0x801404cc, offset: 0xc
} tlk_ir_reg_t;

#define TLK_IR_BASE_ADDR (0x801404c0U)
#define tlk_ir_reg (*(volatile tlk_ir_reg_t *) TLK_IR_BASE_ADDR)

#endif

