/********************************************************************************************************
 * @file    common.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
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
#pragma once

#include "core/include/tlk_sys.h"

/**
    ===============================================================================
                         ##### platform init and clock init #####
    ===============================================================================
    flash_protect_en assignment description, the default is 1, use the following rules:
    1. By default, the application needs to enable write protection in all cases, this parameter is reserved for some internal special test cases;
    2. Some demos have the operation of writing flash, remember to remove the write protection before erasing, and add the write protection after erasing;
    3. There are some demos that do not need to add write protection, such as dut-related, internal testing;

    @note if flash protection fails, LED1 lights up long, and keeps while.
    ===============================================================================
*/

void tlk_platform_init(enum tlk_power_mode power_mode, enum tlk_power_vbat vbat_v, enum tlk_power_cap cap, unsigned char flash_protect_en);
    #ifndef TLK_PLATFORM_INIT
        #define TLK_PLATFORM_INIT tlk_platform_init(TLK_POWER_LDO_LDO_MODE, TLK_POWER_VBAT_MAX_VALUE_GREATER_THAN_3V6, TLK_POWER_INTERNAL_CAP_XTAL24M, 1)
    #endif

