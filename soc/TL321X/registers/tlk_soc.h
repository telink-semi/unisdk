/********************************************************************************************************
 * @file    soc.h
 *
 * @brief   This is the header file for TL321X
 *
 * @author  Driver Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#pragma once

/*Attention: for all the 2-byte address operation (like REG_ADDR16,write_reg16,read_reg16) ,
 *           the address parameter should  be multiple of 2. For example:REG_ADDR16(0x2),REG_ADDR16(0x4).
 *
 *           for all the 4-byte address operation (like REG_ADDR32,write_reg32,read_reg32) ,
 *           the address parameter should  be multiple of 4. For example:write_reg32(0x4),write_reg32(0x8).
*/
#define FLASH_R_BASE_ADDR     0x20000000
#define REG_RW_BASE_ADDR      0x80000000

#define ILM_BASE              (0x68000)
#define DLM_BASE              (0x80000)

#define SC_BASE_ADDR          0x140800
#define SC_BB_BASE_ADDR       0x80170C00

#define reg_mspi_mode         REG_ADDR8(SC_BASE_ADDR)

enum
{
    TLK_CHIP_VERSION_A0 = 0x00,
    TLK_CHIP_VERSION_A1 = 0x01,
    TLK_CHIP_VERSION_A2 = 0x81,
};

enum
{
    FLD_MSPI_MOD        = TLK_BIT_RNG(0, 3),
    FLD_MSPI_DIV_IN_SEL = TLK_BIT_RNG(4, 5),
};

#define reg_gspi_clk_set REG_ADDR16(SC_BASE_ADDR + 0x2)

enum
{
    FLD_GSPI_CLK_MOD    = TLK_BIT_RNG(0, 7),
    FLD_GSPI_DIV_IN_SEL = TLK_BIT_RNG(8, 9), //0:rc24m;   1:xtl24m;  2:pll;
};

#define reg_sdm_step REG_ADDR16(SC_BASE_ADDR + 0x06)

enum
{
    FLD_SDM_STEP     = TLK_BIT_RNG(0, 14),
    FLD_SDM_CLK_EN_0 = TLK_BIT(15),
};

#define reg_i2s_step REG_ADDR16(SC_BASE_ADDR + 0x08)

enum
{
    FLD_I2S_STEP     = TLK_BIT_RNG(0, 14),
    FLD_I2S_CLK_EN_0 = TLK_BIT(15),
};

#define reg_sdm_mod    REG_ADDR16(SC_BASE_ADDR + 0x2a)


#define reg_i2s_mod    REG_ADDR16(SC_BASE_ADDR + 0x0a)
#define reg_cache_init REG_ADDR8(SC_BASE_ADDR + 0x10)

enum
{
    FLD_CACHE_I_DISABLE_INIT = TLK_BIT(6),
    FLD_CACHE_D_DISABLE_INIT = TLK_BIT(7),
};

#define reg_eoc_ts_value REG_ADDR8(SC_BASE_ADDR + 0x11)

#define reg_mcu_ctrl     REG_ADDR32(SC_BASE_ADDR + 0x14)
#define reg_mcu_ctrl0    REG_ADDR8(SC_BASE_ADDR + 0x14)

enum
{
    FLD_MCU_REBOOT = TLK_BIT(7),
};

#define reg_busclk_ratio REG_ADDR8(SC_BASE_ADDR + 0x18)

enum
{
    FLD_BUSCLK_RATIO = TLK_BIT_RNG(0, 2),
};

#define reg_probe_clk_sel REG_ADDR8(SC_BASE_ADDR + 0x1a)

enum
{
    FLD_PROBE_CLK_SEL = TLK_BIT_RNG(0, 4),
};

/*******************************      reset registers: SC_BASE_ADDR+0x20      ******************************/
#define reg_rst  REG_ADDR32(SC_BASE_ADDR + 0x20)

#define reg_rst0 REG_ADDR8(SC_BASE_ADDR + 0x20)

enum
{
    FLD_RST0_I2C   = TLK_BIT(1),
    FLD_RST0_UART0 = TLK_BIT(2),
    FLD_RST0_USB   = TLK_BIT(3),
    FLD_RST0_PWM   = TLK_BIT(4),
    /* RSVD */
    FLD_RST0_UART1 = TLK_BIT(6),
    FLD_RST0_SWIRE = TLK_BIT(7),
};

#define reg_rst1 REG_ADDR8(SC_BASE_ADDR + 0x21)

enum
{
    FLD_RST1_SYS_STIMER = TLK_BIT(1),
    FLD_RST1_DMA        = TLK_BIT(2),
    FLD_RST1_ALGM       = TLK_BIT(3),
    FLD_RST1_PKE        = TLK_BIT(4),
    /* RSVD */
    FLD_RST1_GSPI   = TLK_BIT(6),
    FLD_RST1_SPISLV = TLK_BIT(7),
};

#define reg_rst2 REG_ADDR8(SC_BASE_ADDR + 0x22)

enum
{
    FLD_RST2_TIMER           = TLK_BIT(0),
    FLD_RST2_AUDIO           = TLK_BIT(1),
    FLD_RST2_I2C1            = TLK_BIT(2),
    FLD_RST2_REST_MCU_DIS    = TLK_BIT(3),
    FLD_RST2_MCU_REST_ENABLE = TLK_BIT(4),
    FLD_RST2_LM              = TLK_BIT(5),
    FLD_RST2_TRNG            = TLK_BIT(6),
};

#define reg_rst3 REG_ADDR8(SC_BASE_ADDR + 0x23)

enum
{
    FLD_RST3_TRACE = TLK_BIT(1),
    FLD_RST3_BROM  = TLK_BIT(2),
    /* RSVD */
    FLD_RST3_MSPI   = TLK_BIT(4),
    FLD_RST3_QDEC   = TLK_BIT(5),
    FLD_RST3_SARADC = TLK_BIT(6),
    FLD_RST3_ALG    = TLK_BIT(7),
};

#define reg_rst_1 REG_ADDR32(SC_BASE_ADDR + 0x40)
#define reg_rst4  REG_ADDR8(SC_BASE_ADDR + 0x40)

enum
{
    FLD_RST4_SKE  = TLK_BIT(4),
    FLD_RST4_HASH = TLK_BIT(5),
    /* RSVD */
    FLD_RST4_ZB = TLK_BIT(7),
};

#define reg_rst5 REG_ADDR8(SC_BASE_ADDR + 0x41)

enum
{
    FLD_RST5_UART2 = TLK_BIT(1),
    /* RSVD */
    FLD_RST5_IR_LEARN = TLK_BIT(4),
    FLD_RST5_KEY_SCAN = TLK_BIT(5),
    FLD_RST5_PEM      = TLK_BIT(6),
};

#define reg_rst6 REG_ADDR8(SC_BASE_ADDR + 0x42)

enum
{
    FLD_RST6_RZ = TLK_BIT(0),
};

#define reg_rst7     REG_ADDR8(SC_BASE_ADDR + 0x43)

#define reg_clk_en_1 REG_ADDR32(SC_BASE_ADDR + 0x44)
#define reg_clk_en4  REG_ADDR8(SC_BASE_ADDR + 0x44)

enum
{
    //RSVD
    FLD_CLK4_SKE_EN  = TLK_BIT(4),
    FLD_CLK4_HASH_EN = TLK_BIT(5),
    FLD_CLK4_CCLK_EN = TLK_BIT(6),
    FLD_CLK4_ZB_EN   = TLK_BIT(7),
};

#define reg_clk_en5 REG_ADDR8(SC_BASE_ADDR + 0x45)

enum
{
    //RSVD
    FLD_CLK5_UART2_EN = TLK_BIT(1),
    //RSVD
    FLD_CLK5_IR_LEARN_EN = TLK_BIT(4),
    FLD_CLK5_KEYSCAN_EN  = TLK_BIT(5),
    FLD_CLK5_PEM_EN      = TLK_BIT(6),
    FLD_CLK5_CHACHA20_EN = TLK_BIT(7),
};

#define reg_clk_en6 REG_ADDR8(SC_BASE_ADDR + 0x46)

enum
{
    FLD_CLK6_RZ_EN = TLK_BIT(0),
};

#define reg_clk_en7 REG_ADDR8(SC_BASE_ADDR + 0x47)

#define reg_clk_en  REG_ADDR32(SC_BASE_ADDR + 0x24)

#define reg_clk_en0 REG_ADDR8(SC_BASE_ADDR + 0x24)

enum
{
    FLD_CLK0_I2C_EN   = TLK_BIT(1),
    FLD_CLK0_UART0_EN = TLK_BIT(2),
    FLD_CLK0_USB_EN   = TLK_BIT(3),
    FLD_CLK0_PWM_EN   = TLK_BIT(4),
    FLD_CLK0_DBGEN    = TLK_BIT(5),
    FLD_CLK0_UART1_EN = TLK_BIT(6),
    FLD_CLK0_SWIRE_EN = TLK_BIT(7),
};

#define reg_clk_en1 REG_ADDR8(SC_BASE_ADDR + 0x25)

enum
{
    FLD_CLK1_SYS_STIMER_EN = TLK_BIT(1),
    FLD_CLK1_DMA_EN        = TLK_BIT(2),
    FLD_CLK1_ALGM_EN       = TLK_BIT(3),
    FLD_CLK1_PKE_EN        = TLK_BIT(4),
    FLD_CLK1_MACHINETIME   = TLK_BIT(5),
    FLD_CLK1_GSPI_EN       = TLK_BIT(6),
    FLD_CLK1_SPISLV_EN     = TLK_BIT(7),
};

#define reg_clk_en2 REG_ADDR8(SC_BASE_ADDR + 0x26)

enum
{
    FLD_CLK2_TIMER_EN = TLK_BIT(0),
    FLD_CLK2_AUDIO_EN = TLK_BIT(1),
    FLD_CLK2_I2C1_EN  = TLK_BIT(2),
    //RSVD
    FLD_CLK2_MCU_EN  = TLK_BIT(4),
    FLD_CLK2_LM_EN   = TLK_BIT(5),
    FLD_CLK2_TRNG_EN = TLK_BIT(6),
};

#define reg_clk_en3 REG_ADDR8(SC_BASE_ADDR + 0x27)

enum
{
    //RSVD
    FLD_CLK3_TRACE_EN  = TLK_BIT(1),
    FLD_CLK3_BROM_EN   = TLK_BIT(2),
    FLD_CLK3_EFUSE     = TLK_BIT(3),
    FLD_CLK3_MSPI_EN   = TLK_BIT(4),
    FLD_CLK3_QDEC_EN   = TLK_BIT(5),
    FLD_CLK3_SARADC_EN = TLK_BIT(6),
    /* RSVD */
};

#define reg_pwdn_en REG_ADDR8(SC_BASE_ADDR + 0x2f)

enum
{
    FLD_SUSPEND_EN_O     = TLK_BIT(0),
    FLD_RAMCRC_CLREN_TGL = TLK_BIT(4),
    FLD_RST_ALL          = TLK_BIT(5),
    FLD_STALL_EN_TRG     = TLK_BIT(7),
};

#define reg_cclk_sel REG_ADDR8(SC_BASE_ADDR + 0x28)

enum
{
    FLD_CLK_SCLK_DIV = TLK_BIT_RNG(0, 3),
    FLD_CLK_SCLK_SEL = TLK_BIT_RNG(4, 5),
};

#define reg_dmic_step REG_ADDR16(SC_BASE_ADDR + 0x2c)

enum
{
    FLD_DMIC_STEP = TLK_BIT_RNG(0, 14),
    FLD_DMIC_SEL  = TLK_BIT(15),
};

#define reg_dmic_mod  REG_ADDR16(SC_BASE_ADDR + 0x36)

#define reg_wakeup_en REG_ADDR8(SC_BASE_ADDR + 0x2e)

enum
{
    FLD_USB_PWDN_I    = TLK_BIT(0),
    FLD_GPIO_WAKEUP_I = TLK_BIT(1),
    FLD_QDEC_RESUME   = TLK_BIT(2),
    FLD_KS_WAKEUP_I   = TLK_BIT(3),
    FLD_USB_RESUME    = TLK_BIT(4),
    FLD_STANDBY_EX    = TLK_BIT(5),
};

#define reg_wakeup_status 0x64

typedef enum
{
    FLD_WKUP_CMP   = TLK_BIT(0),
    FLD_WKUP_TIMER = TLK_BIT(1),
    FLD_WKUP_DIG   = TLK_BIT(2),
    FLD_WKUP_PAD   = TLK_BIT(3),
    // FLD_WKUP_MDEC                       = TLK_BIT(4),
    // FLD_MDEC_RSVD                       = TLK_BIT_RNG(5,7),
} wakeup_status_e;

#define reg_clk_div REG_ADDR8(SC_BASE_ADDR + 0x30)

enum
{
    FLD_S7816_MOD    = TLK_BIT_RNG(4, 6),
    FLD_S7816_CLK_EN = TLK_BIT(7),
};

#define reg_ram_crc REG_ADDR8(SC_BASE_ADDR + 0x32)

enum
{
    FLD_RAM_CRC_ERR          = TLK_BIT(1),
    FLD_RST_ALL_STATUS       = TLK_BIT(4),
    FLD_WATCH_DOG_RST_STATUS = TLK_BIT(5),
    FLD_RST_MCU_STATUS       = TLK_BIT(6),
};

#define reg_sel_jtag REG_ADDR8(SC_BASE_ADDR + 0x33)

enum
{
    FLD_JTAG_SEL = TLK_BIT(0),
};

#define reg_usb_div REG_ADDR8(SC_BASE_ADDR + 0x3b)

enum
{
    FLD_USB_DIV = TLK_BIT_RNG(0, 2),
};

#define reg_boot_idcode(i) REG_ADDR8(SC_BASE_ADDR + 0x5c + (i)) /* i[0-8] */

/**
 * this register is to configure RF related reset.
*/
#define reg_n22_rst  REG_ADDR16(SC_BB_BASE_ADDR + 0x18)
#define reg_n22_rst0 REG_ADDR8(SC_BB_BASE_ADDR + 0x18)

enum
{
    FLD_RST0_ZB     = TLK_BIT(2), // Clears IRQ status and some internal states of the link layer.
    FLD_RST0_ZB_PON = TLK_BIT(6), // Restores all RF-related registers to their default value.
                              // RF related registers include: baseband, linklayer, modem, radio, bb_dma, bb_timer, pdzb.
    FLD_RST0_DMA_BB = TLK_BIT(7), // Resets the BB DMA status, and related registers will be cleared. After configuration, BB DMA needs to be reconfigured.
};

#define reg_n22_rst1 REG_ADDR8(SC_BB_BASE_ADDR + 0x19)

enum
{
    FLD_RST1_RSTL_BB     = TLK_BIT(2), // Clears RF state machine and some internal states of the link layer.
    FLD_RST1_RST_MDM     = TLK_BIT(3), // Clears all digital logic states related to mdm, the related configuration will not be lost after reset.
    FLD_RST1_RSTL_STIMER = TLK_BIT(4), // Resets BB STIMER status. RW registers remain, read-only registers will be cleared, and BB timer tick will be restarted.
};

#define reg_n22_clk_en  REG_ADDR16(SC_BB_BASE_ADDR + 0x1a)
#define reg_n22_clk_en0 REG_ADDR8(SC_BB_BASE_ADDR + 0x1a)

enum
{
    FLD_CLK0_ZB_HCLK_EN = TLK_BIT(2),
    FLD_CLK0_DMA_BB_EN  = TLK_BIT(7),
};

#define reg_n22_clk_en1 REG_ADDR8(SC_BB_BASE_ADDR + 0x1b)

enum
{
    FLD_CLK1_CLK_BB      = TLK_BIT(3),
    FLD_CLK1_CLKZB32K_LP = TLK_BIT(4),
};
