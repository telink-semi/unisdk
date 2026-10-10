/********************************************************************************************************
 * @file    fal_cfg.h.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2026
 *
 * @par     Copyright (c) 2026, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

/* Disable logs */
#define FAL_PRINTF(...)
#define FDB_PRINT(...)

#define TLK_FLASH0_DEV_NAME "tlk_flash0"
/* ===================== Flash device Configuration ========================= */
extern struct fal_flash_dev tlk_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                                                        \
    {                                                                                              \
        &tlk_flash0,                                                                               \
    }
/* ====================== Partition Configuration ========================== */
#define FAL_PART_HAS_TABLE_CFG
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                                             \
    {                                                                                              \
        {FAL_PART_MAGIC_WORD, "bl", TLK_FLASH0_DEV_NAME, 0, 256 * 1024, 0},                        \
        {FAL_PART_MAGIC_WORD, "fdb_kvdb1", TLK_FLASH0_DEV_NAME, 256 * 1024, 704 * 1024, 0},        \
    }
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
