#ifndef TLK_USB_REGISTERS_H_
#define TLK_USB_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned reg_ptr : 4;
            unsigned reserved : 4;
        } __attribute__((packed)) edp0_pointer_bit;
        uint8_t edp0_pointer; // address: 0x80100800, offset: 0x0
    };
    union {
        struct {
            unsigned buff : 8;
        } __attribute__((packed)) edp0_data_bit;
        uint8_t edp0_data; // address: 0x80100801, offset: 0x1
    };
    union {
        struct {
            unsigned ack_data : 1;
            unsigned stall_data : 1;
            unsigned ack_status : 1;
            unsigned stall_status : 1;
            unsigned udc_count : 4;
        } __attribute__((packed)) edp0_control_bit;
        uint8_t edp0_control; // address: 0x80100802, offset: 0x2
    };
    union {
        struct {
            unsigned irq_reset : 1;
            unsigned irq_250us_sof : 1;
            unsigned suspend_i : 1;
            unsigned reserved0 : 1;
            unsigned irq_setup : 1;
            unsigned irq_data : 1;
            unsigned irq_status : 1;
            unsigned irq_setinf : 1;
        } __attribute__((packed)) edp0_status_bit;
        uint8_t edp0_status; // address: 0x80100803, offset: 0x3
    };
    union {
        struct {
            unsigned r_en_sadr : 1;
            unsigned r_en_cfg : 1;
            unsigned r_en_inf : 1;
            unsigned r_en_sta : 1;
            unsigned r_en_frm : 1;
            unsigned r_en_desc : 1;
            unsigned r_en_fea : 1;
            unsigned r_en_hw : 1;
        } __attribute__((packed)) edp0_mode_bit;
        uint8_t edp0_mode; // address: 0x80100804, offset: 0x4
    };
    union {
        struct {
            unsigned r_clk_sel_0 : 1;
            unsigned low_speed : 1;
            unsigned r_clk_sel_2 : 1;
            unsigned test_mode : 1;
            unsigned r_clk_sel_o : 4;
        } __attribute__((packed)) control_bit;
        uint8_t control; // address: 0x80100805, offset: 0x5
    };
    union {
        struct {
            unsigned clk_div : 11;
            unsigned reserved : 5;
        } __attribute__((packed)) cal_cycl_bit;
        uint16_t cal_cycl; // address: 0x80100806, offset: 0x6
    };
    uint8_t reserved0[0x2]; // address: 0x80100808, offset: 0x8
    union {
        struct {
            unsigned r_mdev : 1;
            unsigned reserved0 : 1;
            unsigned wakeup_feature_o : 1;
            unsigned r_vend : 1;
            unsigned r_vend_disable : 1;
            unsigned mode_sel : 2;
            unsigned reserved1 : 1;
        } __attribute__((packed)) mdev_bit;
        uint8_t mdev; // address: 0x8010080a, offset: 0xa
    };
    union {
        struct {
            unsigned sie_adr_i : 7;
            unsigned r_config : 1;
        } __attribute__((packed)) endpoint0_sie_bit;
        uint8_t endpoint0_sie; // address: 0x8010080b, offset: 0xb
    };
    union {
        struct {
            unsigned r_suspend_count : 5;
            unsigned r_endpoint0_stall : 1;
            unsigned hold0 : 2;
        } __attribute__((packed)) suspend_cyc_bit;
        uint8_t suspend_cyc; // address: 0x8010080c, offset: 0xc
    };
    uint8_t inf_alt; // address: 0x8010080d, offset: 0xd
    uint8_t edps_en; // address: 0x8010080e, offset: 0xe
    union {
        struct {
            unsigned r_mask : 3;
            unsigned r_lvl : 2;
            unsigned r_250us_sof_sel : 1;
            unsigned hold1 : 2;
        } __attribute__((packed)) irq_mask_bit;
        uint8_t irq_mask; // address: 0x8010080f, offset: 0xf
    };
    uint8_t edps_ptr_low[8]; // address: 0x80100810, offset: 0x10
    union {
        struct {
            unsigned val : 2;
            unsigned reserved : 6;
        } __attribute__((packed)) edps_ptr_high_bit[8];
        uint8_t edps_ptr_high[8]; // address: 0x80100818, offset: 0x18
    };
    uint8_t edps_data[8]; // address: 0x80100820, offset: 0x20
    union {
        struct {
            unsigned rd_ack : 1;
            unsigned rd_stall : 1;
            unsigned set_data0 : 1;
            unsigned set_data1 : 1;
            unsigned reserved : 2;
            unsigned endpoint_specific0 : 1;
            unsigned endpoint_specific1 : 1;
        } __attribute__((packed)) edps_ct_bit[8];
        uint8_t edps_ct[8]; // address: 0x80100828, offset: 0x28
    };
    uint8_t edps_adr_low[8]; // address: 0x80100830, offset: 0x30
    union {
        struct {
            unsigned value : 2;
            unsigned reserved : 6;
        } __attribute__((packed)) edps_adr_high_bit[8];
        uint8_t edps_adr_high[8]; // address: 0x80100838, offset: 0x38
    };
    uint8_t usb_iso; // address: 0x80100840, offset: 0x40
    uint8_t usb_irq; // address: 0x80100841, offset: 0x41
    uint8_t usb_mask; // address: 0x80100842, offset: 0x42
    uint8_t usb_max0; // address: 0x80100843, offset: 0x43
    uint8_t usb_min0; // address: 0x80100844, offset: 0x44
    union {
        struct {
            unsigned r_fifo0 : 1;
            unsigned full0 : 1;
            unsigned r_mode00 : 1;
            unsigned endpoint8_eof : 1;
            unsigned endpoint8_dma_eof : 3;
            unsigned r_mode05 : 1;
        } __attribute__((packed)) usb_fifo_bit;
        uint8_t usb_fifo; // address: 0x80100845, offset: 0x45
    };
    union {
        struct {
            unsigned max_in : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) usb_max_bit;
        uint8_t usb_max; // address: 0x80100846, offset: 0x46
    };
    uint8_t usb_tick; // address: 0x80100847, offset: 0x47
    union {
        struct {
            unsigned sr_cen : 1;
            unsigned sr_clk : 1;
            unsigned r_ram2 : 1;
            unsigned wen_i : 1;
            unsigned r_ram4 : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) usb_ram_bit;
        uint8_t usb_ram; // address: 0x80100848, offset: 0x48
    };
    union {
        struct {
            unsigned blk1 : 2;
            unsigned hold2 : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) usb_min1_bit;
        uint8_t usb_min1; // address: 0x80100849, offset: 0x49
    };
} tlk_usb_reg_t;

#define TLK_USB_BASE_ADDR (0x80100800U)
#define tlk_usb_reg (*(volatile tlk_usb_reg_t *) TLK_USB_BASE_ADDR)

#endif

