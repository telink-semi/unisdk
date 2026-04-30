#ifndef TLK_AES_REGISTERS_H_
#define TLK_AES_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned reserved0 : 26;
            unsigned soft_reset : 1;
            unsigned reserved1 : 5;
        } __attribute__((packed)) rw_bt_control_bit;
        uint32_t rw_bt_control; // address: 0x80160000, offset: 0x0
    };
    uint8_t reserved0[0x8]; // address: 0x80160004, offset: 0x4
    union {
        struct {
            unsigned reserved0 : 7;
            unsigned crypt_irq : 1;
            unsigned reserved1 : 24;
        } __attribute__((packed)) irq_mask_bit;
        uint32_t irq_mask; // address: 0x8016000c, offset: 0xc
    };
    union {
        struct {
            unsigned reserved0 : 7;
            unsigned crypt_irq : 1;
            unsigned reserved1 : 24;
        } __attribute__((packed)) irq_status_bit;
        uint32_t irq_status; // address: 0x80160010, offset: 0x10
    };
    union {
        struct {
            unsigned reserved0 : 7;
            unsigned crypt_irq : 1;
            unsigned reserved1 : 24;
        } __attribute__((packed)) irq_raw_status_bit;
        uint32_t irq_raw_status; // address: 0x80160014, offset: 0x14
    };
    union {
        struct {
            unsigned reserved0 : 7;
            unsigned crypt_irq : 1;
            unsigned reserved1 : 24;
        } __attribute__((packed)) irq_clear_bit;
        uint32_t irq_clear; // address: 0x80160018, offset: 0x18
    };
    uint8_t reserved1[0x94]; // address: 0x8016001c, offset: 0x1c
    union {
        struct {
            unsigned start : 1;
            unsigned mode : 1;
            unsigned reserved : 30;
        } __attribute__((packed)) control_bit;
        uint32_t control; // address: 0x801600b0, offset: 0xb0
    };
    uint32_t key[4]; // address: 0x801600b4, offset: 0xb4
    union {
        struct {
            unsigned value : 16;
            unsigned e0 : 16;
        } __attribute__((packed)) pointer_bit;
        uint32_t pointer; // address: 0x801600c4, offset: 0xc4
    };
    uint8_t reserved2[0x1c0]; // address: 0x801600c8, offset: 0xc8
    union {
        struct {
            unsigned prand : 24;
            unsigned irk_num : 4;
            unsigned reserved : 1;
            unsigned gen_res : 1;
            unsigned start : 1;
            unsigned en : 1;
        } __attribute__((packed)) rpase_count_bit;
        uint32_t rpase_count; // address: 0x80160288, offset: 0x288
    };
    union {
        struct {
            unsigned value : 24;
            unsigned irk_cnt : 4;
            unsigned rpase_status_clr : 1;
            unsigned rpase_status : 2;
            unsigned match : 1;
        } __attribute__((packed)) hash_status_bit;
        uint32_t hash_status; // address: 0x8016028c, offset: 0x28c
    };
    union {
        struct {
            unsigned value : 16;
            unsigned reserved : 16;
        } __attribute__((packed)) irk_pointer_bit;
        uint32_t irk_pointer; // address: 0x80160290, offset: 0x290
    };
} tlk_aes_reg_t;

#define TLK_AES_BASE_ADDR (0x80160000U)
#define tlk_aes_reg (*(volatile tlk_aes_reg_t *) TLK_AES_BASE_ADDR)

#endif

