#ifndef TLK_TLK_TRNG_REGISTERS_H_
#define TLK_TLK_TRNG_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_trng_control0_e {
    TLK_TRNG_CONTROL0_RNG_EN  = TLK_BIT(0),
    TLK_TRNG_CONTROL0_ROS3_EN = TLK_BIT(4),
    TLK_TRNG_CONTROL0_ROS2_EN = TLK_BIT(5),
    TLK_TRNG_CONTROL0_ROS1_EN = TLK_BIT(6),
    TLK_TRNG_CONTROL0_ROS0_EN = TLK_BIT(7),
} tlk_trng_control0_e;

typedef enum tlk_trng_control2_e {
    TLK_TRNG_CONTROL2_DATA_IRQ_EN = TLK_BIT(0),
    TLK_TRNG_CONTROL2_READ_IRQ_EN = TLK_BIT(1),
} tlk_trng_control2_e;

typedef enum tlk_trng_control3_e {
    TLK_TRNG_CONTROL3_IRQ_EN = TLK_BIT(0),
} tlk_trng_control3_e;

typedef enum tlk_trng_mode_sel_e {
    TLK_TRNG_MODE_SEL_VALUE = TLK_BIT(0),
} tlk_trng_mode_sel_e;

typedef enum tlk_trng_status0_e {
    TLK_TRNG_STATUS0_HTF      = TLK_BIT(0),
    TLK_TRNG_STATUS0_DRDY     = TLK_BIT(1),
    TLK_TRNG_STATUS0_READ_ERR = TLK_BIT(2),
} tlk_trng_status0_e;

typedef enum tlk_trng_reseed_e {
    TLK_TRNG_RESEED_VALUE = TLK_BIT(0),
} tlk_trng_reseed_e;

typedef enum tlk_trng_ht_control_e {
    TLK_TRNG_HT_CONTROL_DRPTEN = TLK_BIT(0),
    TLK_TRNG_HT_CONTROL_DRCTEN = TLK_BIT(1),
    TLK_TRNG_HT_CONTROL_TRATEN = TLK_BIT(4),
    TLK_TRNG_HT_CONTROL_TRCTEN = TLK_BIT(5),
} tlk_trng_ht_control_e;

typedef enum tlk_trng_ht_status_e {
    TLK_TRNG_HT_STATUS_DRPTBF   = TLK_BIT(0),
    TLK_TRNG_HT_STATUS_DRCTBF   = TLK_BIT(1),
    TLK_TRNG_HT_STATUS_TAPTNBBF = TLK_BIT(8),
    TLK_TRNG_HT_STATUS_TAPTBBF  = TLK_BIT(9),
    TLK_TRNG_HT_STATUS_TRCTBF   = TLK_BIT(10),
    TLK_TRNG_HT_STATUS_DRPTF    = TLK_BIT(16),
    TLK_TRNG_HT_STATUS_DRCTF    = TLK_BIT(17),
    TLK_TRNG_HT_STATUS_TAPTF    = TLK_BIT(24),
    TLK_TRNG_HT_STATUS_TRCTF    = TLK_BIT(25),
} tlk_trng_ht_status_e;

typedef enum tlk_trng_tero_control_e {
    TLK_TRNG_TERO_CONTROL_EN   = TLK_BIT(0),
    TLK_TRNG_TERO_CONTROL_MS   = TLK_BIT(1),
    TLK_TRNG_TERO_CONTROL_OSEL = TLK_BIT(2),
} tlk_trng_tero_control_e;

typedef enum tlk_trng_tero_status_e {
    TLK_TRNG_TERO_STATUS_DR = TLK_BIT(0),
} tlk_trng_tero_status_e;

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
        struct {
            enum tlk_trng_control0_e __attribute__((packed)) control0 : 8;
        }; // address: 0x80103000, offset: 0x0
    };
    uint8_t reserved0; // address: 0x80103001, offset: 0x1
    union {
        struct {
            unsigned data_irq_en : 1;
            unsigned read_irq_en : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) control2_bit;
        struct {
            enum tlk_trng_control2_e __attribute__((packed)) control2 : 8;
        }; // address: 0x80103002, offset: 0x2
    };
    union {
        struct {
            unsigned irq_en : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) control3_bit;
        struct {
            enum tlk_trng_control3_e __attribute__((packed)) control3 : 8;
        }; // address: 0x80103003, offset: 0x3
    };
    union {
        struct {
            unsigned value : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) mode_sel_bit;
        struct {
            enum tlk_trng_mode_sel_e __attribute__((packed)) mode_sel : 8;
        }; // address: 0x80103004, offset: 0x4
    };
    uint8_t reserved1[0x3]; // address: 0x80103005, offset: 0x5
    union {
        struct {
            unsigned htf : 1;
            unsigned drdy : 1;
            unsigned read_err : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) status0_bit;
        struct {
            enum tlk_trng_status0_e __attribute__((packed)) status0 : 8;
        }; // address: 0x80103008, offset: 0x8
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
        struct {
            enum tlk_trng_reseed_e __attribute__((packed)) reseed : 8;
        }; // address: 0x80103040, offset: 0x40
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
        struct {
            enum tlk_trng_ht_control_e __attribute__((packed)) ht_control : 8;
        }; // address: 0x80103060, offset: 0x60
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
        struct {
            enum tlk_trng_ht_status_e __attribute__((packed)) ht_status : 32;
        }; // address: 0x80103070, offset: 0x70
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
        struct {
            enum tlk_trng_tero_control_e __attribute__((packed)) tero_control : 32;
        }; // address: 0x801030b0, offset: 0xb0
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
        struct {
            enum tlk_trng_tero_status_e __attribute__((packed)) tero_status : 8;
        }; // address: 0x801030d0, offset: 0xd0
    };
    uint8_t reserved10[0x3]; // address: 0x801030d1, offset: 0xd1
    uint32_t tero_data; // address: 0x801030d4, offset: 0xd4
    uint8_t reserved11[0x8]; // address: 0x801030d8, offset: 0xd8
    uint32_t tero_rcr; // address: 0x801030e0, offset: 0xe0
} tlk_trng_reg_t;

#define TLK_TRNG_BASE_ADDR (0x80103000U)
#define tlk_trng_reg (*(volatile tlk_trng_reg_t *) TLK_TRNG_BASE_ADDR)



typedef enum tlk_trng_extended_control_e {
    TLK_TRNG_EXTENDED_CONTROL_TRNG_RXDMA_EN     = TLK_BIT(0),
    TLK_TRNG_EXTENDED_CONTROL_TRNG_SKIP_STARTUP = TLK_BIT(1),
    TLK_TRNG_EXTENDED_CONTROL_TRNG_SCLK_SEL     = TLK_BIT(2),
    TLK_TRNG_EXTENDED_CONTROL_TRNG_IRQ_EN       = TLK_BIT(3),
    TLK_TRNG_EXTENDED_CONTROL_TRNG_IRQ_ALARM_EN = TLK_BIT(4),
} tlk_trng_extended_control_e;

typedef enum tlk_trng_extended_status_e {
    TLK_TRNG_EXTENDED_STATUS_TRNG_IRQ_STATUS   = TLK_BIT(0),
    TLK_TRNG_EXTENDED_STATUS_TRNG_ALARM_STATUS = TLK_BIT(1),
    TLK_TRNG_EXTENDED_STATUS_TRNG_READY_STATUS = TLK_BIT(2),
} tlk_trng_extended_status_e;

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
        struct {
            enum tlk_trng_extended_control_e __attribute__((packed)) control : 8;
        }; // address: 0x80100804, offset: 0x4
    };
    union {
        struct {
            unsigned trng_irq_status : 1;
            unsigned trng_alarm_status : 1;
            unsigned trng_ready_status : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) status_bit;
        struct {
            enum tlk_trng_extended_status_e __attribute__((packed)) status : 8;
        }; // address: 0x80100805, offset: 0x5
    };
} tlk_trng_extended_reg_t;

#define TLK_TRNG_EXTENDED_BASE_ADDR (0x80100800U)
#define tlk_trng_extended_reg (*(volatile tlk_trng_extended_reg_t *) TLK_TRNG_EXTENDED_BASE_ADDR)

#endif