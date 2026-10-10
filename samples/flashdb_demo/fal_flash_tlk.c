/********************************************************************************************************
 * @file    fal_flash_tlk.c
 *
 * @brief   This is the source file for Telink RISC-V MCU
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
#include <fal.h>

#include "storage.h"
#include "storage_device.h"

static int init(void);
static int read(long offset, uint8_t* buf, size_t size);
static int write(long offset, const uint8_t* buf, size_t size);
static int erase(long offset, size_t size);

static struct tlk_storage_device* dev;

struct fal_flash_dev tlk_flash0 = {
    .name       = "tlk_flash0",
    .addr       = 0,
    .len        = 0,
    .blk_size   = 0,
    .ops        = {init, read, write, erase},
    .write_gran = 1,
};

static int init(void)
{
    dev = tlk_storage_get_next_device(NULL);
    if (!dev)
    {
        return -1;
    }

    /* update the flash chip information */
    tlk_flash0.blk_size = dev->page;
    tlk_flash0.len      = dev->size;
    tlk_flash0.addr     = dev->base;

    return 0;
}

static int read(long offset, uint8_t* buf, size_t size)
{
    if (!dev)
    {
        return -1;
    }

    return tlk_storage_read(dev->base + offset, buf, size) ? 0 : (int) size;
}

static int write(long offset, const uint8_t* buf, size_t size)
{
    if (!dev)
    {
        return -1;
    }

    return tlk_storage_write(dev->base + offset, buf, size) ? -1 : (int) size;
}

static int erase(long offset, size_t size)
{
    if (!dev)
    {
        return -1;
    }

    return tlk_storage_erase(dev->base + offset, size) ? -1 : (int) size;
}
