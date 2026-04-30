/********************************************************************************************************
 * @file    soc.h
 *
 * @brief   This is the header file for B92
 *
 * @author  Driver Group
 * @date    2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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

#define TCMD_UNDER_BOTH       0xc0
#define TCMD_UNDER_RD         0x80
#define TCMD_UNDER_WR         0x40

#define TCMD_MASK             0x3f

#define TCMD_WRITE            0x3
#define TCMD_WAIT             0x7
#define TCMD_WAREG            0x8
//#if 1 //optimize
/*
 * IRAM area:0x00000~0x3FFFF TLK_BIT(19) is 0,TLK_BIT(17~0) 256K is address offset
 * DRAM area:0x80000~0xBFFFF TLK_BIT(19) is 1,TLK_BIT(17~0) 256K is address offset
 * ILM area:0xc0000000~0xc0040000 TLK_BIT(31~30) is 3,TLK_BIT(21) is 0, TLK_BIT(20~18) do not care  TLK_BIT(17~0) 256K is address offset 256K is address offset
 * DLM area:0xc0200000~0xc0240000 TLK_BIT(31~30) is 3,TLK_BIT(21) is 1, TLK_BIT(20~18) do not care  TLK_BIT(17~0) 256K is address offset 256K is address offset
 * TLK_BIT(19) is used to distinguish from IRAM to DRAM, TLK_BIT(21) is used to distinguish from ILM to DLM.
 * so we can write it as follow
 * #define  convert_ram_addr_cpu2bus  (((((addr))&0x80000)? ((addr)| 0xc0200000) : ((addr)|0xc0000000)))
 * TLK_BIT(20~17) are invalid address line ,IRAM address is less than 0x80000, (address-0x80000)must borrow from TLK_BIT(21)
 *   #define convert(addr) ((addr)-0x80000+0xc0200000)
 *  to simplify
 *  #define convert(addr) ((addr)+0xc0180000)
 * */
//#define convert_ram_addr_cpu2bus(addr)   ((unsigned int)(addr)+0xc0180000)
//#else  //no optimize
//#define  convert_ram_addr_cpu2bus  (((((unsigned int)(addr)) >=0x80000)?(((unsigned int)(addr))-0x80000+0xc0200000) : (((unsigned int)(addr)) + 0xc0000000)))
//#endif
// go further, if the source and destination  address is not in the sram(IRAM/DRAM)  interval, no address translation
#define convert_ram_addr_cpu2bus(addr) (unsigned int)(addr) < 0xc0000 ? ((unsigned int)(addr) + 0xc0180000) : (unsigned int)(addr)
//When using convert_ram_addr_cpu2bus(addr) formula to write to the register, TLK_BIT(20~18) may not be zero,  need to clear the corresponding bit to zero
#define convert_ram_addr_bus2cpu(addr) (((((unsigned int)(addr)) >= 0xc0200000) ? (((unsigned int)(addr)) + 0x80000 - 0xc0200000) : (((unsigned int)(addr) & (~0x1c0000)) - 0xc0000000)))


#define LM_BASE                        0x80000000

#define ILM_BASE                       (LM_BASE + 0x40000000)
#define DLM_BASE                       (LM_BASE + 0x40200000)
#define CPU_ILM_BASE                   (0x00000000)
#define CPU_DLM_BASE                   (0x00080000)

/*******************************       sc registers: 0x1401c0      ******************************/
#define SC_BASE_ADDR     0x1401c0

#define reg_mspi_clk_set REG_ADDR8(SC_BASE_ADDR + 0x00)

enum
{
    FLD_MSPI_CLK_MOD    = TLK_BIT_RNG(0, 5),
    FLD_MSPI_DIV_IN_SEL = TLK_BIT(6), //0:rc24m   1:hs_mux
    FLD_MSPI_DIV_RSTN   = TLK_BIT(7), //0:rset clkdiv  1:release
};

#define reg_lspi_clk_set REG_ADDR8(SC_BASE_ADDR + 0x01)

enum
{
    FLD_LSPI_CLK_MOD    = TLK_BIT_RNG(0, 5),
    FLD_LSPI_DIV_IN_SEL = TLK_BIT(6), //0:rc24m   1:hs_mux
    FLD_LSPI_DIV_RSTN   = TLK_BIT(7), //0:rset clkdiv  1:release
};

#define reg_gspi_clk_set REG_ADDR16(SC_BASE_ADDR + 0x2)

enum
{
    FLD_GSPI_CLK_MOD    = TLK_BIT_RNG(0, 7),
    FLD_GSPI_DIV_IN_SEL = TLK_BIT(14), //0:rc24m   1:hs_mux
    FLD_GSPI_DIV_RSTN   = TLK_BIT(15), //0:rset clkdiv  1:release
};

#define reg_efuse_addr REG_ADDR8(SC_BASE_ADDR + 0x08)
#define reg_efuse_rdat REG_ADDR8(SC_BASE_ADDR + 0x09)
#define reg_efuse_wdat REG_ADDR8(SC_BASE_ADDR + 0x0a)
#define reg_efuse_ctrl REG_ADDR8(SC_BASE_ADDR + 0x0b)

enum
{
    FLD_EFUSE_WREN    = TLK_BIT(0),
    FLD_EFUSE_RDEN    = TLK_BIT(1),
    FLD_EFUSE_CLKEN   = TLK_BIT(2),
    FLD_EFUSE_WR_TRIG = TLK_BIT(3),
    //RSVD
    //RSVD
    //RSVD
    FLD_EFUSE_BUSY = TLK_BIT(7),
};

#define reg_efuse_b0      REG_ADDR32(SC_BASE_ADDR + 0x0c)
#define reg_efuse_read_en REG_ADDR8(SC_BASE_ADDR + 0x34)

/*******************************      reset registers: 1401e0      ******************************/
#define reg_rst  REG_ADDR32(0x1401e0)

#define reg_rst0 REG_ADDR8(0x1401e0)

enum
{
    FLD_RST0_LSPI  = TLK_BIT(0),
    FLD_RST0_I2C   = TLK_BIT(1),
    FLD_RST0_UART0 = TLK_BIT(2),
    FLD_RST0_USB   = TLK_BIT(3),
    FLD_RST0_PWM   = TLK_BIT(4),
    //RSVD
    FLD_RST0_UART1 = TLK_BIT(6),
    FLD_RST0_SWIRE = TLK_BIT(7),
};

#define reg_rst1 REG_ADDR8(0x1401e1)

enum
{
    FLD_RST1_CHG        = TLK_BIT(0),
    FLD_RST1_SYS_STIMER = TLK_BIT(1),
    FLD_RST1_DMA        = TLK_BIT(2),
    FLD_RST1_ALGM       = TLK_BIT(3),
    FLD_RST1_PKE        = TLK_BIT(4),
    //RSVD
    FLD_RST1_GSPI   = TLK_BIT(6),
    FLD_RST1_SPISLV = TLK_BIT(7),
};

#define reg_rst2 REG_ADDR8(0x1401e2)

enum
{
    FLD_RST2_TIMER = TLK_BIT(0),
    FLD_RST2_AUD   = TLK_BIT(1),
    FLD_RST2_I2C1  = TLK_BIT(2),
    //RSVD
    FLD_RST2_MCU  = TLK_BIT(4),
    FLD_RST2_LM   = TLK_BIT(5),
    FLD_RST2_TRNG = TLK_BIT(6),
    FLD_RST2_DPR  = TLK_BIT(7),
};

#define reg_rst3 REG_ADDR8(0x1401e3)

enum
{
    FLD_RST3_ZB       = TLK_BIT(0),
    FLD_RST3_MSTCLK   = TLK_BIT(1),
    FLD_RST3_LPCLK    = TLK_BIT(2),
    FLD_RST3_ZB_CRYPT = TLK_BIT(3),
    FLD_RST3_MSPI     = TLK_BIT(4),
    FLD_RST3_QDEC     = TLK_BIT(5),
    FLD_RST3_SARADC   = TLK_BIT(6),
    FLD_RST3_ALG      = TLK_BIT(7),
};

#define reg_clk_en  REG_ADDR32(0x1401e4)

#define reg_clk_en0 REG_ADDR8(0x1401e4)

enum
{
    FLD_CLK0_LSPI_EN  = TLK_BIT(0),
    FLD_CLK0_I2C_EN   = TLK_BIT(1),
    FLD_CLK0_UART0_EN = TLK_BIT(2),
    FLD_CLK0_USB_EN   = TLK_BIT(3),
    FLD_CLK0_PWM_EN   = TLK_BIT(4),
    FLD_CLK0_DBGEN_EN = TLK_BIT(5),
    FLD_CLK0_UART1_EN = TLK_BIT(6),
    FLD_CLK0_SWIRE_EN = TLK_BIT(7),
};

#define reg_clk_en1 REG_ADDR8(0x1401e5)

enum
{
    FLD_CLK1_CHG_EN         = TLK_BIT(0),
    FLD_CLK1_SYS_TIMER_EN   = TLK_BIT(1),
    FLD_CLK1_DMA_EN         = TLK_BIT(2),
    FLD_CLK1_ALGM_EN        = TLK_BIT(3),
    FLD_CLK1_PKE_EN         = TLK_BIT(4),
    FLD_CLK1_MACHINETIME_EN = TLK_BIT(5),
    FLD_CLK1_GSPI_EN        = TLK_BIT(6),
    FLD_CLK1_SPISLV_EN      = TLK_BIT(7),

};

#define reg_clk_en2 REG_ADDR8(0x1401e6)

enum
{
    FLD_CLK2_TIMER_EN = TLK_BIT(0),
    FLD_CLK2_AUD_EN   = TLK_BIT(1),
    FLD_CLK2_I2C1_EN  = TLK_BIT(2),
    //RSVD
    FLD_CLK2_MCU_EN  = TLK_BIT(4),
    FLD_CLK2_LM_EN   = TLK_BIT(5),
    FLD_CLK2_TRNG_EN = TLK_BIT(6),
    FLD_CLK2_DPR_EN  = TLK_BIT(7),
};

#define reg_clk_en3 REG_ADDR8(0x1401e7)

enum
{
    FLD_CLK3_ZB_PCLK_EN   = TLK_BIT(0),
    FLD_CLK3_ZB_MSTCLK_EN = TLK_BIT(1),
    FLD_CLK3_ZB_LPCLK_EN  = TLK_BIT(2),
    //RSVD
    FLD_CLK3_MSPI_EN = TLK_BIT(4),
    FLD_CLK3_QDEC_EN = TLK_BIT(5),
    //RSVD
    //RSVD
};

#define reg_clk_sel0 REG_ADDR8(0x1401e8)

enum
{
    FLD_CLK_SCLK_DIV = TLK_BIT_RNG(0, 3),
    FLD_CLK_SCLK_SEL = TLK_BIT_RNG(4, 6),
};

#define reg_i2s_step(i) REG_ADDR16(SC_BASE_ADDR + 0x06 + ((i) * 0x16)) //i2s0-0x06 , i2s1-0x1c

enum
{
    FLD_I2S_STEP   = TLK_BIT_RNG(0, 14),
    FLD_I2S_CLK_EN = TLK_BIT(15),
};

#define reg_i2s_mod(i) REG_ADDR16(SC_BASE_ADDR + 0x2a - ((i) * 0x0C)) //i2s0-0x2a , i2s1-0x1e


#define reg_dmic_step  REG_ADDR16(SC_BASE_ADDR + 0x2c)

enum
{
    FLD_DMIC_STEP = TLK_BIT_RNG(0, 14),
    FLD_DMIC_SEL  = TLK_BIT(15),
};

#define reg_pwdn_en       REG_ADDR8(SC_BASE_ADDR + 0x2f)

#define reg_dmic_mod      REG_ADDR16(SC_BASE_ADDR + 0x36)

#define reg_probe_clk_sel REG_ADDR8(SC_BASE_ADDR + 0x1a)

#define reg_wakeup_en     REG_ADDR8(SC_BASE_ADDR + 0x2e)

enum
{
    FLD_USB_PWDN_I    = TLK_BIT(0),
    FLD_GPIO_WAKEUP_I = TLK_BIT(1),
    FLD_QDEC_WAKEUP_I = TLK_BIT(2),
    FLD_USB_RESUME    = TLK_BIT(4),
    FLD_STANDBY_EX    = TLK_BIT(5),
};

#define reg_wakeup_status 0x64

typedef enum
{
    FLD_WKUP_PAD   = TLK_BIT(0),
    FLD_WKUP_DIG   = TLK_BIT(1),
    FLD_WKUP_TIMER = TLK_BIT(2),
    FLD_WKUP_CMP   = TLK_BIT(3),
    FLD_WKUP_MDEC  = TLK_BIT(4),
    FLD_WKUP_CTB   = TLK_BIT(5),
    FLD_WKUP_VAD   = TLK_BIT(6),
    FLD_VBUS_ON    = TLK_BIT(7),
} wakeup_status_e;
