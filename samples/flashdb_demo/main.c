/********************************************************************************************************
 * @file    main.c
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

#include "core/include/tlk_core.h"
#include "flashdb.h"
#include "properties/tlk_chip.h"
#include "storage.h"
#include "storage_device.h"
#include "system/debug/log/tlk_log.h"

static uint8_t         boot_count = 0;
static struct fdb_kvdb kvdb       = {0};

static struct fdb_default_kv_node default_kv_table[] = {
    {"boot_count", &boot_count, sizeof(boot_count)},
};

TLK_LOG_CREATE(flashdb_demo, "FLASHDB_DEMO");

static void update_boot_count(void)
{
    struct fdb_blob blob;
    uint8_t         boot_c = 0;

    fdb_kv_get_blob(&kvdb, "boot_count", fdb_blob_make(&blob, &boot_c, sizeof(boot_c)));
    if (blob.saved.len > 0)
    {
        TLK_LOG_INFO(flashdb_demo, "Boot count: 0x%02x", boot_c);
    }
    else
    {
        TLK_LOG_INFO(flashdb_demo, "Failed to get boot count");
    }

    boot_c++;

    fdb_kv_set_blob(&kvdb, "boot_count", fdb_blob_make(&blob, &boot_c, sizeof(boot_c)));
    TLK_LOG_INFO(flashdb_demo, "set the 'boot_count' value to 0x%02X", boot_c);
}

int main(void)
{
    struct fdb_default_kv default_kv;
    fdb_err_t             result;

    tlk_core_interrupt_enable();

    TLK_LOG_INFO(flashdb_demo, "FlashDB demo start...");

    default_kv.kvs = default_kv_table;
    default_kv.num = sizeof(default_kv_table) / sizeof(default_kv_table[0]);

    result = fdb_kvdb_init(&kvdb, "env", "fdb_kvdb1", &default_kv, NULL);
    if (result != FDB_NO_ERR)
    {
        TLK_LOG_INFO(flashdb_demo, "Failed to initialize flash db: %d", result);
        return -1;
    }

    update_boot_count();

    while (1)
    {
    }

    return 0;
}
