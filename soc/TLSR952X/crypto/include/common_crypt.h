/********************************************************************************************************
 * @file    common_crypt.h
 *
 * @brief   This is the header file for B92
 *
 * @author  Driver Group
 * @date    2026
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

#ifndef _COMMON_CRYPT_H_
#define _COMMON_CRYPT_H_

typedef enum
{
    DRV_API_SUCCESS       = 0x00,
    DRV_API_FAILURE       = 0x01,
    DRV_API_TIMEOUT       = 0x02,
    DRV_API_INVALID_PARAM = 0x03
} drv_api_status_e;

#endif
