/********************************************************************************************************
 * @file    tlk_sdk_version.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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

/*
 * It is only used by SDK, and the customer is not allowed to modify it
 */
#if defined(MCU_CORE_B91)
    /*
 * Release Tool need to change this macro to match the release version,
 * the replace rules is: "$$$tl_platform_sdk_"#sdk_version_num"$$$", The "#sdk_version_num"
 * will replace with this macro value.
 */
    #define B91_SDK_VERSION_NUM V3.7.0
    #define SDK_VERSION_NUM     B91_SDK_VERSION_NUM

#elif defined(MCU_CORE_B92)
    /*
 * Release Tool need to change this macro to match the release version,
 * the replace rules is: "$$$tl_platform_sdk_"#sdk_version_num"$$$", The "#sdk_version_num"
 * will replace with this macro value.
 */
    #define B92_SDK_VERSION_NUM V3.7.0
    #define SDK_VERSION_NUM     B92_SDK_VERSION_NUM

#elif defined(MCU_CORE_TL7518)
    /*
 * Release Tool need to change this macro to match the release version,
 * the replace rules is: "$$$tl_platform_sdk_"#sdk_version_num"$$$", The "#sdk_version_num"
 * will replace with this macro value.
 */
    #define TL7518_SDK_VERSION_NUM V3.7.0
    #define SDK_VERSION_NUM        TL7518_SDK_VERSION_NUM
#elif defined(MCU_CORE_TL751X)
    /*
 * Release Tool need to change this macro to match the release version,
 * the replace rules is: "$$$tl_platform_sdk_"#sdk_version_num"$$$", The "#sdk_version_num"
 * will replace with this macro value.
 */
    #define TL751X_SDK_VERSION_NUM V3.7.0
    #define SDK_VERSION_NUM        TL751X_SDK_VERSION_NUM


#elif defined(MCU_CORE_TL721X)
    /*
 * Release Tool need to change this macro to match the release version,
 * the replace rules is: "$$$tl_platform_sdk_"#sdk_version_num"$$$", The "#sdk_version_num"
 * will replace with this macro value.
 */
    #define TL721X_SDK_VERSION_NUM V3.7.0
    #define SDK_VERSION_NUM        TL721X_SDK_VERSION_NUM

#elif defined(MCU_CORE_TL321X)
    /*
 * Release Tool need to change this macro to match the release version,
 * the replace rules is: "$$$tl_platform_sdk_"#sdk_version_num"$$$", The "#sdk_version_num"
 * will replace with this macro value.
 */
    #define TL321X_SDK_VERSION_NUM V3.7.0
    #define SDK_VERSION_NUM        TL321X_SDK_VERSION_NUM

#elif defined(MCU_CORE_TL751X)
    /*
 * Release Tool need to change this macro to match the release version,
 * the replace rules is: "$$$tl_platform_sdk_"#sdk_version_num"$$$", The "#sdk_version_num"
 * will replace with this macro value.
 */
    #define TL751X_SDK_VERSION_NUM V3.7.0
    #define SDK_VERSION_NUM        TL751X_SDK_VERSION_NUM

#elif defined(MCU_CORE_TL322x)
    /*
 * Release Tool need to change this macro to match the release version,
 * the replace rules is: "$$$tl_platform_sdk_"#sdk_version_num"$$$", The "#sdk_version_num"
 * will replace with this macro value.
 */
    #define TL322x_SDK_VERSION_NUM V3.7.0
    #define SDK_VERSION_NUM        TL322x_SDK_VERSION_NUM
#endif

#define SDK_VERSION1(sdk_version_num) "$$$tl_platform_sdk_" #sdk_version_num "$$$"
#define SDK_VERSION(sdk_version_num)  SDK_VERSION1(sdk_version_num)
