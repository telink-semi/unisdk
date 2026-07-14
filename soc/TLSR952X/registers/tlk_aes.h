#ifndef TLK_TLK_AES_REGISTERS_H_
#define TLK_TLK_AES_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_aes_rw_bt_control_e {
    TLK_AES_RW_BT_CONTROL_SOFT_RESET = TLK_BIT(26),
} tlk_aes_rw_bt_control_e;

typedef enum tlk_aes_irq_mask_e {
    TLK_AES_IRQ_MASK_CRYPT_IRQ = TLK_BIT(7),
} tlk_aes_irq_mask_e;

typedef enum tlk_aes_irq_status_e {
    TLK_AES_IRQ_STATUS_CRYPT_IRQ = TLK_BIT(7),
} tlk_aes_irq_status_e;

typedef enum tlk_aes_irq_raw_status_e {
    TLK_AES_IRQ_RAW_STATUS_CRYPT_IRQ = TLK_BIT(7),
} tlk_aes_irq_raw_status_e;

typedef enum tlk_aes_irq_clear_e {
    TLK_AES_IRQ_CLEAR_CRYPT_IRQ = TLK_BIT(7),
} tlk_aes_irq_clear_e;

typedef enum tlk_aes_control_e {
    TLK_AES_CONTROL_START = TLK_BIT(0),
    TLK_AES_CONTROL_MODE  = TLK_BIT(1),
} tlk_aes_control_e;

typedef enum tlk_aes_rpase_count_e {
    TLK_AES_RPASE_COUNT_GEN_RES = TLK_BIT(29),
    TLK_AES_RPASE_COUNT_START   = TLK_BIT(30),
    TLK_AES_RPASE_COUNT_EN      = TLK_BIT(31),
} tlk_aes_rpase_count_e;

typedef enum tlk_aes_hash_status_e {
    TLK_AES_HASH_STATUS_RPASE_STATUS_CLR = TLK_BIT(28),
    TLK_AES_HASH_STATUS_MATCH            = TLK_BIT(31),
} tlk_aes_hash_status_e;

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned reserved0 : 26;
            unsigned soft_reset : 1;
            unsigned reserved1 : 5;
        } __attribute__((packed)) rw_bt_control_bit;
        struct {
            enum tlk_aes_rw_bt_control_e __attribute__((packed)) rw_bt_control : 32;
        }; // address: 0x80160000, offset: 0x0
    };
    uint8_t reserved0[0x8]; // address: 0x80160004, offset: 0x4
    union {
        struct {
            unsigned reserved0 : 7;
            unsigned crypt_irq : 1;
            unsigned reserved1 : 24;
        } __attribute__((packed)) irq_mask_bit;
        struct {
            enum tlk_aes_irq_mask_e __attribute__((packed)) irq_mask : 32;
        }; // address: 0x8016000c, offset: 0xc
    };
    union {
        struct {
            unsigned reserved0 : 7;
            unsigned crypt_irq : 1;
            unsigned reserved1 : 24;
        } __attribute__((packed)) irq_status_bit;
        struct {
            enum tlk_aes_irq_status_e __attribute__((packed)) irq_status : 32;
        }; // address: 0x80160010, offset: 0x10
    };
    union {
        struct {
            unsigned reserved0 : 7;
            unsigned crypt_irq : 1;
            unsigned reserved1 : 24;
        } __attribute__((packed)) irq_raw_status_bit;
        struct {
            enum tlk_aes_irq_raw_status_e __attribute__((packed)) irq_raw_status : 32;
        }; // address: 0x80160014, offset: 0x14
    };
    union {
        struct {
            unsigned reserved0 : 7;
            unsigned crypt_irq : 1;
            unsigned reserved1 : 24;
        } __attribute__((packed)) irq_clear_bit;
        struct {
            enum tlk_aes_irq_clear_e __attribute__((packed)) irq_clear : 32;
        }; // address: 0x80160018, offset: 0x18
    };
    uint8_t reserved1[0x94]; // address: 0x8016001c, offset: 0x1c
    union {
        struct {
            unsigned start : 1;
            unsigned mode : 1;
            unsigned reserved : 30;
        } __attribute__((packed)) control_bit;
        struct {
            enum tlk_aes_control_e __attribute__((packed)) control : 32;
        }; // address: 0x801600b0, offset: 0xb0
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
        struct {
            enum tlk_aes_rpase_count_e __attribute__((packed)) rpase_count : 32;
        }; // address: 0x80160288, offset: 0x288
    };
    union {
        struct {
            unsigned value : 24;
            unsigned irk_cnt : 4;
            unsigned rpase_status_clr : 1;
            unsigned rpase_status : 2;
            unsigned match : 1;
        } __attribute__((packed)) hash_status_bit;
        struct {
            enum tlk_aes_hash_status_e __attribute__((packed)) hash_status : 32;
        }; // address: 0x8016028c, offset: 0x28c
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