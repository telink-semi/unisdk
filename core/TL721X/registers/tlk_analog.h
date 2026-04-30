#ifndef TLK_ANALOG_REGISTERS_H_
#define TLK_ANALOG_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t address; // address: 0x80140180, offset: 0x0
    uint8_t reserved0; // address: 0x80140181, offset: 0x1
    union {
        struct {
            unsigned tx_en : 1;
            unsigned rx_en : 1;
            unsigned mask_tx_done : 1;
            unsigned mask_rx_done : 1;
            unsigned contiu_acc : 1;
            unsigned rw : 1;
            unsigned cyc : 1;
            unsigned busy : 1;
        } __attribute__((packed)) control_bit;
        uint8_t control; // address: 0x80140182, offset: 0x2
    };
    uint8_t length; // address: 0x80140183, offset: 0x3
    uint8_t data[4]; // address: 0x80140184, offset: 0x4
    union {
        struct {
            unsigned rx_count : 4;
            unsigned tx_count : 4;
        } __attribute__((packed)) buff_count_bit;
        uint8_t buff_count; // address: 0x80140188, offset: 0x8
    };
    union {
        struct {
            unsigned reserved : 7;
            unsigned rx_done : 1;
        } __attribute__((packed)) status_bit;
        uint8_t status; // address: 0x80140189, offset: 0x9
    };
    union {
        struct {
            unsigned txbuff_irq : 1;
            unsigned rxbuff_irq : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) irq_status_bit;
        uint8_t irq_status; // address: 0x8014018a, offset: 0xa
    };
    union {
        struct {
            unsigned cyc1 : 1;
            unsigned dma_en : 1;
            unsigned auto_rxclr_en : 1;
            unsigned ndma_rxdone_en : 1;
            unsigned div_mode : 2;
            unsigned reserved : 2;
        } __attribute__((packed)) dma_control_bit;
        uint8_t dma_control; // address: 0x8014018b, offset: 0xb
    };
} tlk_analog_reg_t;

#define TLK_ANALOG_BASE_ADDR (0x80140180U)
#define tlk_analog_reg (*(volatile tlk_analog_reg_t *) TLK_ANALOG_BASE_ADDR)

#endif

