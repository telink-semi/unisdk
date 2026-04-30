/********************************************************************************************************
 * @file    tlk_otp_reg.h
 *
 * @brief   This is the header file for TL721X
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

// TODO: Integrate

#ifndef OTP_REG_H_
#define OTP_REG_H_
#include "tlk_soc.h"

/******************************* otp registers: 0x80140300 ******************************/

#define OTP_BASE_ADDR 0x80140300

#define reg_otp_ctrl0 REG_ADDR8(OTP_BASE_ADDR + 0x0)

enum
{
    FLD_OTP_PCE     = TLK_BIT(1),
    FLD_OTP_PPROG   = TLK_BIT(2),
    FLD_OTP_PWE     = TLK_BIT(3),
    FLD_OTP_PAS     = TLK_BIT(4),
    FLD_OTP_PTR     = TLK_BIT(5),
    FLD_OTP_PTC     = TLK_BIT(6),
    FLD_OTP_ECC_RDB = TLK_BIT(7),
};

#define reg_otp_ctrl1 REG_ADDR8(OTP_BASE_ADDR + 0x01)

enum
{
    FLD_OTP_PTM   = TLK_BIT_RNG(0, 3),
    FLD_OTP_PLDO  = TLK_BIT(4),
    FLD_OTP_PDSTD = TLK_BIT(5),
};

#define reg_otp_ctrl2 REG_ADDR8(OTP_BASE_ADDR + 0x02)

enum
{
    FLD_OTP_DATA_ECC = TLK_BIT_RNG(0, 5),
};

#define reg_otp_ctrl3 REG_ADDR8(OTP_BASE_ADDR + 0x03)

enum
{
    FLD_OTP_AUTO_INC = TLK_BIT(4), /**< 1:when the latch is finished PDOUT data, PA automatically adds 4 */
    FLD_OTP_MAN_MODE = TLK_BIT(5), /**< 1:manual mode */
    FLD_OTP_MAN_PCLK = TLK_BIT(6), /**< control PCLK in manual mode */
};

#define reg_otp_pa   REG_ADDR16(OTP_BASE_ADDR + 0x04)

#define reg_otp_paio REG_ADDR8(OTP_BASE_ADDR + 0x06)

enum
{
    FLD_OTP_PAIO = TLK_BIT_RNG(0, 5),
};

#define reg_otp_status REG_ADDR8(OTP_BASE_ADDR + 0x07)

enum
{
    FLD_OTP_BUSY           = TLK_BIT(0), /**< 1:read busy or auto_load_busy */
    FLD_OTP_AUTO_PWUP_TRIG = TLK_BIT(1), /**< w:trigger auto power up otp */
    FLD_OTP_KEYLOCK        = TLK_BIT(2), /**< Can only write 1 from 0. After setting 1, OTP specific address cannot be read */
};

#define reg_otp_wr_dat REG_ADDR32(OTP_BASE_ADDR + 0x08)

#define reg_otp_rd_dat REG_ADDR32(OTP_BASE_ADDR + 0x0c)

#define reg_otp_ctrl4  REG_ADDR8(OTP_BASE_ADDR + 0x10)

enum
{
    FLD_OTP_CAP_EDGE = TLK_BIT_RNG(0, 4), /**< Indicates how many PCLK edges to latch PDOUT data after the PCLK rising edge. */
};

#define reg_otp_ctrl5 REG_ADDR8(OTP_BASE_ADDR + 0x11)

enum
{
    FLD_OTP_TIM_CFG = TLK_BIT_RNG(0, 2),
};

#define reg_otp_auto_load_data0   REG_ADDR32(OTP_BASE_ADDR + 0x14)
#define reg_otp_auto_load_data1   REG_ADDR32(OTP_BASE_ADDR + 0x18)
#define reg_otp_auto_load_data2   REG_ADDR32(OTP_BASE_ADDR + 0x1c)
#define reg_otp_auto_load_data(i) REG_ADDR32(OTP_BASE_ADDR + 0x14 + (((i) & 0x03) << 2)) /* i[0-2] */

#endif                                                                                   /* OTP_REG_H_ */
