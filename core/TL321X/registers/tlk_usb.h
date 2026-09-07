#ifndef TLK_TLK_USB_REGISTERS_H_
#define TLK_TLK_USB_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_usb_edp0_control_e {
    TLK_USB_EDP0_CONTROL_ACK_DATA     = TLK_BIT(0),
    TLK_USB_EDP0_CONTROL_STALL_DATA   = TLK_BIT(1),
    TLK_USB_EDP0_CONTROL_ACK_STATUS   = TLK_BIT(2),
    TLK_USB_EDP0_CONTROL_STALL_STATUS = TLK_BIT(3),
} tlk_usb_edp0_control_e;

typedef enum tlk_usb_edp0_status_e {
    TLK_USB_EDP0_STATUS_IRQ_RESET  = TLK_BIT(0),
    TLK_USB_EDP0_STATUS_IRQ_250US  = TLK_BIT(1),
    TLK_USB_EDP0_STATUS_SUSPEND_I  = TLK_BIT(2),
    TLK_USB_EDP0_STATUS_IRQ_SOF    = TLK_BIT(3),
    TLK_USB_EDP0_STATUS_IRQ_SETUP  = TLK_BIT(4),
    TLK_USB_EDP0_STATUS_IRQ_DATA   = TLK_BIT(5),
    TLK_USB_EDP0_STATUS_IRQ_STATUS = TLK_BIT(6),
    TLK_USB_EDP0_STATUS_IRQ_SETINF = TLK_BIT(7),
} tlk_usb_edp0_status_e;

typedef enum tlk_usb_edp0_mode_e {
    TLK_USB_EDP0_MODE_R_EN_SADR = TLK_BIT(0),
    TLK_USB_EDP0_MODE_R_EN_CFG  = TLK_BIT(1),
    TLK_USB_EDP0_MODE_R_EN_INF  = TLK_BIT(2),
    TLK_USB_EDP0_MODE_R_EN_STA  = TLK_BIT(3),
    TLK_USB_EDP0_MODE_R_EN_FRM  = TLK_BIT(4),
    TLK_USB_EDP0_MODE_R_EN_DESC = TLK_BIT(5),
    TLK_USB_EDP0_MODE_R_EN_FEA  = TLK_BIT(6),
    TLK_USB_EDP0_MODE_R_EN_HW   = TLK_BIT(7),
} tlk_usb_edp0_mode_e;

typedef enum tlk_usb_edp0_mode1_e {
    TLK_USB_EDP0_MODE1_R_EN_HALT_CLR   = TLK_BIT(0),
    TLK_USB_EDP0_MODE1_R_EN_HALT_STALL = TLK_BIT(1),
    TLK_USB_EDP0_MODE1_R_EN_HALT_TGL   = TLK_BIT(2),
    TLK_USB_EDP0_MODE1_R_EN_WKUP_FEA   = TLK_BIT(3),
} tlk_usb_edp0_mode1_e;

typedef enum tlk_usb_edp0_size_e {
    TLK_USB_EDP0_SIZE_R_LVL0 = TLK_BIT(4),
    TLK_USB_EDP0_SIZE_R_LVL1 = TLK_BIT(5),
    TLK_USB_EDP0_SIZE_R_LVL3 = TLK_BIT(7),
} tlk_usb_edp0_size_e;

typedef enum tlk_usb_mdev_e {
    TLK_USB_MDEV_R_MDEV             = TLK_BIT(0),
    TLK_USB_MDEV_SET_WAKEUP_FEATURE = TLK_BIT(1),
    TLK_USB_MDEV_WAKEUP_FEATURE_O   = TLK_BIT(2),
    TLK_USB_MDEV_R_VEND             = TLK_BIT(3),
    TLK_USB_MDEV_R_VEND_DISABLE     = TLK_BIT(4),
} tlk_usb_mdev_e;

typedef enum tlk_usb_edp0_sie_e {
    TLK_USB_EDP0_SIE_ADDR_ERR_FLAG = TLK_BIT(7),
} tlk_usb_edp0_sie_e;

typedef enum tlk_usb_suspend_cyc_e {
    TLK_USB_SUSPEND_CYC_R_EDP0_STALL = TLK_BIT(5),
} tlk_usb_suspend_cyc_e;

typedef enum tlk_usb_irq_mask_e {
    TLK_USB_IRQ_MASK_R_MASK0 = TLK_BIT(0),
    TLK_USB_IRQ_MASK_R_MASK1 = TLK_BIT(1),
    TLK_USB_IRQ_MASK_R_MASK2 = TLK_BIT(2),
    TLK_USB_IRQ_MASK_R_MASK3 = TLK_BIT(3),
    TLK_USB_IRQ_MASK_R_MASK4 = TLK_BIT(4),
    TLK_USB_IRQ_MASK_R_MASK5 = TLK_BIT(5),
    TLK_USB_IRQ_MASK_R_MASK6 = TLK_BIT(6),
    TLK_USB_IRQ_MASK_R_MASK7 = TLK_BIT(7),
} tlk_usb_irq_mask_e;

typedef enum tlk_usb_edps_ct_e {
    TLK_USB_EDPS_CT_RD_ACK    = TLK_BIT(0),
    TLK_USB_EDPS_CT_RD_STALL  = TLK_BIT(1),
    TLK_USB_EDPS_CT_SET_DATA0 = TLK_BIT(2),
    TLK_USB_EDPS_CT_SET_DATA1 = TLK_BIT(3),
} tlk_usb_edps_ct_e;

typedef enum tlk_usb_usb_fifo_e {
    TLK_USB_USB_FIFO_R_FIFO0  = TLK_BIT(0),
    TLK_USB_USB_FIFO_FULL0    = TLK_BIT(1),
    TLK_USB_USB_FIFO_R_MODE00 = TLK_BIT(2),
    TLK_USB_USB_FIFO_EDP8_EOF = TLK_BIT(3),
} tlk_usb_usb_fifo_e;

typedef enum tlk_usb_usb_ram_e {
    TLK_USB_USB_RAM_SR_CEN = TLK_BIT(0),
    TLK_USB_USB_RAM_SR_CLK = TLK_BIT(1),
    TLK_USB_USB_RAM_R_RAM2 = TLK_BIT(2),
    TLK_USB_USB_RAM_WEN_I  = TLK_BIT(3),
    TLK_USB_USB_RAM_R_RAM4 = TLK_BIT(4),
} tlk_usb_usb_ram_e;

typedef enum tlk_usb_usb_min1_e {
    TLK_USB_USB_MIN1_R_EDPS_MAP_MANUAL = TLK_BIT(3),
    TLK_USB_USB_MIN1_R_EDPS_MAP_AUTO   = TLK_BIT(4),
    TLK_USB_USB_MIN1_R_EDPS_SM_MAP_EN  = TLK_BIT(5),
    TLK_USB_USB_MIN1_R_EDPS_MAP_TGL_EN = TLK_BIT(6),
    TLK_USB_USB_MIN1_R_GET_STA_MAP_EN  = TLK_BIT(7),
} tlk_usb_usb_min1_e;

typedef enum tlk_usb_edps_maxh0_e {
    TLK_USB_EDPS_MAXH0_R_MAXH0        = TLK_BIT(0),
    TLK_USB_EDPS_MAXH0_R_EN_AIN_MAXH0 = TLK_BIT(1),
    TLK_USB_EDPS_MAXH0_FULL           = TLK_BIT(5),
    TLK_USB_EDPS_MAXH0_FULL_NZ        = TLK_BIT(6),
    TLK_USB_EDPS_MAXH0_FULL_THRD      = TLK_BIT(7),
} tlk_usb_edps_maxh0_e;

typedef enum tlk_usb_pem_control_e {
    TLK_USB_PEM_CONTROL_PEM_EVENT_EN  = TLK_BIT(0),
    TLK_USB_PEM_CONTROL_PEM_EVENT_SEL = TLK_BIT(1),
} tlk_usb_pem_control_e;

typedef enum tlk_usb_usb_ct_e {
    TLK_USB_USB_CT_R_CLK_SEL_0 = TLK_BIT(0),
    TLK_USB_USB_CT_LOW_SPEED   = TLK_BIT(1),
    TLK_USB_USB_CT_R_CLK_SEL_2 = TLK_BIT(2),
    TLK_USB_USB_CT_TEST_MODE   = TLK_BIT(3),
} tlk_usb_usb_ct_e;

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned reg_ptr : 7;
            unsigned reserved : 1;
        } __attribute__((packed)) edp0_pointer_bit;
        uint8_t edp0_pointer; // address: 0x80118000, offset: 0x0
    };
    uint8_t edp0_data; // address: 0x80118001, offset: 0x1
    union {
        struct {
            unsigned ack_data : 1;
            unsigned stall_data : 1;
            unsigned ack_status : 1;
            unsigned stall_status : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) edp0_control_bit;
        struct {
            enum tlk_usb_edp0_control_e __attribute__((packed)) edp0_control : 8;
        }; // address: 0x80118002, offset: 0x2
    };
    union {
        struct {
            unsigned irq_reset : 1;
            unsigned irq_250us : 1;
            unsigned suspend_i : 1;
            unsigned irq_sof : 1;
            unsigned irq_setup : 1;
            unsigned irq_data : 1;
            unsigned irq_status : 1;
            unsigned irq_setinf : 1;
        } __attribute__((packed)) edp0_status_bit;
        struct {
            enum tlk_usb_edp0_status_e __attribute__((packed)) edp0_status : 8;
        }; // address: 0x80118003, offset: 0x3
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
        struct {
            enum tlk_usb_edp0_mode_e __attribute__((packed)) edp0_mode : 8;
        }; // address: 0x80118004, offset: 0x4
    };
    union {
        struct {
            unsigned r_en_halt_clr : 1;
            unsigned r_en_halt_stall : 1;
            unsigned r_en_halt_tgl : 1;
            unsigned r_en_wkup_fea : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) edp0_mode1_bit;
        struct {
            enum tlk_usb_edp0_mode1_e __attribute__((packed)) edp0_mode1 : 8;
        }; // address: 0x80118005, offset: 0x5
    };
    uint16_t usb_pid_ro; // address: 0x80118006, offset: 0x6
    union {
        struct {
            unsigned udc_cnt : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) edp0_udc_bit;
        uint8_t edp0_udc; // address: 0x80118008, offset: 0x8
    };
    union {
        struct {
            unsigned r_cwptr_mux : 2;
            unsigned r_clens_mux : 2;
            unsigned r_lvl0 : 1;
            unsigned r_lvl1 : 1;
            unsigned reserved : 1;
            unsigned r_lvl3 : 1;
        } __attribute__((packed)) edp0_size_bit;
        struct {
            enum tlk_usb_edp0_size_e __attribute__((packed)) edp0_size : 8;
        }; // address: 0x80118009, offset: 0x9
    };
    union {
        struct {
            unsigned r_mdev : 1;
            unsigned set_wakeup_feature : 1;
            unsigned wakeup_feature_o : 1;
            unsigned r_vend : 1;
            unsigned r_vend_disable : 1;
            unsigned mode_sel : 2;
            unsigned reserved : 1;
        } __attribute__((packed)) mdev_bit;
        struct {
            enum tlk_usb_mdev_e __attribute__((packed)) mdev : 8;
        }; // address: 0x8011800a, offset: 0xa
    };
    union {
        struct {
            unsigned sie_adr_i : 7;
            unsigned addr_err_flag : 1;
        } __attribute__((packed)) edp0_sie_bit;
        struct {
            enum tlk_usb_edp0_sie_e __attribute__((packed)) edp0_sie : 8;
        }; // address: 0x8011800b, offset: 0xb
    };
    union {
        struct {
            unsigned r_suspend_cnt : 5;
            unsigned r_edp0_stall : 1;
            unsigned reserved : 2;
        } __attribute__((packed)) suspend_cyc_bit;
        struct {
            enum tlk_usb_suspend_cyc_e __attribute__((packed)) suspend_cyc : 8;
        }; // address: 0x8011800c, offset: 0xc
    };
    uint8_t inf_alt; // address: 0x8011800d, offset: 0xd
    uint8_t edps_en; // address: 0x8011800e, offset: 0xe
    union {
        struct {
            unsigned r_mask0 : 1;
            unsigned r_mask1 : 1;
            unsigned r_mask2 : 1;
            unsigned r_mask3 : 1;
            unsigned r_mask4 : 1;
            unsigned r_mask5 : 1;
            unsigned r_mask6 : 1;
            unsigned r_mask7 : 1;
        } __attribute__((packed)) irq_mask_bit;
        struct {
            enum tlk_usb_irq_mask_e __attribute__((packed)) irq_mask : 8;
        }; // address: 0x8011800f, offset: 0xf
    };
    uint8_t edps_ptr_low[8]; // address: 0x80118010, offset: 0x10
    union {
        struct {
            unsigned val : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) edps_ptr_high_bit[8];
        uint8_t edps_ptr_high[8]; // address: 0x80118018, offset: 0x18
    };
    uint8_t edps_data[8]; // address: 0x80118020, offset: 0x20
    union {
        struct {
            unsigned rd_ack : 1;
            unsigned rd_stall : 1;
            unsigned set_data0 : 1;
            unsigned set_data1 : 1;
            unsigned reserved : 4;
        } __attribute__((packed)) edps_ct_bit[8];
        struct {
            enum tlk_usb_edps_ct_e __attribute__((packed)) edps_ct : 8;
        }; // address: 0x80118028, offset: 0x28
    };
    uint8_t edps_adr_low[8]; // address: 0x80118030, offset: 0x30
    uint8_t edps_adr_high[8]; // address: 0x80118038, offset: 0x38
    uint8_t usb_iso; // address: 0x80118040, offset: 0x40
    uint8_t usb_irq; // address: 0x80118041, offset: 0x41
    uint8_t usb_mask; // address: 0x80118042, offset: 0x42
    uint8_t usb_max0; // address: 0x80118043, offset: 0x43
    uint8_t usb_min0; // address: 0x80118044, offset: 0x44
    union {
        struct {
            unsigned r_fifo0 : 1;
            unsigned full0 : 1;
            unsigned r_mode00 : 1;
            unsigned edp8_eof : 1;
            unsigned edp8_dma_eof : 3;
            unsigned reserved : 1;
        } __attribute__((packed)) usb_fifo_bit;
        struct {
            enum tlk_usb_usb_fifo_e __attribute__((packed)) usb_fifo : 8;
        }; // address: 0x80118045, offset: 0x45
    };
    uint8_t usb_max; // address: 0x80118046, offset: 0x46
    uint8_t usb_tick; // address: 0x80118047, offset: 0x47
    union {
        struct {
            unsigned sr_cen : 1;
            unsigned sr_clk : 1;
            unsigned r_ram2 : 1;
            unsigned wen_i : 1;
            unsigned r_ram4 : 1;
            unsigned reserved : 3;
        } __attribute__((packed)) usb_ram_bit;
        struct {
            enum tlk_usb_usb_ram_e __attribute__((packed)) usb_ram : 8;
        }; // address: 0x80118048, offset: 0x48
    };
    union {
        struct {
            unsigned usb_blk1 : 3;
            unsigned r_edps_map_manual : 1;
            unsigned r_edps_map_auto : 1;
            unsigned r_edps_sm_map_en : 1;
            unsigned r_edps_map_tgl_en : 1;
            unsigned r_get_sta_map_en : 1;
        } __attribute__((packed)) usb_min1_bit;
        struct {
            enum tlk_usb_usb_min1_e __attribute__((packed)) usb_min1 : 8;
        }; // address: 0x80118049, offset: 0x49
    };
    uint32_t edps_map; // address: 0x8011804a, offset: 0x4a
    uint16_t sof_frame; // address: 0x8011804e, offset: 0x4e
    union {
        struct {
            unsigned r_maxh0 : 1;
            unsigned r_en_ain_maxh0 : 1;
            unsigned reserved : 3;
            unsigned full : 1;
            unsigned full_nz : 1;
            unsigned full_thrd : 1;
        } __attribute__((packed)) edps_maxh0_bit;
        struct {
            enum tlk_usb_edps_maxh0_e __attribute__((packed)) edps_maxh0 : 8;
        }; // address: 0x80118050, offset: 0x50
    };
    union {
        struct {
            unsigned sie_edp : 4;
            unsigned sie_edp_nomap : 4;
        } __attribute__((packed)) edps_edp_r_bit;
        uint8_t edps_edp_r; // address: 0x80118051, offset: 0x51
    };
    uint16_t usb_pid; // address: 0x80118052, offset: 0x52
    uint16_t edps_eptr; // address: 0x80118054, offset: 0x54
    uint16_t edps_udc_ptr; // address: 0x80118056, offset: 0x56
    uint16_t edps_s_ptr; // address: 0x80118058, offset: 0x58
    uint8_t edps_map_en; // address: 0x8011805a, offset: 0x5a
    uint8_t edps_logic_en; // address: 0x8011805b, offset: 0x5b
    uint8_t edps_full_thrd; // address: 0x8011805c, offset: 0x5c
    union {
        struct {
            unsigned pem_event_en : 1;
            unsigned pem_event_sel : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) pem_control_bit;
        struct {
            enum tlk_usb_pem_control_e __attribute__((packed)) pem_control : 8;
        }; // address: 0x8011805d, offset: 0x5d
    };
    uint8_t pem_task_en; // address: 0x8011805e, offset: 0x5e
    uint8_t reserved0; // address: 0x8011805f, offset: 0x5f
    uint32_t tstamp; // address: 0x80118060, offset: 0x60
    uint16_t cal_cyc; // address: 0x80118064, offset: 0x64
    union {
        struct {
            unsigned r_clk_sel_0 : 1;
            unsigned low_speed : 1;
            unsigned r_clk_sel_2 : 1;
            unsigned test_mode : 1;
            unsigned r_clk_sel_o : 4;
        } __attribute__((packed)) usb_ct_bit;
        struct {
            enum tlk_usb_usb_ct_e __attribute__((packed)) usb_ct : 8;
        }; // address: 0x80118066, offset: 0x66
    };
} tlk_usb_reg_t;

#define TLK_USB_BASE_ADDR (0x80118000U)
#define tlk_usb_reg (*(volatile tlk_usb_reg_t *) TLK_USB_BASE_ADDR)

#endif