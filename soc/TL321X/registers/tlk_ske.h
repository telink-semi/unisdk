#ifndef TLK_SKE_REGISTERS_H_
#define TLK_SKE_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned start : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) control_bit;
        uint8_t control; // address: 0x80104000, offset: 0x0
    };
    uint8_t reserved0[0x3]; // address: 0x80104001, offset: 0x1
    union {
        struct {
            unsigned alg : 4;
            unsigned reserved0 : 7;
            unsigned dec : 1;
            unsigned up_cfg : 1;
            unsigned reserved1 : 3;
            unsigned dma_en : 1;
            unsigned irq_en : 1;
            unsigned reserved2 : 6;
            unsigned data_type : 2;
            unsigned reserved3 : 2;
            unsigned mode : 4;
        } __attribute__((packed)) config_bit;
        uint32_t config; // address: 0x80104004, offset: 0x4
    };
    union {
        struct {
            unsigned busy : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) status1_bit;
        uint8_t status1; // address: 0x80104008, offset: 0x8
    };
    uint8_t reserved1[0x3]; // address: 0x80104009, offset: 0x9
    union {
        struct {
            unsigned core_done : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) status2_bit;
        uint8_t status2; // address: 0x8010400c, offset: 0xc
    };
    uint8_t reserved2[0x3]; // address: 0x8010400d, offset: 0xd
    uint8_t key1[32]; // address: 0x80104010, offset: 0x10
    uint8_t reserved3[0x30]; // address: 0x80104030, offset: 0x30
    uint8_t aad[8]; // address: 0x80104060, offset: 0x60
    uint8_t clen[8]; // address: 0x80104068, offset: 0x68
    uint8_t iv[16]; // address: 0x80104070, offset: 0x70
    union {
        struct {
            unsigned reserved0 : 16;
            unsigned last : 1;
            unsigned reserved1 : 15;
        } __attribute__((packed)) data_in_control_bit;
        uint32_t data_in_control; // address: 0x80104080, offset: 0x80
    };
    uint8_t reserved4[0xc]; // address: 0x80104084, offset: 0x84
    uint8_t data_in[16]; // address: 0x80104090, offset: 0x90
    uint8_t reserved5[0x10]; // address: 0x801040a0, offset: 0xa0
    uint8_t data_out[16]; // address: 0x801040b0, offset: 0xb0
    uint32_t source_addr; // address: 0x801040c0, offset: 0xc0
    uint32_t dest_addr; // address: 0x801040c4, offset: 0xc4
    uint32_t rlen; // address: 0x801040c8, offset: 0xc8
    uint32_t wlen; // address: 0x801040cc, offset: 0xcc
    uint8_t reserved6[0x2c]; // address: 0x801040d0, offset: 0xd0
    union {
        struct {
            unsigned mir : 4;
            unsigned mar : 4;
            unsigned reserved : 8;
            unsigned project_l : 8;
            unsigned project_h : 8;
        } __attribute__((packed)) version_bit;
        uint32_t version; // address: 0x801040fc, offset: 0xfc
    };
} tlk_ske_reg_t;

#define TLK_SKE_BASE_ADDR (0x80104000U)
#define tlk_ske_reg (*(volatile tlk_ske_reg_t *) TLK_SKE_BASE_ADDR)

#endif


#ifndef TLK_SKE_EXTENDED_REGISTERS_H_
#define TLK_SKE_EXTENDED_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x20]; // address: 0x80100800, offset: 0x0
    uint32_t fifo; // address: 0x80100820, offset: 0x20
    union {
        struct {
            unsigned ske_txdma_en : 1;
            unsigned ske_rxdma_en : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) dma_en_bit;
        uint8_t dma_en; // address: 0x80100824, offset: 0x24
    };
    union {
        struct {
            unsigned ske_txf_thres : 4;
            unsigned ske_rxf_thres : 4;
        } __attribute__((packed)) fifo_threshold_bit;
        uint8_t fifo_threshold; // address: 0x80100825, offset: 0x25
    };
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) ske_sp_vld_bit;
        uint8_t ske_sp_vld; // address: 0x80100826, offset: 0x26
    };
} tlk_ske_extended_reg_t;

#define TLK_SKE_EXTENDED_BASE_ADDR (0x80100800U)
#define tlk_ske_extended_reg (*(volatile tlk_ske_extended_reg_t *) TLK_SKE_EXTENDED_BASE_ADDR)

#endif

