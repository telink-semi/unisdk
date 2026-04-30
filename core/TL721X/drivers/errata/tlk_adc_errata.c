/********************************************************************************************************
 * @file    tlk_adc.c
 *
 * @brief   This is the source file for TL721X
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

#include "core/include/tlk_clock.h"
#include "tlk_core.h"
#include "registers/tlk_soc.h"
#include "registers/tlk_analog_reg.h"
#include "registers/tlk_adc_reg.h"

typedef enum
{
    TLK_ADC_M_CHANNEL,
    TLK_ADC_L_CHANNEL,
    TLK_ADC_R_CHANNEL,
} tlk_adc_sample_chn_e;

typedef enum
{
    TLK_ADC_VBAT_DIV_OFF = 0,
    TLK_ADC_VBAT_DIV_1F4 = 0x01,
    TLK_ADC_VBAT_DIV_1F2 = 0x03,
} tlk_adc_vbat_div_e;

typedef enum
{
    TLK_ADC_VREF_1P2V = 0x01,
    TLK_ADC_VREF_ANTI_AGING = 0x03,
} tlk_adc_ref_vol_e;

/** 
 * @brief   This function is designed to prevent the aging of the ADC reference voltage during deep sleep.
 * @return  none
 * @note    The reference voltage value of ADC Vref decreases with time, which can be avoided by calling this interface.
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_adc_anti_aging_mode_for_asm(void)
{

    //The following logic is equivalent to tlk_adc_reset()
    reg_rst3 &= (~FLD_RST3_SARADC);
    reg_rst3 |= FLD_RST3_SARADC;
    //The following logic is equivalent to tlk_adc_clk_en()
    reg_clk_en3 |= FLD_CLK3_SARADC_EN;

    if (read_reg8(0x14083d) == TLK_CHIP_VERSION_A1) 
    {
        //Since this interface is called within the .s file and requires the configuration of analog registers, it is necessary to enable the analog clock first.
        reg_rst1 |= FLD_RST1_ALGM;
        reg_clk_en1 |= FLD_CLK1_ALGM_EN,
        /*
        * Turn on xtal_24M clock to analog (includes stimer), this should setup before power up PLL and call tlk_pm_wait_xtal_ready.
        * Because the stimer is necessary for the tlk_pm_wait_xtal_ready.
        * ADC operation depends on the 24M_xtl in sc_clk. In sc_clk, the 24M_xtl source requires two switches to be enabled in sequence.
        * The first level enables the 24M_xtl source, and the second level is controlled by bit 0x8c[1] in reg_ana.
        * (add by bolong, confirmed by ya.yan,qiangkai20240513)
        */
        //The following logic is equivalent to     tlk_analog_write_reg8(areg_0x8c, 0x82)

        reg_ana_len     = 1;
        reg_ana_addr    = 0x8c;
        reg_ana_data(0) = 0x82;
        while (!(reg_ana_buf_cnt & FLD_ANA_TX_BUFCNT))
            ;
        reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
        while (reg_ana_ctrl & FLD_ANA_BUSY)
            ;
        reg_ana_ctrl = 0x00;
    }

    //The following logic is equivalent to tlk_adc_set_scan_chn_cnt(1)
    reg_adc_config0 = ((reg_adc_config0 & (~FLD_SCANT_MAX)) | ((1 * 2) << 4)); //scan_cnt = chn_cnt*2

    //The following logic is equivalent to a tlk_adc_set_clk(FLD_SAR_ADC_CLK_DIV)
    reg_adc_config1 = ((reg_adc_config1 & FLD_SAR_ADC_CLK_DIV) | 1); //div=1, adc digital clk = 24MHz/div.(crystal = 24MHz)

    //The following logic is equivalent to a reg_adc_capture_state(TLK_ADC_M_CHANNEL)
    reg_adc_config2 |= TLK_BIT(TLK_ADC_M_CHANNEL);

    //Anti-aging configuration
    reg_adc_vbat_div = (reg_adc_vbat_div & (~TLK_BIT_RNG(0, 1))) | ((TLK_ADC_VBAT_DIV_1F4));  
    reg_adc_channel_set_state(TLK_ADC_M_CHANNEL) = (reg_adc_channel_set_state(TLK_ADC_M_CHANNEL) & (~FLD_SEL_VREF)) | (TLK_ADC_VREF_ANTI_AGING << 6);

    reg_adc_capture_state(TLK_ADC_M_CHANNEL)     = 1;
    reg_adc_rng_capture_state = 0x01;
    
    //The following logic is equivalent to a tlk_adc_dig_clk_en();
    reg_adc_config2 |= FLD_CLK_EN;

    //The following logic is equivalent to a delay_us(1)
    unsigned long long start = tlk_clock_cclk_get_tick();
    while (tlk_clock_cclk_get_tick() - start < (unsigned long long)(1 * 24)) {
    }

    //Restore default
    reg_adc_config2 = 0x10; 

}
