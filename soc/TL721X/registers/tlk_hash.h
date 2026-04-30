#ifndef TLK_HASH_LP_REGISTERS_H_
#define TLK_HASH_LP_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned startr : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) control_bit;
        uint8_t control; // address: 0x80102000, offset: 0x0
    };
    uint8_t reserved0[0x3]; // address: 0x80102001, offset: 0x1
    union {
        struct {
            unsigned msel : 4;
            unsigned reserved : 4;
        } __attribute__((packed)) config0_bit;
        uint8_t config0; // address: 0x80102004, offset: 0x4
    };
    uint8_t reserved1; // address: 0x80102005, offset: 0x5
    union {
        struct {
            unsigned irq_en : 1;
            unsigned dma_en : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) config1_bit;
        uint8_t config1; // address: 0x80102006, offset: 0x6
    };
    union {
        struct {
            unsigned last : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) config2_bit;
        uint8_t config2; // address: 0x80102007, offset: 0x7
    };
    union {
        struct {
            unsigned busy : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) status1_bit;
        uint8_t status1; // address: 0x80102008, offset: 0x8
    };
    uint8_t reserved2[0x3]; // address: 0x80102009, offset: 0x9
    union {
        struct {
            unsigned done : 1;
            unsigned err_config : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) status2_bit;
        uint8_t status2; // address: 0x8010200c, offset: 0xc
    };
    uint8_t reserved3[0x13]; // address: 0x8010200d, offset: 0xd
    uint8_t pcr_len[16]; // address: 0x80102020, offset: 0x20
    uint8_t value_out[64]; // address: 0x80102030, offset: 0x30
    uint8_t value_in[64]; // address: 0x80102070, offset: 0x70
    union {
        struct {
            unsigned mir : 4;
            unsigned mar : 4;
        } __attribute__((packed)) version_bit;
        uint8_t version; // address: 0x801020b0, offset: 0xb0
    };
    uint8_t reserved4; // address: 0x801020b1, offset: 0xb1
    uint16_t version_project; // address: 0x801020b2, offset: 0xb2
    uint8_t reserved5[0x4c]; // address: 0x801020b4, offset: 0xb4
    uint8_t message_in[128]; // address: 0x80102100, offset: 0x100
    uint8_t reserved6[0x8]; // address: 0x80102180, offset: 0x180
    uint32_t dma_rlen; // address: 0x80102188, offset: 0x188
    uint32_t dma_wlen; // address: 0x8010218c, offset: 0x18c
} tlk_hash_lp_reg_t;

#define TLK_HASH_LP_BASE_ADDR (0x80102000U)
#define tlk_hash_lp_reg (*(volatile tlk_hash_lp_reg_t *) TLK_HASH_LP_BASE_ADDR)

#endif


#ifndef TLK_HASH_LP_EXTENDED_REGISTERS_H_
#define TLK_HASH_LP_EXTENDED_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x30]; // address: 0x80100800, offset: 0x0
    uint8_t fifo[4]; // address: 0x80100830, offset: 0x30
    union {
        struct {
            unsigned txdma_en : 1;
            unsigned rxdma_en : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) dma_en_bit;
        uint8_t dma_en; // address: 0x80100834, offset: 0x34
    };
    union {
        struct {
            unsigned txf_thres : 4;
            unsigned rxf_thres : 4;
        } __attribute__((packed)) fifo_threshold_bit;
        uint8_t fifo_threshold; // address: 0x80100835, offset: 0x35
    };
} tlk_hash_lp_extended_reg_t;

#define TLK_HASH_LP_EXTENDED_BASE_ADDR (0x80100800U)
#define tlk_hash_lp_extended_reg (*(volatile tlk_hash_lp_extended_reg_t *) TLK_HASH_LP_EXTENDED_BASE_ADDR)

#endif

