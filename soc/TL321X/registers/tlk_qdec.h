#ifndef TLK_TLK_QDEC_REGISTERS_H_
#define TLK_TLK_QDEC_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_qdec_dbntime_e {
    TLK_QDEC_DBNTIME_POLA     = TLK_BIT(4),
    TLK_QDEC_DBNTIME_SHUTTLE0 = TLK_BIT(5),
} tlk_qdec_dbntime_e;

typedef enum tlk_qdec_irq_mask_e {
    TLK_QDEC_IRQ_MASK_IRQ_EN = TLK_BIT(0),
} tlk_qdec_irq_mask_e;

typedef enum tlk_qdec_irq_status_e {
    TLK_QDEC_IRQ_STATUS_IRQ_FLAG = TLK_BIT(0),
} tlk_qdec_irq_status_e;

typedef enum tlk_qdec_double_acc_mode_e {
    TLK_QDEC_DOUBLE_ACC_MODE_MODE_EN = TLK_BIT(0),
} tlk_qdec_double_acc_mode_e;

typedef enum tlk_qdec_count_reload_e {
    TLK_QDEC_COUNT_RELOAD_LOAD_DATA = TLK_BIT(0),
} tlk_qdec_count_reload_e;

typedef struct __attribute__((packed)) {
    uint8_t count; // address: 0x80140240, offset: 0x0
    union {
        struct {
            unsigned value : 3;
            unsigned reserved0 : 1;
            unsigned pola : 1;
            unsigned shuttle0 : 1;
            unsigned reserved1 : 2;
        } __attribute__((packed)) dbntime_bit;
        struct {
            enum tlk_qdec_dbntime_e __attribute__((packed)) dbntime : 8;
        }; // address: 0x80140241, offset: 0x1
    };
    union {
        struct {
            unsigned pin_sel : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) channel_a0_bit;
        uint8_t channel_a0; // address: 0x80140242, offset: 0x2
    };
    union {
        struct {
            unsigned pin_sel : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) channel_b0_bit;
        uint8_t channel_b0; // address: 0x80140243, offset: 0x3
    };
    union {
        struct {
            unsigned irq_en : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) irq_mask_bit;
        struct {
            enum tlk_qdec_irq_mask_e __attribute__((packed)) irq_mask : 8;
        }; // address: 0x80140244, offset: 0x4
    };
    union {
        struct {
            unsigned irq_flag : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) irq_status_bit;
        struct {
            enum tlk_qdec_irq_status_e __attribute__((packed)) irq_status : 8;
        }; // address: 0x80140245, offset: 0x5
    };
    uint8_t read; // address: 0x80140246, offset: 0x6
    union {
        struct {
            unsigned mode_en : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) double_acc_mode_bit;
        struct {
            enum tlk_qdec_double_acc_mode_e __attribute__((packed)) double_acc_mode : 8;
        }; // address: 0x80140247, offset: 0x7
    };
    union {
        struct {
            unsigned load_data : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) count_reload_bit;
        struct {
            enum tlk_qdec_count_reload_e __attribute__((packed)) count_reload : 8;
        }; // address: 0x80140248, offset: 0x8
    };
} tlk_qdec_reg_t;

#define TLK_QDEC_BASE_ADDR (0x80140240U)
#define tlk_qdec_reg (*(volatile tlk_qdec_reg_t *) TLK_QDEC_BASE_ADDR)

#endif