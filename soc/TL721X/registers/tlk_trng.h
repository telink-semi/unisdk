#ifndef TLK_TRNG_REGISTERS_H_
#define TLK_TRNG_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned rng_en : 1;
            unsigned reserved : 3;
            unsigned ros3_en : 1;
            unsigned ros2_en : 1;
            unsigned ros1_en : 1;
            unsigned ros0_en : 1;
        } __attribute__((packed)) control0_bit;
        uint8_t control0; // address: 0x80103000, offset: 0x0
    };
    uint8_t reserved0; // address: 0x80103001, offset: 0x1
    union {
        struct {
            unsigned data_irq_en : 1;
            unsigned read_irq_en : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0x80103002, offset: 0x2
    };
    union {
        struct {
            unsigned irq_en : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) control3_bit;
        uint8_t control3; // address: 0x80103003, offset: 0x3
    };
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) mode_sel_bit;
        uint8_t mode_sel; // address: 0x80103004, offset: 0x4
    };
    uint8_t reserved1[0x3]; // address: 0x80103005, offset: 0x5
    union {
        struct {
            unsigned htf : 1;
            unsigned drdy : 1;
            unsigned read_err : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) status0_bit;
        uint8_t status0; // address: 0x80103008, offset: 0x8
    };
    uint8_t reserved2[0x3]; // address: 0x80103009, offset: 0x9
    uint32_t data; // address: 0x8010300c, offset: 0xc
    union {
        struct {
            unsigned mir : 4;
            unsigned mar : 4;
            unsigned reserved : 8;
            unsigned project0 : 8;
            unsigned project1 : 8;
        } __attribute__((packed)) version_bit;
        uint32_t version; // address: 0x80103010, offset: 0x10
    };
    uint8_t reserved3[0x2c]; // address: 0x80103014, offset: 0x14
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) reseed_bit;
        uint8_t reseed; // address: 0x80103040, offset: 0x40
    };
    uint8_t reserved4[0x1f]; // address: 0x80103041, offset: 0x41
    union {
        struct {
            unsigned drpten : 1;
            unsigned drcten : 1;
            unsigned reserved0 : 2;
            unsigned traten : 1;
            unsigned trcten : 1;
            unsigned reserved1 : 2;
        } __attribute__((packed)) ht_control_bit;
        uint8_t ht_control; // address: 0x80103060, offset: 0x60
    };
    uint8_t reserved5[0xf]; // address: 0x80103061, offset: 0x61
    union {
        struct {
            unsigned drptbf : 1;
            unsigned drctbf : 1;
            unsigned reserved0 : 6;
            unsigned taptnbbf : 1;
            unsigned taptbbf : 1;
            unsigned trctbf : 1;
            unsigned reserved1 : 5;
            unsigned drptf : 1;
            unsigned drctf : 1;
            unsigned reserved2 : 6;
            unsigned taptf : 1;
            unsigned trctf : 1;
            unsigned reserved3 : 6;
        } __attribute__((packed)) ht_status_bit;
        uint32_t ht_status; // address: 0x80103070, offset: 0x70
    };
    uint8_t reserved6[0xc]; // address: 0x80103074, offset: 0x74
    uint32_t ro_src_en1; // address: 0x80103080, offset: 0x80
    uint32_t ro_src_en2; // address: 0x80103084, offset: 0x84
    union {
        struct {
            unsigned value : 2;
            unsigned reserved : 6;
        } __attribute__((packed)) sclk_freq_bit;
        uint8_t sclk_freq; // address: 0x80103088, offset: 0x88
    };
    uint8_t reserved7[0x27]; // address: 0x80103089, offset: 0x89
    union {
        struct {
            unsigned en : 1;
            unsigned ms : 1;
            unsigned osel : 1;
            unsigned reserved0 : 5;
            unsigned tero_en : 4;
            unsigned reserved1 : 4;
            unsigned reserved2 : 8;
            unsigned cotv : 8;
        } __attribute__((packed)) tero_control_bit;
        uint32_t tero_control; // address: 0x801030b0, offset: 0xb0
    };
    uint16_t tero_threshold_ub; // address: 0x801030b4, offset: 0xb4
    uint16_t tero_threshold_lb; // address: 0x801030b6, offset: 0xb6
    uint8_t reserved8[0x8]; // address: 0x801030b8, offset: 0xb8
    uint16_t tero_cnt_neg; // address: 0x801030c0, offset: 0xc0
    uint16_t tero_cnt_pos; // address: 0x801030c2, offset: 0xc2
    uint8_t reserved9[0xc]; // address: 0x801030c4, offset: 0xc4
    union {
        struct {
            unsigned dr : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) tero_status_bit;
        uint8_t tero_status; // address: 0x801030d0, offset: 0xd0
    };
    uint8_t reserved10[0x3]; // address: 0x801030d1, offset: 0xd1
    uint32_t tero_data; // address: 0x801030d4, offset: 0xd4
    uint8_t reserved11[0x8]; // address: 0x801030d8, offset: 0xd8
    uint32_t tero_rcr; // address: 0x801030e0, offset: 0xe0
} tlk_trng_reg_t;

#define TLK_TRNG_BASE_ADDR (0x80103000U)
#define tlk_trng_reg (*(volatile tlk_trng_reg_t *) TLK_TRNG_BASE_ADDR)

#endif


#ifndef TLK_TRNG_EXTENDED_REGISTERS_H_
#define TLK_TRNG_EXTENDED_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t fifo; // address: 0x80100800, offset: 0x0
    union {
        struct {
            unsigned trng_rxdma_en : 1;
            unsigned trng_skip_startup : 1;
            unsigned trng_sclk_sel : 1;
            unsigned trng_irq_en : 1;
            unsigned trng_irq_alarm_en : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) control_bit;
        uint8_t control; // address: 0x80100804, offset: 0x4
    };
    union {
        struct {
            unsigned trng_irq_status : 1;
            unsigned trng_alarm_status : 1;
            unsigned trng_ready_status : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) status_bit;
        uint8_t status; // address: 0x80100805, offset: 0x5
    };
} tlk_trng_extended_reg_t;

#define TLK_TRNG_EXTENDED_BASE_ADDR (0x80100800U)
#define tlk_trng_extended_reg (*(volatile tlk_trng_extended_reg_t *) TLK_TRNG_EXTENDED_BASE_ADDR)

#endif

