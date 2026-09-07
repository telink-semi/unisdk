#ifndef TLK_TLK_DMA_REGISTERS_H_
#define TLK_TLK_DMA_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_dma_soft_reset_e {
    TLK_DMA_SOFT_RESET_SOFT_RESET = TLK_BIT(0),
    TLK_DMA_SOFT_RESET_REQ_CLR    = TLK_BIT(1),
} tlk_dma_soft_reset_e;

typedef enum tlk_dma_chn_control_e {
    TLK_DMA_CHN_CONTROL_CHN_EN       = TLK_BIT(0),
    TLK_DMA_CHN_CONTROL_TC_IRQ_EN    = TLK_BIT(1),
    TLK_DMA_CHN_CONTROL_ERR_IRQ_EN   = TLK_BIT(2),
    TLK_DMA_CHN_CONTROL_ABT_IRQ_EN   = TLK_BIT(3),
    TLK_DMA_CHN_CONTROL_DST_MODE     = TLK_BIT(18),
    TLK_DMA_CHN_CONTROL_SRC_MODE     = TLK_BIT(19),
    TLK_DMA_CHN_CONTROL_READ_NUM_EN  = TLK_BIT(28),
    TLK_DMA_CHN_CONTROL_PRIORITY     = TLK_BIT(29),
    TLK_DMA_CHN_CONTROL_WRITE_NUM_EN = TLK_BIT(30),
    TLK_DMA_CHN_CONTROL_AUTO_EN      = TLK_BIT(31),
} tlk_dma_chn_control_e;

typedef struct __attribute__((packed)) {
    uint8_t reserved0[0x20]; // address: 0x80100400, offset: 0x0
    union {
        struct {
            unsigned soft_reset : 1;
            unsigned req_clr : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) soft_reset_bit;
        struct {
            enum tlk_dma_soft_reset_e __attribute__((packed)) soft_reset : 8;
        }; // address: 0x80100420, offset: 0x20
    };
    uint8_t reserved1[0xf]; // address: 0x80100421, offset: 0x21
    union {
        struct {
            unsigned error_irq : 8;
            unsigned abort_irq : 8;
            unsigned tc_irq : 8;
            unsigned reserved : 8;
        } __attribute__((packed)) irq_status_bit;
        uint32_t irq_status; // address: 0x80100430, offset: 0x30
    };
    uint8_t reserved2[0xc]; // address: 0x80100434, offset: 0x34
    uint8_t abort; // address: 0x80100440, offset: 0x40
    uint8_t reserved3[0x3]; // address: 0x80100441, offset: 0x41
    struct {
        union {
            struct {
                unsigned chn_en : 1;
                unsigned tc_irq_en : 1;
                unsigned err_irq_en : 1;
                unsigned abt_irq_en : 1;
                unsigned dst_req_sel : 5;
                unsigned src_req_sel : 5;
                unsigned dst_addr_ctrl : 2;
                unsigned src_addr_ctrl : 2;
                unsigned dst_mode : 1;
                unsigned src_mode : 1;
                unsigned dst_width : 2;
                unsigned src_width : 2;
                unsigned src_burst_size : 3;
                unsigned reserved : 1;
                unsigned read_num_en : 1;
                unsigned priority : 1;
                unsigned write_num_en : 1;
                unsigned auto_en : 1;
            } __attribute__((packed)) control_bit;
            struct {
                enum tlk_dma_chn_control_e __attribute__((packed)) control : 32;
            }; // address: 0x80100444, offset: 0x44
        };
        uint32_t src_addr; // address: 0x80100448, offset: 0x48
        uint32_t dst_addr; // address: 0x8010044c, offset: 0x4c
        union {
            struct {
                unsigned tran_size : 22;
                unsigned tran_size_idx : 2;
                unsigned reserved : 8;
            } __attribute__((packed)) size_bit;
            uint32_t size; // address: 0x80100450, offset: 0x50
        };
        union {
            struct {
                unsigned reserved : 2;
                unsigned pointer : 30;
            } __attribute__((packed)) ll_pointer_bit;
            uint32_t ll_pointer; // address: 0x80100454, offset: 0x54
        };
    } __attribute__((packed)) chn[8]; // address: 0x80100444, offset: 0x44
    uint8_t reserved4[0x2f]; // address: 0x801004e4, offset: 0xe4
    uint8_t ll_int_mode[2]; // address: 0x80100513, offset: 0x113
} tlk_dma_reg_t;

#define TLK_DMA_BASE_ADDR (0x80100400U)
#define tlk_dma_reg (*(volatile tlk_dma_reg_t *) TLK_DMA_BASE_ADDR)

#endif