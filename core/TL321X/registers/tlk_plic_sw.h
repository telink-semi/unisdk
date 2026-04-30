#ifndef TLK_PLIC_SW_REGISTERS_H_
#define TLK_PLIC_SW_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x1000]; // address: 0xc6400000, offset: 0x0
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned source : 1;
            unsigned reserved1 : 30;
        } __attribute__((packed)) irq_pending_bit;
        uint32_t irq_pending; // address: 0xc6401000, offset: 0x1000
    };
    uint8_t reserved1[0xffc]; // address: 0xc6401004, offset: 0x1004
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned source : 1;
            unsigned reserved1 : 30;
        } __attribute__((packed)) irq_en_bit;
        uint32_t irq_en; // address: 0xc6402000, offset: 0x2000
    };
    uint8_t reserved2[0x1fe000]; // address: 0xc6402004, offset: 0x2004
    union {
        struct {
            unsigned source : 10;
            unsigned reserved : 22;
        } __attribute__((packed)) irq_claim_compl_bit;
        uint32_t irq_claim_compl; // address: 0xc6600004, offset: 0x200004
    };
} tlk_plic_sw_reg_t;

#define TLK_PLIC_SW_BASE_ADDR (0xc6400000U)
#define tlk_plic_sw_reg (*(volatile tlk_plic_sw_reg_t *) TLK_PLIC_SW_BASE_ADDR)

#endif

