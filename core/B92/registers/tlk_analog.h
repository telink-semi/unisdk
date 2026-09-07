#ifndef TLK_TLK_ANALOG_REGISTERS_H_
#define TLK_TLK_ANALOG_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_analog_control_e {
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
            unsigned reserved : 1;
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
            unsigned reserved : 4;
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
        unsigned bandgap_test_out_sel : 1;
        unsigned bandgap_trim_3v : 3;
        unsigned pd_bg_ts : 1;
        unsigned ldo_main_trim : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x00;

#define TLK_AREG_POWER0 0x01
#define TLK_AREG_0x01_SIZE 8

typedef union tlk_areg_power0 {
    struct {
        unsigned bbpll_ldo_trim : 3;
        unsigned dcdc_lc_en : 1;
        unsigned ana_ldo_trim : 3;
        unsigned mux_32k_xtal : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x01;

#define TLK_AREG_POWER1 0x02
#define TLK_AREG_0x02_SIZE 8

typedef union tlk_areg_power1 {
    struct {
        unsigned ldo_ret_trim : 3;
        unsigned ldo_flash_bypass_en : 1;
        unsigned ldo_spd_trim : 3;
        unsigned flash_vo_2p2_en : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x02;

#define TLK_AREG_POWER_CLK_CTRL0 0x03
#define TLK_AREG_0x03_SIZE 8

typedef union tlk_areg_power_clk_ctrl0 {
    struct {
        unsigned en_ton_h_1p2 : 2;
        unsigned en_ton_h_2p0 : 2;
        unsigned force_start_32k : 1;
        unsigned low_power_32k : 1;
        unsigned trim_b_32k : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x03;

#define TLK_AREG_POWER_CLK_CTRL1 0x04
#define TLK_AREG_0x04_SIZE 8

typedef union tlk_areg_power_clk_ctrl1 {
    struct {
        unsigned clk_32k_rc_freq_sel : 2;
        unsigned clk_32k_rc_temp_cal : 2;
        unsigned spd_ana_ldo_trim : 3;
        unsigned pd_ibias_32k_1p0v : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x04;

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
        unsigned pd_pl_vbus_ldo_3v : 1;
        unsigned pd_ana_ldo : 1;
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
        unsigned pd_vbus_switch : 1;
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
        unsigned pm_pdb_uvlo_ib_1p0v : 1;
        unsigned pm_pd_4m_rcosc_1p0v : 1;
        unsigned pm_ldo_flash_pd_1p0v : 1;
        unsigned reserved : 5;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x07;

// uint8_t reserved0; // address: 0x8

#define TLK_AREG_POWER3 0x09
#define TLK_AREG_0x09_SIZE 8

typedef union tlk_areg_power3 {
    struct {
        unsigned tr_dcdc_ldo_1p2 : 2;
        unsigned bbpll_freq_sel : 2;
        unsigned tr_ldo_2p0 : 2;
        unsigned pd_sw_dcore : 1;
        unsigned pd_sw_sram : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x09;

#define TLK_AREG_POWER4 0x0a
#define TLK_AREG_0x0a_SIZE 8

typedef union tlk_areg_power4 {
    struct {
        unsigned pd_dcdc_ldo_sw : 2;
        unsigned tr_oc_1p2 : 3;
        unsigned tr_oc_2p0 : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0a;

#define TLK_AREG_POWER5 0x0b
#define TLK_AREG_0x0b_SIZE 8

typedef union tlk_areg_power5 {
    struct {
        unsigned pd_nvt_1p2 : 1;
        unsigned pd_nvt_2p0 : 1;
        unsigned pm_pd_voice_det_1p0v : 1;
        unsigned comp_ref_mod_sel : 1;
        unsigned comp_refscale : 2;
        unsigned mscn_pullup_res_en_b : 1;
        unsigned dp_pullup_res_en_b : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0b;

#define TLK_AREG_POWER6 0x0c
#define TLK_AREG_0x0c_SIZE 8

typedef union tlk_areg_power6 {
    struct {
        unsigned tr_dcdc_2p0 : 3;
        unsigned pd_lpc_diff : 1;
        unsigned tr_dcdc_1p2 : 3;
        unsigned pd_mic_bias : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0c;

#define TLK_AREG_LC_COMP_CTRL0 0x0d
#define TLK_AREG_0x0d_SIZE 8

typedef union tlk_areg_lc_comp_ctrl0 {
    struct {
        unsigned lc_comp_chn_sel : 3;
        unsigned reserved : 1;
        unsigned lc_comp_refsel : 3;
        unsigned pd_lc_comp_10u : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x0d;

#define TLK_AREG_PX_PUPD(offset) (TLK_AREG_PX_PUPD0 + offset * 2)
#define TLK_AREG_PX_PUPD0 0x0e
#define TLK_AREG_PX_PUPD1 0x10
#define TLK_AREG_PX_PUPD2 0x12
#define TLK_AREG_PX_PUPD3 0x14
#define TLK_AREG_PX_PUPD4 0x16

#define TLK_AREG_POWER7 0x18
#define TLK_AREG_0x18_SIZE 8

typedef union tlk_areg_power7 {
    struct {
        unsigned trim_voice_det : 3;
        unsigned cdc_rst : 1;
        unsigned trim_vbat_aoldo : 3;
        unsigned pm_pd_bg_lc : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x18;

#define TLK_AREG_POWER8 0x19
#define TLK_AREG_0x19_SIZE 8

typedef union tlk_areg_power8 {
    struct {
        unsigned trim_vbat_lcldo : 3;
        unsigned chg_ldo_sw_3p3v_1p8v : 1;
        unsigned trim_vbat_ldo : 3;
        unsigned pd_vbatlao_1p2_aon : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x19;

#define TLK_AREG_CDC_CTRL0 0x1a
#define TLK_AREG_0x1a_SIZE 8

typedef union tlk_areg_cdc_ctrl0 {
    struct {
        unsigned pm_cdc_ldo_adc_pd_1p0v : 1;
        unsigned pm_cdc_ldo_dac_pd_1p0v : 1;
        unsigned pm_cdc_ldo_pa_pd_1p0v : 1;
        unsigned pm_cdc_ldo_pa_bypass_en_1v : 1;
        unsigned pm_cdc_ldo_adc_bypass_en_1v : 1;
        unsigned pm_cdc_ldo_dac_bypass_en_1v : 1;
        unsigned cdc_ldo_vo_test_sel : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x1a;

#define TLK_AREG_POWER9 0x1b
#define TLK_AREG_0x1b_SIZE 8

typedef union tlk_areg_power9 {
    struct {
        unsigned load_enhance : 2;
        unsigned en_retldo_0p8 : 1;
        unsigned atb_in_dcore : 1;
        unsigned ldo_flash_trim : 3;
        unsigned atb_1p0_aon : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x1b;

#define TLK_AREG_POWER10 0x1c
#define TLK_AREG_0x1c_SIZE 8

typedef union tlk_areg_power10 {
    struct {
        unsigned chg_ldo_2p0_sel : 2;
        unsigned chg_ldo_1p2_sel : 2;
        unsigned pm_pd_bgr_standby_1p0v : 1;
        unsigned dfbias_h : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x1c;

#define TLK_AREG_POWER11 0x1d
#define TLK_AREG_0x1d_SIZE 8

typedef union tlk_areg_power11 {
    struct {
        unsigned trim_ldo_dcore : 5;
        unsigned en_vo_h_dcore : 1;
        unsigned mux_dcore_select : 1;
        unsigned usb_detect_irq_mask : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x1d;

#define TLK_AREG_POWER12 0x1e
#define TLK_AREG_0x1e_SIZE 8

typedef union tlk_areg_power12 {
    struct {
        unsigned trim_ldo_sram : 5;
        unsigned pd_vdd_codec_1p0v : 1;
        unsigned lpc_irq_mask : 1;
        unsigned en_vo_h_sram : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x1e;

#define TLK_AREG_POWER13 0x1f
#define TLK_AREG_0x1f_SIZE 8

typedef union tlk_areg_power13 {
    struct {
        unsigned diag_hv_ana_sw_en : 1;
        unsigned charger_control_sel : 1;
        unsigned pm_rc_4m_sel_1p0v : 2;
        unsigned pm_rc_4m_ctrim_1p0v : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x1f;

#define TLK_AREG_LED_CTRL0 0x20
#define TLK_AREG_0x20_SIZE 8

typedef union tlk_areg_led_ctrl0 {
    struct {
        unsigned led2_trim_bit : 3;
        unsigned led2_enable : 1;
        unsigned ps_pe0 : 1;
        unsigned ps_pe1 : 1;
        unsigned ps_pe2 : 1;
        unsigned ps_pe3 : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x20;

#define TLK_AREG_LED_CTRL1 0x21
#define TLK_AREG_0x21_SIZE 8

typedef union tlk_areg_led_ctrl1 {
    struct {
        unsigned led0_trim_bit : 3;
        unsigned led0_enable : 1;
        unsigned led1_trim_bit : 3;
        unsigned led1_enable : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x21;

#define TLK_AREG_POWER14 0x22
#define TLK_AREG_0x22_SIZE 8

typedef union tlk_areg_power14 {
    struct {
        unsigned pm_chg_rx_en_1p0v : 1;
        unsigned pm_chg_tx_en_1p0v : 1;
        unsigned pm_chg_txtx_base_1p0v : 1;
        unsigned pm_chg_en_1p0v : 1;
        unsigned pm_rc_24m_4m_1p0v : 1;
        unsigned pm_en_bypass_ldo_sram_1p0v : 1;
        unsigned xo_lp_en : 1;
        unsigned clk_auto_lp_en : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x22;

#define TLK_AREG_PF_PUPD 0x23

#define TLK_AREG_CHG_CTRL 0x25
#define TLK_AREG_0x25_SIZE 8

typedef union tlk_areg_chg_ctrl {
    struct {
        unsigned chg_state_cc_mode_current : 1;
        unsigned chg_state_en : 1;
        unsigned chg_state_en_cc : 1;
        unsigned chg_state_en_cv : 1;
        unsigned chg_state_error : 1;
        unsigned chg_state_stdby : 1;
        unsigned reserved : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x25;

#define TLK_AREG_CDC_CTRL1 0x26
#define TLK_AREG_0x26_SIZE 8

typedef union tlk_areg_cdc_ctrl1 {
    struct {
        unsigned cdc_mbvsel_aon : 1;
        unsigned cdc_pdmicbias_aon : 1;
        unsigned reserved0 : 2;
        unsigned cdc_byp_aon : 1;
        unsigned reserved1 : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x26;

// uint8_t reserved1[0xe]; // address: 0x27

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
        unsigned reserved0 : 2;
        unsigned sw_reboot_reason : 4;
        unsigned reserved1 : 1;
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
        unsigned mdec_wakeup_en : 1;
        unsigned ctb_wakeup_en : 1;
        unsigned vad_wakeup_en : 1;
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
        unsigned auto_pd_4m_rcosc : 1;
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
        unsigned auto_pd_flash_ldo : 1;
        unsigned reserved : 1;
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
        unsigned xo_48m_pmu_en : 1;
        unsigned clk32k_sel : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x4e;

#define TLK_AREG_CLK_CTRL0 0x4f
#define TLK_AREG_0x4f_SIZE 8

typedef union tlk_areg_clk_ctrl0 {
    struct {
        unsigned rc_32k_cap : 6;
        unsigned rc_32k_cap_sel : 1;
        unsigned rc_24m_cap_sel : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x4f;

#define TLK_AREG_CLK_CTRL1 0x50
#define TLK_AREG_0x50_SIZE 8

typedef union tlk_areg_clk_ctrl1 {
    struct {
        unsigned rc_32k_res_l : 6;
        unsigned reserved : 1;
        unsigned rst_xtal_quickstart_cnt : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x50;

#define TLK_AREG_CLK_RC_32K_RES_H 0x51

#define TLK_AREG_CLK_RC_24M_CAP 0x52

#define TLK_AREG_MDEC_CTRL0 0x53
#define TLK_AREG_0x53_SIZE 8

typedef union tlk_areg_mdec_ctrl0 {
    struct {
        unsigned mdec_sel : 5;
        unsigned reserved : 2;
        unsigned mdec_rst : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x53;

#define TLK_AREG_MDEC_CTRL1 0x54
#define TLK_AREG_0x54_SIZE 8

typedef union tlk_areg_mdec_ctrl1 {
    struct {
        unsigned mdec_ctrl_bit : 4;
        unsigned reserved : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x54;

#define TLK_AREG_CTB_CTRL0 0x55
#define TLK_AREG_0x55_SIZE 8

typedef union tlk_areg_ctb_ctrl0 {
    struct {
        unsigned ctb_samp_num_sel : 2;
        unsigned reserved0 : 2;
        unsigned ctb_manual_en : 1;
        unsigned ctb_auto_en : 1;
        unsigned reserved1 : 1;
        unsigned ctb_rst : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x55;

#define TLK_AREG_CTB_CHANNEL_EN 0x56

#define TLK_AREG_CTB_CTRL1 0x57
#define TLK_AREG_0x57_SIZE 8

typedef union tlk_areg_ctb_ctrl1 {
    struct {
        unsigned ctb_interval_sel : 4;
        unsigned ctb_th_h : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x57;

#define TLK_AREG_CTB_MASK 0x58

#define TLK_AREG_VAD_CTRL0 0x59
#define TLK_AREG_0x59_SIZE 8

typedef union tlk_areg_vad_ctrl0 {
    struct {
        unsigned vad_threhold_l : 6;
        unsigned reserved : 1;
        unsigned rst_vad_n : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x59;

#define TLK_AREG_VAD_CTRL1 0x5a
#define TLK_AREG_0x5a_SIZE 8

typedef union tlk_areg_vad_ctrl1 {
    struct {
        unsigned vad_threhold_h : 6;
        unsigned reserved : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x5a;

// uint8_t reserved2[0x5]; // address: 0x5b

#define TLK_AREG_CLK_CNT_32K 0x60

#define TLK_AREG_WAKEUP_STATUS 0x64
#define TLK_AREG_0x64_SIZE 8

typedef union tlk_areg_wakeup_status {
    struct {
        unsigned wakeup_pad : 1;
        unsigned wakeup_dig : 1;
        unsigned wakeup_timer : 1;
        unsigned wakeup_comp : 1;
        unsigned wakeup_mdec : 1;
        unsigned wakeup_ctb : 1;
        unsigned wakeup_vad : 1;
        unsigned watchdog : 1;
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
        unsigned pd_bg_vbus_ldo : 1;
        unsigned vbus_detect : 1;
        unsigned wdt_status : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x69;

#define TLK_AREG_MDEC_RCV_DATA_H 0x6a
#define TLK_AREG_0x6a_SIZE 8

typedef union tlk_areg_mdec_rcv_data_h {
    struct {
        unsigned data0 : 3;
        unsigned reserved : 1;
        unsigned data1 : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x6a;

#define TLK_AREG_MDEC_RCV_DATA_REVERSED(offset) (TLK_AREG_MDEC_RCV_DATA_REVERSED0 + offset * 1)
#define TLK_AREG_MDEC_RCV_DATA_REVERSED0 0x6b
#define TLK_AREG_MDEC_RCV_DATA_REVERSED1 0x6c
#define TLK_AREG_MDEC_RCV_DATA_REVERSED2 0x6d
#define TLK_AREG_MDEC_RCV_DATA_REVERSED3 0x6e

#define TLK_AREG_CTB_STS 0x6f
#define TLK_AREG_0x6f_SIZE 8

typedef union tlk_areg_ctb_sts {
    struct {
        unsigned ctb_dout : 4;
        unsigned ctb_channel : 3;
        unsigned reserved : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x6f;

#define TLK_AREG_CTB_IRQ_STS 0x70

// uint8_t reserved4[0x8]; // address: 0x71

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
        unsigned sram_ret : 3;
        unsigned reserved0 : 1;
        unsigned sram_slp : 3;
        unsigned reserved1 : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x7e;

#define TLK_AREG_BOOT_CTRL1 0x7f
#define TLK_AREG_0x7f_SIZE 8

typedef union tlk_areg_boot_ctrl1 {
    struct {
        unsigned brom_boot : 1;
        unsigned ret_sram_crc_en : 1;
        unsigned secureboot_en : 1;
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
        unsigned bbpll_m_div_i : 5;
        unsigned bbpll_pfd_mux_sel : 3;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x80;

#define TLK_AREG_BBPLL_CTRL1 0x81
#define TLK_AREG_0x81_SIZE 8

typedef union tlk_areg_bbpll_ctrl1 {
    struct {
        unsigned bbpll_kvco_sel : 3;
        unsigned bbpll_lpf_r_trim : 1;
        unsigned bbpll_lpf_c1_trim : 1;
        unsigned bbpll_lpf_c2_trim : 1;
        unsigned bbpll_lck_det_rstb : 1;
        unsigned vco_pd : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x81;

#define TLK_AREG_BBPLL_ADC_CTRL 0x82
#define TLK_AREG_0x82_SIZE 8

typedef union tlk_areg_bbpll_adc_ctrl {
    struct {
        unsigned bbpll_ibuf_up : 2;
        unsigned bbpll_cp_sel : 2;
        unsigned pll_rst : 1;
        unsigned reserved : 1;
        unsigned clk_24m_to_sar_en : 1;
        unsigned bbpll_pd : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x82;

// uint8_t reserved5[0x5]; // address: 0x83

#define TLK_AREG_XO_CAL_CTRL 0x88
#define TLK_AREG_0x88_SIZE 8

typedef union tlk_areg_xo_cal_ctrl {
    struct {
        unsigned doubler_cal_code : 5;
        unsigned doubler_cal_done : 1;
        unsigned lc_comp_out : 1;
        unsigned xo_ready_ana : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x88;

// uint8_t reserved6; // address: 0x89

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

#define TLK_AREG_XO_CTRL3 0x8c
#define TLK_AREG_0x8c_SIZE 8

typedef union tlk_areg_xo_ctrl3 {
    struct {
        unsigned xo_ldo_force_ana : 1;
        unsigned xo_en_clk_ana_ana : 1;
        unsigned reserved : 6;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0x8c;

// uint8_t reserved7[0x30]; // address: 0x8d

#define TLK_AREG_PC_IE 0xbd

#define TLK_AREG_PC_PUPD 0xbe

#define TLK_AREG_PC_DS 0xbf

#define TLK_AREG_PD_IE 0xc0

#define TLK_AREG_PD_PUPD 0xc1

#define TLK_AREG_PD_DS 0xc2

// uint8_t reserved8[0x3]; // address: 0xc3

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

// uint8_t reserved9[0x1a]; // address: 0xd0

#define TLK_AREG_ADC_VREF 0xea
#define TLK_AREG_0xea_SIZE 8

typedef union tlk_areg_adc_vref {
    struct {
        unsigned vref_m : 2;
        unsigned reserved : 6;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xea;

#define TLK_AREG_ADC_AIN_SEL 0xeb
#define TLK_AREG_0xeb_SIZE 8

typedef union tlk_areg_adc_ain_sel {
    struct {
        unsigned ain_neg : 4;
        unsigned ain_pos : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xeb;

#define TLK_AREG_ADC_CONTROL 0xec
#define TLK_AREG_0xec_SIZE 8

typedef union tlk_areg_adc_control {
    struct {
        unsigned adc_resolution : 2;
        unsigned reserved0 : 4;
        unsigned adc_en_diff_m : 1;
        unsigned reserved1 : 1;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xec;

// uint8_t reserved10; // address: 0xed

#define TLK_AREG_ADC_TSAMPLE_M 0xee
#define TLK_AREG_0xee_SIZE 8

typedef union tlk_areg_adc_tsample_m {
    struct {
        unsigned tsample_cycle_m : 4;
        unsigned reserved : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xee;

#define TLK_AREG_ADC_R_MAX_MC_L 0xef

// uint8_t reserved11; // address: 0xf0

#define TLK_AREG_ADC_R_MAX 0xf1
#define TLK_AREG_0xf1_SIZE 8

typedef union tlk_areg_adc_r_max {
    struct {
        unsigned r_max_s : 4;
        unsigned reserved : 2;
        unsigned r_max_mc_h : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xf1;

#define TLK_AREG_ADC_CHN 0xf2
#define TLK_AREG_0xf2_SIZE 8

typedef union tlk_areg_adc_chn {
    struct {
        unsigned reserved0 : 2;
        unsigned chn_en_m : 1;
        unsigned reserved1 : 1;
        unsigned max_scnt : 2;
        unsigned reserved2 : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xf2;

#define TLK_AREG_ADC_DATA_SAMPLE 0xf3
#define TLK_AREG_0xf3_SIZE 8

typedef union tlk_areg_adc_data_sample {
    struct {
        unsigned not_sample_data : 1;
        unsigned reserved : 7;
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

// uint8_t reserved12; // address: 0xf5

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

#define TLK_AREG_ADC_VREF_VBAT_DIV 0xf9
#define TLK_AREG_0xf9_SIZE 8

typedef union tlk_areg_adc_vref_vbat_div {
    struct {
        unsigned reserved0 : 2;
        unsigned div : 2;
        unsigned reserved1 : 4;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xf9;

#define TLK_AREG_ADC_AIN_SCALE 0xfa
#define TLK_AREG_0xfa_SIZE 8

typedef union tlk_areg_adc_ain_scale {
    struct {
        unsigned adc_itrim_preamp : 2;
        unsigned adc_itrim_vrefbuf : 2;
        unsigned adc_itrim_vcmbuf : 2;
        unsigned sel_ain_scale : 2;
    } __attribute__((packed)) bit;
    uint8_t raw;
} tlk_areg_0xfa;

// uint8_t reserved13; // address: 0xfb

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