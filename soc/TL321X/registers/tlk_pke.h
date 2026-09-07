#ifndef TLK_TLK_PKE_REGISTERS_H_
#define TLK_TLK_PKE_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_pke_control_e {
    TLK_PKE_CONTROL_START = TLK_BIT(0),
} tlk_pke_control_e;

typedef enum tlk_pke_risr_e {
    TLK_PKE_RISR_CORE_ISR = TLK_BIT(0),
} tlk_pke_risr_e;

typedef enum tlk_pke_imcr_e {
    TLK_PKE_IMCR_CORE_IRQ_EN = TLK_BIT(0),
} tlk_pke_imcr_e;

typedef enum tlk_pke_misr_e {
    TLK_PKE_MISR_CORE_MI = TLK_BIT(0),
} tlk_pke_misr_e;

typedef enum tlk_pke_exe_config_e {
    TLK_PKE_EXE_CONFIG_IAFF_R0 = TLK_BIT(0),
    TLK_PKE_EXE_CONFIG_IMON_R0 = TLK_BIT(1),
    TLK_PKE_EXE_CONFIG_IAFF_R1 = TLK_BIT(2),
    TLK_PKE_EXE_CONFIG_IMON_R1 = TLK_BIT(3),
    TLK_PKE_EXE_CONFIG_OAFF    = TLK_BIT(4),
    TLK_PKE_EXE_CONFIG_OMON    = TLK_BIT(5),
} tlk_pke_exe_config_e;

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned start : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) control_bit;
        struct {
            enum tlk_pke_control_e __attribute__((packed)) control : 8;
        }; // address: 0x80110000, offset: 0x0
    };
    uint8_t reserved0[0x3]; // address: 0x80110001, offset: 0x1
    union {
        struct {
            unsigned partial_radix : 13;
            unsigned reserved0 : 3;
            unsigned base_radix : 3;
            unsigned reserved1 : 13;
        } __attribute__((packed)) config_bit;
        uint32_t config; // address: 0x80110004, offset: 0x4
    };
    union {
        struct {
            unsigned address : 12;
            unsigned reserved : 4;
        } __attribute__((packed)) mc_pointer_bit;
        uint16_t mc_pointer; // address: 0x80110008, offset: 0x8
    };
    uint8_t reserved1[0x2]; // address: 0x8011000a, offset: 0xa
    union {
        struct {
            unsigned core_isr : 1;
            unsigned reserved : 31;
        } __attribute__((packed)) risr_bit;
        struct {
            enum tlk_pke_risr_e __attribute__((packed)) risr : 32;
        }; // address: 0x8011000c, offset: 0xc
    };
    union {
        struct {
            unsigned core_irq_en : 1;
            unsigned reserved : 31;
        } __attribute__((packed)) imcr_bit;
        struct {
            enum tlk_pke_imcr_e __attribute__((packed)) imcr : 32;
        }; // address: 0x80110010, offset: 0x10
    };
    union {
        struct {
            unsigned core_mi : 1;
            unsigned reserved : 31;
        } __attribute__((packed)) misr_bit;
        struct {
            enum tlk_pke_misr_e __attribute__((packed)) misr : 32;
        }; // address: 0x80110014, offset: 0x14
    };
    uint8_t reserved2[0xc]; // address: 0x80110018, offset: 0x18
    union {
        struct {
            unsigned stop_log : 4;
            unsigned reserved : 28;
        } __attribute__((packed)) rt_code_bit;
        uint32_t rt_code; // address: 0x80110024, offset: 0x24
    };
    uint8_t reserved3[0x28]; // address: 0x80110028, offset: 0x28
    union {
        struct {
            unsigned iaff_r0 : 1;
            unsigned imon_r0 : 1;
            unsigned iaff_r1 : 1;
            unsigned imon_r1 : 1;
            unsigned oaff : 1;
            unsigned omon : 1;
            unsigned reserved : 26;
        } __attribute__((packed)) exe_config_bit;
        struct {
            enum tlk_pke_exe_config_e __attribute__((packed)) exe_config : 32;
        }; // address: 0x80110050, offset: 0x50
    };
    uint8_t reserved4[0xa8]; // address: 0x80110054, offset: 0x54
    union {
        struct {
            unsigned mir : 4;
            unsigned mar : 4;
            unsigned reserved0 : 8;
            unsigned project : 16;
        } __attribute__((packed)) version_bit;
        uint32_t version; // address: 0x801100fc, offset: 0xfc
    };
} tlk_pke_reg_t;

#define TLK_PKE_BASE_ADDR (0x80110000U)
#define tlk_pke_reg (*(volatile tlk_pke_reg_t *) TLK_PKE_BASE_ADDR)

#endif