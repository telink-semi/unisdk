/********************************************************************************************************
 * @file    common.c
 *
 * @brief   This is the source file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2023
 *
 * @par     Copyright (c) 2023, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "tlk_platform.h"
#include "core/include/tlk_analog.h"
#include "common/include/tlk_init.h"
// #include "registers/tlk_gpio_reg.h"
#include "registers/tlk_analog_afe3v_reg.h"

// void gpio_shutdown_all(void)
// {
//     //disable input
//     reg_gpio_pa_ie = 0x80; //except SWS
//     reg_gpio_pb_ie &= 0xf0;
//     tlk_analog_write_reg8(areg_gpio_pb_ie, (tlk_analog_read_reg8(areg_gpio_pb_ie) & 0x0f));
//     tlk_analog_write_reg8(areg_gpio_pc_ie, 0);
//     reg_gpio_pd_ie = 0x00;
//     reg_gpio_pe_ie = 0x00;

//     //output disable
//     reg_gpio_pa_oen = 0xff;
//     reg_gpio_pb_oen = 0xff;
//     reg_gpio_pc_oen = 0xff;
//     reg_gpio_pd_oen = 0xff;
//     reg_gpio_pe_oen = 0xff;

//     //as gpio
//     reg_gpio_pa_gpio = 0x7f; //except SWS
//     reg_gpio_pb_gpio = 0xff;
//     reg_gpio_pc_gpio = 0xff;
//     reg_gpio_pd_gpio = 0xff;
//     reg_gpio_pe_gpio = 0xff;
// }

// void gpio_set_up_down_res_sws(void)
// {
//     enum tlk_gpio_port sws_port = TLK_GPIO_PORT_A;
//     enum tlk_gpio_pin sws_pin = TLK_GPIO_PIN_7;
//     unsigned char up_down_res = 1;
//     unsigned char r_val = up_down_res & 0x03;

//     unsigned char base_ana_reg = 0;

//     base_ana_reg = 0x17 + (sws_port << 1) + ((sws_pin & 0xf0) ? 1 : 0);

//     unsigned char shift_num, mask_not;

//     if (sws_pin & 0x11) {
//         shift_num = 0;
//         mask_not  = 0xfc;
//     } else if (sws_pin & 0x22) {
//         shift_num = 2;
//         mask_not  = 0xf3;
//     } else if (sws_pin & 0x44) {
//         shift_num = 4;
//         mask_not  = 0xcf;
//     } else if (sws_pin & 0x88) {
//         shift_num = 6;
//         mask_not  = 0x3f;
//     } else {
//         return;
//     }
//     tlk_analog_write_reg8(base_ana_reg, (tlk_analog_read_reg8(base_ana_reg) & mask_not) | (r_val << shift_num));
// }

void tlk_platform_init(enum tlk_power_mode power_mode, enum tlk_power_vbat vbat_v, enum tlk_power_cap cap, unsigned char flash_protect_en)
{
    (void)flash_protect_en;
/**
    ===============================================================================
                         ##### tlk_sys_init #####
    ===============================================================================
*/
    tlk_sys_init(power_mode, vbat_v, 0, cap);

/**
    ===============================================================================
                         ##### update system status #####
    ===============================================================================
    After each initialization, you need to update the system status and set it to a fixed value.
    Otherwise, the next judgment may be inaccurate because the corresponding value is not configured.
    ===============================================================================
*/
    tlk_sys_update_reboot_reason(1);

    /**
    ===============================================================================
                         ##### set SWS pull #####
    ===============================================================================
    If SWS is not connected to the analog pull up and down,it is in a suspended state and may be in a level indeterminate state,
    which will affect the electric leakage of digital and lead to higher current than normal during suspend sleep.
    there may be the risk of SWS miswriting the chip registers or sram causing a crash.
    ===============================================================================
*/
    // gpio_set_up_down_res_sws();
}

__attribute__((used, noinline))
void tlk_platform_init_(void) {
    TLK_PLATFORM_INIT;
    // TLK_CLOCK_INIT;
}

TLK_REGISTER_PRE_INIT(tlk_platform_init_, TLK_INIT_LEVEL_SYSTEM, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_AFTER_SLEEP(tlk_platform_init_, TLK_INIT_LEVEL_SYSTEM, TLK_INIT_PRIORITY_NORMAL)
