#ifndef TLK_TLK_PLIC_REGISTERS_H_
#define TLK_TLK_PLIC_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef struct __attribute__((packed)) {
    uint32_t feat_en; // address: 0xe4000000, offset: 0x0
    union {
        struct {
            unsigned priority : 2;
            unsigned reserved : 30;
        } __attribute__((packed)) irq_priority_bit[46];
        uint32_t irq_priority[46]; // address: 0xe4000004, offset: 0x4
    };
    uint8_t reserved0[0xf44]; // address: 0xe40000bc, offset: 0xbc
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned source : 31;
        } __attribute__((packed)) irq_pending_l_bit;
        uint32_t irq_pending_l; // address: 0xe4001000, offset: 0x1000
    };
    union {
        struct {
            unsigned source : 15;
            unsigned reserved1 : 17;
        } __attribute__((packed)) irq_pending_h_bit;
        uint32_t irq_pending_h; // address: 0xe4001004, offset: 0x1004
    };
    uint8_t reserved1[0xff8]; // address: 0xe4001008, offset: 0x1008
    union {
        struct {
            unsigned reserved0 : 1;
            unsigned source : 31;
        } __attribute__((packed)) irq_en_l_bit;
        uint32_t irq_en_l; // address: 0xe4002000, offset: 0x2000
    };
    union {
        struct {
            unsigned source : 15;
            unsigned reserved1 : 17;
        } __attribute__((packed)) irq_en_h_bit;
        uint32_t irq_en_h; // address: 0xe4002004, offset: 0x2004
    };
    uint8_t reserved2[0x1fdff8]; // address: 0xe4002008, offset: 0x2008
    uint32_t irq_threshold; // address: 0xe4200000, offset: 0x200000
    union {
        struct {
            unsigned source : 10;
            unsigned reserved : 22;
        } __attribute__((packed)) irq_claim_compl_bit;
        uint32_t irq_claim_compl; // address: 0xe4200004, offset: 0x200004
    };
    uint8_t reserved3[0x3f8]; // address: 0xe4200008, offset: 0x200008
    uint32_t irq_preempted_priority_stack; // address: 0xe4200400, offset: 0x200400
} tlk_plic_reg_t;

#define TLK_PLIC_BASE_ADDR (0xe4000000U)
#define tlk_plic_reg (*(volatile tlk_plic_reg_t *) TLK_PLIC_BASE_ADDR)

#endif