#ifndef TLK_TLK_ANALOG_REGISTERS_H_
#define TLK_TLK_ANALOG_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_analog_control_e {
    TLK_ANALOG_CONTROL_TX_EN        = TLK_BIT(0),
    TLK_ANALOG_CONTROL_RX_EN        = TLK_BIT(1),
    TLK_ANALOG_CONTROL_MASK_TX_DONE = TLK_BIT(2),
    TLK_ANALOG_CONTROL_MASK_RX_DONE = TLK_BIT(3),
    TLK_ANALOG_CONTROL_CONTIU_ACC   = TLK_BIT(4),
    TLK_ANALOG_CONTROL_RW           = TLK_BIT(5),
    TLK_ANALOG_CONTROL_CYC          = TLK_BIT(6),
    TLK_ANALOG_CONTROL_BUSY         = TLK_BIT(7),
} tlk_analog_control_e;

typedef enum tlk_analog_status_e {
    TLK_ANALOG_STATUS_RX_DONE = TLK_BIT(7),
} tlk_analog_status_e;

typedef enum tlk_analog_irq_status_e {
    TLK_ANALOG_IRQ_STATUS_TXBUFF_IRQ = TLK_BIT(0),
    TLK_ANALOG_IRQ_STATUS_RXBUFF_IRQ = TLK_BIT(1),
} tlk_analog_irq_status_e;

typedef enum tlk_analog_dma_control_e {
    TLK_ANALOG_DMA_CONTROL_CYC1           = TLK_BIT(0),
    TLK_ANALOG_DMA_CONTROL_DMA_EN         = TLK_BIT(1),
    TLK_ANALOG_DMA_CONTROL_AUTO_RXCLR_EN  = TLK_BIT(2),
    TLK_ANALOG_DMA_CONTROL_NDMA_RXDONE_EN = TLK_BIT(3),
} tlk_analog_dma_control_e;

typedef struct __attribute__((packed)) {
    uint8_t address; // address: 0x80140180, offset: 0x0
    uint8_t reserved0; // address: 0x80140181, offset: 0x1
    union {
        struct {
            unsigned tx_en : 1;
            unsigned rx_en : 1;
            unsigned mask_tx_done : 1;
            unsigned mask_rx_done : 1;
            unsigned contiu_acc : 1;
            unsigned rw : 1;
            unsigned cyc : 1;
            unsigned busy : 1;
        } __attribute__((packed)) control_bit;
        struct {
            enum tlk_analog_control_e __attribute__((packed)) control : 8;
        }; // address: 0x80140182, offset: 0x2
    };
    uint8_t length; // address: 0x80140183, offset: 0x3
    uint8_t data[4]; // address: 0x80140184, offset: 0x4
    union {
        struct {
            unsigned rx_count : 4;
            unsigned tx_count : 4;
        } __attribute__((packed)) buff_count_bit;
        uint8_t buff_count; // address: 0x80140188, offset: 0x8
    };
    union {
        struct {
            unsigned reserved : 7;
            unsigned rx_done : 1;
        } __attribute__((packed)) status_bit;
        struct {
            enum tlk_analog_status_e __attribute__((packed)) status : 8;
        }; // address: 0x80140189, offset: 0x9
    };
    union {
        struct {
            unsigned txbuff_irq : 1;
            unsigned rxbuff_irq : 1;
            unsigned reserved : 6;
        } __attribute__((packed)) irq_status_bit;
        struct {
            enum tlk_analog_irq_status_e __attribute__((packed)) irq_status : 8;
        }; // address: 0x8014018a, offset: 0xa
    };
    union {
        struct {
            unsigned cyc1 : 1;
            unsigned dma_en : 1;
            unsigned auto_rxclr_en : 1;
            unsigned ndma_rxdone_en : 1;
            unsigned div_mode : 2;
            unsigned reserved : 2;
        } __attribute__((packed)) dma_control_bit;
        struct {
            enum tlk_analog_dma_control_e __attribute__((packed)) dma_control : 8;
        }; // address: 0x8014018b, offset: 0xb
    };
} tlk_analog_reg_t;

#define TLK_ANALOG_BASE_ADDR (0x80140180U)
#define tlk_analog_reg (*(volatile tlk_analog_reg_t *) TLK_ANALOG_BASE_ADDR)



#define TLK_AREG_BG_CTRL0 0x00
#define TLK_AREG_0x00_SIZE 8

typedef union tlk_areg_bg_ctrl0 {
    struct {
        unsigned bandgap_force_start : 1;
        unsigned bandgap_trim_3v : 3;
        unsigned reserved : 1;
        unsigned dcdc_bpo : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x00;

#define TLK_AREG_POWER0 0x01
#define TLK_AREG_0x01_SIZE 8

typedef union tlk_areg_power0 {
    struct {
        unsigned dcdc_btr : 3;
        unsigned dcdc_vcomp_in_cal_offset : 5;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x01;

#define TLK_AREG_POWER1 0x02
#define TLK_AREG_0x02_SIZE 8

typedef union tlk_areg_power1 {
    struct {
        unsigned dcdc_cal_two_high_en : 1;
        unsigned dcdc_vcomp_in_cal_sel : 1;
        unsigned dcdc_vcomp_cal_en : 1;
        unsigned ldo_native_trim : 2;
        unsigned ir_r_trim_30k : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x02;

// uint8_t reserved0[0x2]; // address: 0x3

#define TLK_AREG_POWER_DOWN0 0x05
#define TLK_AREG_0x05_SIZE 8

typedef union tlk_areg_power_down0 {
    struct {
        unsigned pd_32k_rc : 1;
        unsigned pd_32k_xtal : 1;
        unsigned pd_24m_rc : 1;
        unsigned pd_24m_xtal : 1;
        unsigned pd_pl_all_3v : 1;
        unsigned pd_pl_dcdc_ldo_3v : 1;
        unsigned pd_pl_vbat_ldo_3v : 1;
        unsigned pd_ana_ldo_3v : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x05;

#define TLK_AREG_POWER_DOWN1 0x06
#define TLK_AREG_0x06_SIZE 8

typedef union tlk_areg_power_down1 {
    struct {
        unsigned pd_bbpll_ldo : 1;
        unsigned pd_lc_comp_3v : 1;
        unsigned pd_temp_sensor_3v : 1;
        unsigned pd_vbat_switch : 1;
        unsigned pd_dcore_ldo : 1;
        unsigned pd_sram_ldo : 1;
        unsigned pd_spd_ldo : 1;
        unsigned pd_ret_ldo : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x06;

#define TLK_AREG_POWER2 0x07
#define TLK_AREG_0x07_SIZE 8

typedef union tlk_areg_power2 {
    struct {
        unsigned pm_pdb_uvlo_ib : 1;
        unsigned vdd1p8_otp_en : 1;
        unsigned reserved : 6;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x07;

#define TLK_AREG_POWER3 0x08
#define TLK_AREG_0x08_SIZE 8

typedef union tlk_areg_power3 {
    struct {
        unsigned vbat_ldo1p8_trim_3v : 3;
        unsigned pd_vdd_ram : 1;
        unsigned vbat_aonldo_1p8v : 3;
        unsigned pd_vdd_dcore : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x08;

#define TLK_AREG_POWER4 0x09
#define TLK_AREG_0x09_SIZE 8

typedef union tlk_areg_power4 {
    struct {
        unsigned tr_dcdc_ldo_0p94 : 4;
        unsigned tr_dcdc_ldo_1p8 : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x09;

#define TLK_AREG_POWER5 0x0a
#define TLK_AREG_0x0a_SIZE 8

typedef union tlk_areg_power5 {
    struct {
        unsigned pd_dcdc_ldo_sw : 2;
        unsigned dcdc_s_model_sel : 1;
        unsigned clk_32k_xtal_en : 1;
        unsigned tr_dcdc_oc : 3;
        unsigned reserved : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0a;

#define TLK_AREG_POWER6 0x0b
#define TLK_AREG_0x0b_SIZE 8

typedef union tlk_areg_power6 {
    struct {
        unsigned pd_nvt_0p94 : 1;
        unsigned pd_nvt_1p8 : 1;
        unsigned pd_dcdc_sink : 1;
        unsigned comp_ref_mod_sel : 1;
        unsigned comp_refscale : 2;
        unsigned mscn_pullup_res_en_b : 1;
        unsigned dp_pullup_res_en_b : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0b;

#define TLK_AREG_POWER7 0x0c
#define TLK_AREG_0x0c_SIZE 8

typedef union tlk_areg_power7 {
    struct {
        unsigned dcdc_trim_flash_out : 4;
        unsigned dcdc_trim_soc_out : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0c;

#define TLK_AREG_LC_COMP_CTRL0 0x0d
#define TLK_AREG_0x0d_SIZE 8

typedef union tlk_areg_lc_comp_ctrl0 {
    struct {
        unsigned lc_comp_chn_sel : 3;
        unsigned lc_comp_vbus_inn_en : 1;
        unsigned lc_comp_refsel : 3;
        unsigned pd_lc_comp_10u : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0d;

#define TLK_AREG_POWER8 0x0e
#define TLK_AREG_0x0e_SIZE 8

typedef union tlk_areg_power8 {
    struct {
        unsigned suspend_ldo_trim : 3;
        unsigned lc_cmp_current_option : 1;
        unsigned ram_ldo_trim : 3;
        unsigned lc_cmp_vcm_option : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0e;

#define TLK_AREG_POWER9 0x0f
#define TLK_AREG_0x0f_SIZE 8

typedef union tlk_areg_power9 {
    struct {
        unsigned ret_ldo_trim : 3;
        unsigned ir_receiver_en : 1;
        unsigned dig_ldo_trim : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0f;

#define TLK_AREG_POWER10 0x10
#define TLK_AREG_0x10_SIZE 8

typedef union tlk_areg_power10 {
    struct {
        unsigned dig_ldo_pd_trim_3v : 3;
        unsigned pd_bypass_sram_ldo : 1;
        unsigned dig_ldo_fb_trim_3v : 3;
        unsigned pd_bypass_dcore_ldo : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x10;

#define TLK_AREG_POWER11 0x11
#define TLK_AREG_0x11_SIZE 8

typedef union tlk_areg_power11 {
    struct {
        unsigned test_mux_sel_first : 3;
        unsigned pd_atb_buffer : 1;
        unsigned test_mux_sel_second : 3;
        unsigned ir_transmitter_en : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x11;

#define TLK_AREG_POWER12 0x12
#define TLK_AREG_0x12_SIZE 8

typedef union tlk_areg_power12 {
    struct {
        unsigned dcore_ldo_trim : 4;
        unsigned trim_vref_lcldo_ana_3v : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x12;

#define TLK_AREG_POWER13 0x13
#define TLK_AREG_0x13_SIZE 8

typedef union tlk_areg_power13 {
    struct {
        unsigned vbat_aonldo_3v : 3;
        unsigned a1_bb_power_sw : 1;
        unsigned vbat_lcldo_3v : 3;
        unsigned reserved : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x13;

#define TLK_AREG_IR_CTRL0 0x14
#define TLK_AREG_0x14_SIZE 8

typedef union tlk_areg_ir_ctrl0 {
    struct {
        unsigned ir_rtrim : 3;
        unsigned ir_en_ex_diod : 1;
        unsigned ir_htrim : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x14;

#define TLK_AREG_POWER14 0x15
#define TLK_AREG_0x15_SIZE 8

typedef union tlk_areg_power14 {
    struct {
        unsigned vbat_ldo_trim_3v : 3;
        unsigned reserved : 1;
        unsigned analog_ldo_trim : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x15;

#define TLK_AREG_CLK_CTRL0 0x16
#define TLK_AREG_0x16_SIZE 8

typedef union tlk_areg_clk_ctrl0 {
    struct {
        unsigned rtrim_rc24m_3p3v : 2;
        unsigned tcal_rc32k_3p3v : 2;
        unsigned itrim_xtal32k_3p3v : 2;
        unsigned reserved : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x16;

#define TLK_AREG_PX_PUPD(offset) (TLK_AREG_PX_PUPD0 + offset * 2)
#define TLK_AREG_PX_PUPD0 0x17
#define TLK_AREG_PX_PUPD1 0x19
#define TLK_AREG_PX_PUPD2 0x1b
#define TLK_AREG_PX_PUPD3 0x1d
#define TLK_AREG_PX_PUPD4 0x1f
#define TLK_AREG_PX_PUPD5 0x21

// uint8_t reserved1[0x12]; // address: 0x23

#define TLK_AREG_WD_BUFFER_CLR0 0x35
#define TLK_AREG_0x35_SIZE 8

typedef union tlk_areg_wd_buffer_clr0 {
    struct {
        unsigned poweron_flag : 1;
        unsigned reserved : 7;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x35;

#define TLK_AREG_AFE0X36 0x36

#define TLK_AREG_AFE0X37 0x37

#define TLK_AREG_AFE0X38 0x38

#define TLK_AREG_AFE0X39 0x39

#define TLK_AREG_POWERON_BUFFER_CLR0 0x3a
#define TLK_AREG_0x3a_SIZE 8

typedef union tlk_areg_poweron_buffer_clr0 {
    struct {
        unsigned reboot_flag : 1;
        unsigned sw_reboot_reason : 7;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x3a;

#define TLK_AREG_AFE0X3B 0x3b

#define TLK_AREG_AFE0X3C 0x3c

#define TLK_AREG_R_DLY_XTAL 0x3d

#define TLK_AREG_R_DLY 0x3e

#define TLK_AREG_PX_POLARITY(offset) (TLK_AREG_PX_POLARITY0 + offset * 1)
#define TLK_AREG_PX_POLARITY0 0x3f
#define TLK_AREG_PX_POLARITY1 0x40
#define TLK_AREG_PX_POLARITY2 0x41
#define TLK_AREG_PX_POLARITY3 0x42
#define TLK_AREG_PX_POLARITY4 0x43
#define TLK_AREG_PX_POLARITY5 0x44

#define TLK_AREG_PX_WAKEUP_EN(offset) (TLK_AREG_PX_WAKEUP_EN0 + offset * 1)
#define TLK_AREG_PX_WAKEUP_EN0 0x45
#define TLK_AREG_PX_WAKEUP_EN1 0x46
#define TLK_AREG_PX_WAKEUP_EN2 0x47
#define TLK_AREG_PX_WAKEUP_EN3 0x48
#define TLK_AREG_PX_WAKEUP_EN4 0x49
#define TLK_AREG_PX_WAKEUP_EN5 0x4a

#define TLK_AREG_WAKEUP_CTRL 0x4b
#define TLK_AREG_0x4b_SIZE 8

typedef union tlk_areg_wakeup_ctrl {
    struct {
        unsigned pad_wakeup_en : 1;
        unsigned dig_wakeup_en : 1;
        unsigned timer_wakeup_en : 1;
        unsigned comp_wakeup_en : 1;
        unsigned reserved : 3;
        unsigned shutdown_wakeup_en : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x4b;

#define TLK_AREG_AUTO_PD0 0x4c
#define TLK_AREG_0x4c_SIZE 8

typedef union tlk_areg_auto_pd0 {
    struct {
        unsigned auto_pd_32k_rc : 1;
        unsigned auto_pd_32k_xtal : 1;
        unsigned reserved : 1;
        unsigned auto_pd_24m_xtal : 1;
        unsigned auto_pd_pl_all : 1;
        unsigned auto_pd_dcdc : 1;
        unsigned auto_pd_vbus_ldo : 1;
        unsigned auto_pd_ana_bbpll_temp_ldo : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x4c;

#define TLK_AREG_AUTO_PD1 0x4d
#define TLK_AREG_0x4d_SIZE 8

typedef union tlk_areg_auto_pd1 {
    struct {
        unsigned auto_pd_lc_comp : 1;
        unsigned auto_pd_dcore_sram_ldo : 1;
        unsigned auto_pd_uvlo_ib : 1;
        unsigned auto_pd_vbus_switch : 1;
        unsigned reserved : 2;
        unsigned pwdn_seq_en : 1;
        unsigned isolation_en : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x4d;

#define TLK_AREG_XO_CTRL0 0x4e
#define TLK_AREG_0x4e_SIZE 8

typedef union tlk_areg_xo_ctrl0 {
    struct {
        unsigned xo_quick_setting : 2;
        unsigned xo_isel_pmu : 4;
        unsigned reserved : 1;
        unsigned clk32k_sel : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x4e;

#define TLK_AREG_CLK_CTRL1 0x4f
#define TLK_AREG_0x4f_SIZE 8

typedef union tlk_areg_clk_ctrl1 {
    struct {
        unsigned rc_32k_res_l : 6;
        unsigned rc_32k_cap_sel : 1;
        unsigned rc_24m_cap_sel : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x4f;

#define TLK_AREG_CLK_CTRL2 0x50
#define TLK_AREG_0x50_SIZE 8

typedef union tlk_areg_clk_ctrl2 {
    struct {
        unsigned rc_32k_cap : 3;
        unsigned reserved : 4;
        unsigned rst_xtal_quickstart_cnt : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x50;

#define TLK_AREG_CLK_RC_32K_RES_H 0x51

#define TLK_AREG_CLK_RC_24M_CAP 0x52

// uint8_t reserved2[0xd]; // address: 0x53

#define TLK_AREG_CLK_CNT_32K 0x60

#define TLK_AREG_WAKEUP_STATUS 0x64
#define TLK_AREG_0x64_SIZE 8

typedef union tlk_areg_wakeup_status {
    struct {
        unsigned wakeup_pad : 1;
        unsigned wakeup_dig : 1;
        unsigned wakeup_timer : 1;
        unsigned wakeup_comp : 1;
        unsigned reserved : 3;
        unsigned vbus_on : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x64;

#define TLK_AREG_WAKEUP_SET 0x65
#define TLK_AREG_0x65_SIZE 8

typedef union tlk_areg_wakeup_set {
    struct {
        unsigned reserved : 6;
        unsigned reset_xtal_quick_start_cnt : 1;
        unsigned reset_32k_timer : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x65;

// uint8_t reserved3[0x3]; // address: 0x66

#define TLK_AREG_PG_STATUS 0x69
#define TLK_AREG_0x69_SIZE 8

typedef union tlk_areg_pg_status {
    struct {
        unsigned power_zb : 1;
        unsigned power_usb : 1;
        unsigned power_audio : 1;
        unsigned reserved : 2;
        unsigned pd_sm_busy : 1;
        unsigned vbus_detect : 1;
        unsigned wdt_status : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x69;

// uint8_t reserved4[0x7]; // address: 0x6a

#define TLK_AREG_FLASH_KEY(offset) (TLK_AREG_FLASH_KEY0 + offset * 1)
#define TLK_AREG_FLASH_KEY0 0x71
#define TLK_AREG_FLASH_KEY1 0x72
#define TLK_AREG_FLASH_KEY2 0x73
#define TLK_AREG_FLASH_KEY3 0x74
#define TLK_AREG_FLASH_KEY4 0x75

// uint8_t reserved5; // address: 0x76

#define TLK_AREG_BOOT_CTRL0 0x77
#define TLK_AREG_0x77_SIZE 8

typedef union tlk_areg_boot_ctrl0 {
    struct {
        unsigned secure_boot_en : 1;
        unsigned reserved : 7;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x77;

// uint8_t reserved6; // address: 0x78

#define TLK_AREG_WDT_CTRL 0x79
#define TLK_AREG_0x79_SIZE 8

typedef union tlk_areg_wdt_ctrl {
    struct {
        unsigned wdt_en : 1;
        unsigned reserved : 7;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x79;

#define TLK_AREG_WDT_INTERVAL(offset) (TLK_AREG_WDT_INTERVAL0 + offset * 1)
#define TLK_AREG_WDT_INTERVAL0 0x7a
#define TLK_AREG_WDT_INTERVAL1 0x7b
#define TLK_AREG_WDT_INTERVAL2 0x7c

#define TLK_AREG_POWER_DOWN2 0x7d
#define TLK_AREG_0x7d_SIZE 8

typedef union tlk_areg_power_down2 {
    struct {
        unsigned pd_zb : 1;
        unsigned pd_usb : 1;
        unsigned pd_audio : 1;
        unsigned reserved : 4;
        unsigned pg_clk_en : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x7d;

#define TLK_AREG_RET_CTRL 0x7e
#define TLK_AREG_0x7e_SIZE 8

typedef union tlk_areg_ret_ctrl {
    struct {
        unsigned sram_ret : 4;
        unsigned reserved : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x7e;

#define TLK_AREG_BOOT_CTRL1 0x7f
#define TLK_AREG_0x7f_SIZE 8

typedef union tlk_areg_boot_ctrl1 {
    struct {
        unsigned brom_boot : 1;
        unsigned ret_sram_crc_en : 1;
        unsigned reserved : 1;
        unsigned vbus_detect_pol : 1;
        unsigned wakeup_vbus_en : 1;
        unsigned pad_filter_en : 1;
        unsigned dly_sel_en : 1;
        unsigned softstart_dis : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x7f;

#define TLK_AREG_BBPLL_CTRL0 0x80
#define TLK_AREG_0x80_SIZE 8

typedef union tlk_areg_bbpll_ctrl0 {
    struct {
        unsigned reserved : 1;
        unsigned pd_bbpll_240M_vco : 1;
        unsigned pd_bbpll_240M_div : 1;
        unsigned pd_bbpll_240M_cp : 1;
        unsigned pd_bbpll_240M_pfd : 1;
        unsigned bbpll_240M_cp_fc_en : 1;
        unsigned bbpll_240M_cp_test_en : 1;
        unsigned bbpll_240M_fcal_en : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x80;

#define TLK_AREG_BBPLL_CTRL1 0x81
#define TLK_AREG_0x81_SIZE 8

typedef union tlk_areg_bbpll_ctrl1 {
    struct {
        unsigned bbpll_240M_atb_sel : 4;
        unsigned bbpll_240M_lpf_c1_trim : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x81;

#define TLK_AREG_BBPLL_CTRL2 0x82
#define TLK_AREG_0x82_SIZE 8

typedef union tlk_areg_bbpll_ctrl2 {
    struct {
        unsigned bbpll_240M_lpf_r1_trim : 4;
        unsigned bbpll_240M_lpf_r3_trim : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x82;

#define TLK_AREG_BBPLL_CTRL3 0x83
#define TLK_AREG_0x83_SIZE 8

typedef union tlk_areg_bbpll_ctrl3 {
    struct {
        unsigned bbpll_240M_cp_ileakn : 5;
        unsigned bbpll_240M_pfd_dly_trim : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x83;

#define TLK_AREG_BBPLL_CTRL4 0x84
#define TLK_AREG_0x84_SIZE 8

typedef union tlk_areg_bbpll_ctrl4 {
    struct {
        unsigned bbpll_240M_cp_ileakp : 5;
        unsigned bbpll_240M_vco_itrim : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x84;

#define TLK_AREG_BBPLL_CTRL5 0x85
#define TLK_AREG_0x85_SIZE 8

typedef union tlk_areg_bbpll_ctrl5 {
    struct {
        unsigned bbpll_240M_cp_itrim : 5;
        unsigned bbpll_240M_refclk_sel : 2;
        unsigned bbpll_240M_lock_en2x : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x85;

#define TLK_AREG_BBPLL_CTRL6 0x86
#define TLK_AREG_0x86_SIZE 8

typedef union tlk_areg_bbpll_ctrl6 {
    struct {
        unsigned bbpll_240M_div_ratio : 5;
        unsigned bbpll_240M_vco_common : 2;
        unsigned bbpll_240M_lock_rst : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x86;

#define TLK_AREG_BBPLL_CTRL7 0x87
#define TLK_AREG_0x87_SIZE 8

typedef union tlk_areg_bbpll_ctrl7 {
    struct {
        unsigned en_clk24M_tosar_0p8v : 1;
        unsigned reserved : 7;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x87;

#define TLK_AREG_BBPLL_CTRL8 0x88
#define TLK_AREG_0x88_SIZE 8

typedef union tlk_areg_bbpll_ctrl8 {
    struct {
        unsigned reserved : 5;
        unsigned bbpll_lock_detector : 1;
        unsigned lc_comp_out : 1;
        unsigned xo_ready_ana : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x88;

// uint8_t reserved7; // address: 0x89

#define TLK_AREG_XO_CTRL1 0x8a
#define TLK_AREG_0x8a_SIZE 8

typedef union tlk_areg_xo_ctrl1 {
    struct {
        unsigned xo_cdac_ana : 6;
        unsigned xo_mode_ana : 1;
        unsigned xo_cap_off_ana : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x8a;

#define TLK_AREG_XO_CTRL2 0x8b
#define TLK_AREG_0x8b_SIZE 8

typedef union tlk_areg_xo_ctrl2 {
    struct {
        unsigned xo_force_amp_ana : 1;
        unsigned xo_dyn_cap_ana : 1;
        unsigned xo_dyn_isel_ana : 1;
        unsigned xo_cnt_off_ana : 1;
        unsigned xo_ldo_trim_ana : 2;
        unsigned xo_ldo_bypass_ana : 1;
        unsigned xo_ldo_boost_ana : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x8b;

#define TLK_AREG_XO_ADC_PGA_CTRL 0x8c
#define TLK_AREG_0x8c_SIZE 8

typedef union tlk_areg_xo_adc_pga_ctrl {
    struct {
        unsigned xo_ldo_force_ana : 1;
        unsigned xo_en_clk_ana_ana : 1;
        unsigned td_asdmdem_en : 1;
        unsigned td_asdm_dith : 2;
        unsigned td_asdm_dith_en : 1;
        unsigned td_ctr_iref : 1;
        unsigned td_lven : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x8c;

#define TLK_AREG_PGA_CTRL0 0x8d
#define TLK_AREG_0x8d_SIZE 8

typedef union tlk_areg_pga_ctrl0 {
    struct {
        unsigned td_jbsel : 2;
        unsigned td_mute_pga : 1;
        unsigned td_inmute_pga : 1;
        unsigned td_pgavol_in : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x8d;

#define TLK_AREG_PGA_CTRL1 0x8e
#define TLK_AREG_0x8e_SIZE 8

typedef union tlk_areg_pga_ctrl1 {
    struct {
        unsigned td_0p6_en : 1;
        unsigned td_vmdscl : 2;
        unsigned td_pd_inppga : 1;
        unsigned td_pd_pgabuf : 1;
        unsigned td_pd_pgaboost : 1;
        unsigned td_pd_asdm : 1;
        unsigned td_pd_bias : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x8e;

#define TLK_AREG_PGA_CTRL2 0x8f
#define TLK_AREG_0x8f_SIZE 8

typedef union tlk_areg_pga_ctrl2 {
    struct {
        unsigned reserved0 : 1;
        unsigned audio_vmid_pd : 1;
        unsigned td_asdm_dithin : 1;
        unsigned reserved1 : 5;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x8f;

// uint8_t reserved8[0x2d]; // address: 0x90

#define TLK_AREG_PC_IE 0xbd

// uint8_t reserved9; // address: 0xbe

#define TLK_AREG_PC_DS 0xbf

#define TLK_AREG_PC_PD 0xc0

#define TLK_AREG_PC_PU 0xc1

#define TLK_AREG_PD_IE 0xc2

#define TLK_AREG_PD_DS 0xc3

#define TLK_AREG_PD_PD 0xc4

#define TLK_AREG_PD_PU 0xc5

#define TLK_AREG_CAL_CTRL0 0xc6
#define TLK_AREG_0xc6_SIZE 8

typedef union tlk_areg_cal_ctrl0 {
    struct {
        unsigned cal_32k_en : 1;
        unsigned cal_32k_len : 1;
        unsigned cal_32k_new : 1;
        unsigned cal_32k_test : 1;
        unsigned cal_32k_wait_len : 3;
        unsigned cal_32k_fine : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xc6;

#define TLK_AREG_CAL_CTRL1 0xc7
#define TLK_AREG_0xc7_SIZE 8

typedef union tlk_areg_cal_ctrl1 {
    struct {
        unsigned cal_24m_en : 1;
        unsigned cal_24m_len : 1;
        unsigned cal_24m_new : 1;
        unsigned reserved : 5;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xc7;

#define TLK_AREG_CAL_24M_WAIT_LEN 0xc8

#define TLK_AREG_CAL_RC_32K_CAP 0xc9

#define TLK_AREG_CAL_CTRL2 0xca
#define TLK_AREG_0xca_SIZE 8

typedef union tlk_areg_cal_ctrl2 {
    struct {
        unsigned cal_rc_32k_res : 6;
        unsigned reserved : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xca;

#define TLK_AREG_CAL_RC_24M_CAP 0xcb

#define TLK_AREG_CAL_32K_CNT 0xcc

#define TLK_AREG_CAL_24M_CNT_L 0xce

#define TLK_AREG_CAL_CTRL3 0xcf
#define TLK_AREG_0xcf_SIZE 8

typedef union tlk_areg_cal_ctrl3 {
    struct {
        unsigned cal_24m_cnt_h : 3;
        unsigned reserved : 3;
        unsigned cal_32k_done : 1;
        unsigned cal_24m_done : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xcf;

// uint8_t reserved10[0x14]; // address: 0xd0

#define TLK_AREG_RNG_CTRL0 0xe4
#define TLK_AREG_0xe4_SIZE 8

typedef union tlk_areg_rng_ctrl0 {
    struct {
        unsigned rng_mode : 5;
        unsigned rng_rd_en : 1;
        unsigned reserved : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xe4;

#define TLK_AREG_RNG_DATA 0xe5

#define TLK_AREG_RNG_CTRL1 0xe7
#define TLK_AREG_0xe7_SIZE 8

typedef union tlk_areg_rng_ctrl1 {
    struct {
        unsigned rng_seed : 1;
        unsigned rng_seed_d1 : 1;
        unsigned rng_seed_filter : 1;
        unsigned reserved : 5;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xe7;

#define TLK_AREG_RNG_CTRL2 0xe8
#define TLK_AREG_0xe8_SIZE 8

typedef union tlk_areg_rng_ctrl2 {
    struct {
        unsigned rng_valid_rd : 1;
        unsigned reserved : 7;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xe8;

#define TLK_AREG_RNG_CTRL3 0xe9
#define TLK_AREG_0xe9_SIZE 8

typedef union tlk_areg_rng_ctrl3 {
    struct {
        unsigned rng_filter_valid_rd : 1;
        unsigned reserved : 7;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xe9;

// uint8_t reserved11[0x2]; // address: 0xea

#define TLK_AREG_ADC_CONTROL 0xec
#define TLK_AREG_0xec_SIZE 8

typedef union tlk_areg_adc_control {
    struct {
        unsigned adc_resolution : 2;
        unsigned reserved0 : 4;
        unsigned adc_en_diffm : 1;
        unsigned reserved1 : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xec;

// uint8_t reserved12[0x5]; // address: 0xed

#define TLK_AREG_ADC_DATA_TRANSFER 0xf2
#define TLK_AREG_0xf2_SIZE 8

typedef union tlk_areg_adc_data_transfer {
    struct {
        unsigned reserved0 : 3;
        unsigned r_auto_not_en : 1;
        unsigned reserved1 : 3;
        unsigned done_sel : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xf2;

#define TLK_AREG_ADC_DATA_SAMPLE 0xf3
#define TLK_AREG_0xf3_SIZE 8

typedef union tlk_areg_adc_data_sample {
    struct {
        unsigned hold_md : 1;
        unsigned dwa_en : 1;
        unsigned ana_rd_en : 1;
        unsigned reserved : 5;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xf3;

#define TLK_AREG_ADC_CLK 0xf4
#define TLK_AREG_0xf4_SIZE 8

typedef union tlk_areg_adc_clk {
    struct {
        unsigned adc_div_mode : 4;
        unsigned reserved : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xf4;

// uint8_t reserved13; // address: 0xf5

#define TLK_AREG_ADC_DATA_STATUS 0xf6
#define TLK_AREG_0xf6_SIZE 8

typedef union tlk_areg_adc_data_status {
    struct {
        unsigned m_adc_valid : 1;
        unsigned reserved : 7;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xf6;

#define TLK_AREG_ADC_MISC 0xf7

// uint8_t reserved14; // address: 0xf9

#define TLK_AREG_ADC_AIN_SCALE 0xfa
#define TLK_AREG_0xfa_SIZE 8

typedef union tlk_areg_adc_ain_scale {
    struct {
        unsigned adc_itrim_preamp : 2;
        unsigned adc_itrim_vrefbuf : 2;
        unsigned adc_itrim_vcmbuf : 2;
        unsigned reserved : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xfa;

// uint8_t reserved15; // address: 0xfb

#define TLK_AREG_ADC_PGA 0xfc
#define TLK_AREG_0xfc_SIZE 8

typedef union tlk_areg_adc_pga {
    struct {
        unsigned reserved0 : 5;
        unsigned adc_pd : 1;
        unsigned reserved1 : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xfc;


#endif