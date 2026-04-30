#ifndef TLK_CHACHA20_REGISTERS_H_
#define TLK_CHACHA20_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned start : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) control_bit;
        uint8_t control; // address: 0x80105000, offset: 0x0
    };
    uint8_t reserved0[0x3]; // address: 0x80105001, offset: 0x1
    union {
        struct {
            unsigned sec_stage : 2;
            unsigned decode : 1;
            unsigned dma_en : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) config_bit;
        uint8_t config; // address: 0x80105004, offset: 0x4
    };
    uint8_t reserved1[0x3]; // address: 0x80105005, offset: 0x5
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) soft_reset_bit;
        uint8_t soft_reset; // address: 0x80105008, offset: 0x8
    };
    uint8_t reserved2[0x3]; // address: 0x80105009, offset: 0x9
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) data_in_ready_bit;
        uint8_t data_in_ready; // address: 0x8010500c, offset: 0xc
    };
    uint8_t reserved3[0x3]; // address: 0x8010500d, offset: 0xd
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) risr_bit;
        uint8_t risr; // address: 0x80105010, offset: 0x10
    };
    uint8_t reserved4[0x3]; // address: 0x80105011, offset: 0x11
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) imcr_bit;
        uint8_t imcr; // address: 0x80105014, offset: 0x14
    };
    uint8_t reserved5[0x3]; // address: 0x80105015, offset: 0x15
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) status_bit;
        uint8_t status; // address: 0x80105018, offset: 0x18
    };
    uint8_t reserved6[0x3]; // address: 0x80105019, offset: 0x19
    union {
        struct {
            unsigned value : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) err_code_bit;
        uint8_t err_code; // address: 0x8010501c, offset: 0x1c
    };
    uint8_t reserved7[0x3]; // address: 0x8010501d, offset: 0x1d
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) last_bit;
        uint8_t last; // address: 0x80105020, offset: 0x20
    };
    uint8_t reserved8[0x3]; // address: 0x80105021, offset: 0x21
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) se_done_bit;
        uint8_t se_done; // address: 0x80105024, offset: 0x24
    };
    uint8_t reserved9[0xb]; // address: 0x80105025, offset: 0x25
    uint8_t key[32]; // address: 0x80105030, offset: 0x30
    uint8_t iv[8]; // address: 0x80105050, offset: 0x50
    uint8_t const[4]; // address: 0x80105058, offset: 0x58
    uint8_t reserved10[0x4]; // address: 0x8010505c, offset: 0x5c
    uint8_t length[20]; // address: 0x80105060, offset: 0x60
    uint8_t count[4]; // address: 0x80105074, offset: 0x74
    uint8_t reserved11[0x8]; // address: 0x80105078, offset: 0x78
    uint8_t tag_in[20]; // address: 0x80105080, offset: 0x80
    uint8_t data_in[4]; // address: 0x80105094, offset: 0x94
    uint8_t reserved12[0x64]; // address: 0x80105098, offset: 0x98
    union {
        struct {
            unsigned mir : 4;
            unsigned mar : 4;
        } __attribute__((packed)) version_bit;
        uint8_t version; // address: 0x801050fc, offset: 0xfc
    };
    uint8_t reserved13; // address: 0x801050fd, offset: 0xfd
    uint16_t version_project; // address: 0x801050fe, offset: 0xfe
    uint8_t data_out[16]; // address: 0x80105100, offset: 0x100
    uint8_t tag_out[20]; // address: 0x80105110, offset: 0x110
    uint8_t count_out[4]; // address: 0x80105124, offset: 0x124
    uint8_t reserved14[0x98]; // address: 0x80105128, offset: 0x128
    uint32_t dma_saddr; // address: 0x801051c0, offset: 0x1c0
    uint32_t dma_rlen; // address: 0x801051c4, offset: 0x1c4
    uint8_t reserved15[0x10]; // address: 0x801051c8, offset: 0x1c8
    uint32_t dma_len; // address: 0x801051d8, offset: 0x1d8
} tlk_chacha20_reg_t;

#define TLK_CHACHA20_BASE_ADDR (0x80105000U)
#define tlk_chacha20_reg (*(volatile tlk_chacha20_reg_t *) TLK_CHACHA20_BASE_ADDR)

#endif


#ifndef TLK_CHACHA20_EXTENDED_REGISTERS_H_
#define TLK_CHACHA20_EXTENDED_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x10]; // address: 0x80100800, offset: 0x0
    uint8_t fifo[4]; // address: 0x80100810, offset: 0x10
    union {
        struct {
            unsigned txdma_en : 1;
            unsigned rxdma_en : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) dma_en_bit;
        uint8_t dma_en; // address: 0x80100814, offset: 0x14
    };
    union {
        struct {
            unsigned txf_thres : 4;
            unsigned rxf_thres : 4;
        } __attribute__((packed)) fifo_threshold_bit;
        uint8_t fifo_threshold; // address: 0x80100815, offset: 0x15
    };
} tlk_chacha20_extended_reg_t;

#define TLK_CHACHA20_EXTENDED_BASE_ADDR (0x80100800U)
#define tlk_chacha20_extended_reg (*(volatile tlk_chacha20_extended_reg_t *) TLK_CHACHA20_EXTENDED_BASE_ADDR)

#endif

