/********************************************************************************************************
 * @file    main.c
 *
 * @brief   This is the source file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2019
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include <string.h>

#include "api/include/tlk_sleep.h"
#include "ble.h"
#include "common/include/tlk_utils.h"
#include "common/leh.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_sys.h"
#include "system/debug/log/tlk_log.h"
#include "tlk_ble_host_sal.h"

#include <stdlib.h>
#include <string.h>

TLK_LOG_CREATE(ble_host_v2_demo, "BLE_HOST_V2_DEMO");

#define APP_BLE_ACL_PERIPHERAL_MAX_COUNT 4
#define BLE_PERIPHERAL_DEVICE_NAME       "tlk_bluetooth_ble_per"
#define BLE_CONN_HANDLE_INVALID          0xFFFF

static struct app_acl_peripheral_info
{
    uint8_t count;
    struct
    {
        uint16_t conn_handle;
    } peripheral_info[APP_BLE_ACL_PERIPHERAL_MAX_COUNT];
} s_app_acl_peripheral_info;

static const struct ad_data_flags s_adv_flags = {
    .header.length                      = 0x02,
    .header.type                        = DT_FLAGS,
    .flags.le_limited_discoverable_mode = 1,
    .flags.br_edr_not_supported         = 1,
};

static const struct ad_data_complete_local_name_complete s_adv_complete_name = {
    .header.length = sizeof(BLE_PERIPHERAL_DEVICE_NAME),
    .header.type   = DT_COMPLETE_LOCAL_NAME,
    .name          = BLE_PERIPHERAL_DEVICE_NAME,
};

static const struct ltv_data* adv_data[] = {
    (const struct ltv_data*) &s_adv_flags,
    (const struct ltv_data*) &s_adv_complete_name,
    NULL, // end of advertisement data
};
static const struct ltv_data* scan_rsp_data[] = {
    (const struct ltv_data*) &s_adv_complete_name,
    NULL, // end of advertisement data
};

static void app_connected_callback(struct ble_host_conn* conn);
static void app_disconnected_callback(struct ble_host_conn* conn, uint8_t reason);

static const struct ble_host_acl_conn_callbacks s_app_acl_callbacks = {
    .connected    = app_connected_callback,
    .disconnected = app_disconnected_callback,
};

static void app_connected_callback(struct ble_host_conn* conn)
{
    TLK_LOG_INFO(
        ble_host_v2_demo, "[APP] connected callback, ACL handle: 0x%03x", conn->conn_handle);
    if (conn->role == BLE_HOST_ACL_ROLE_PERIPHERAL)
    {
        for (int i = 0; i < APP_BLE_ACL_PERIPHERAL_MAX_COUNT; i++)
        {
            if (s_app_acl_peripheral_info.peripheral_info[i].conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                s_app_acl_peripheral_info.peripheral_info[i].conn_handle = conn->conn_handle;
                s_app_acl_peripheral_info.count++;
                break;
            }
        }
    }

    if (s_app_acl_peripheral_info.count < APP_BLE_ACL_PERIPHERAL_MAX_COUNT)
    {
        ble_host_gap_adv_start();
    }
}

static void app_disconnected_callback(struct ble_host_conn* conn, uint8_t reason)
{
    TLK_LOG_INFO(ble_host_v2_demo,
                 "[APP] disconnected callback, ACL handle: 0x%03x, reason: 0x%02x",
                 conn->conn_handle,
                 reason);
    if (conn->role == BLE_HOST_ACL_ROLE_PERIPHERAL)
    {
        for (int i = 0; i < APP_BLE_ACL_PERIPHERAL_MAX_COUNT; i++)
        {
            if (s_app_acl_peripheral_info.peripheral_info[i].conn_handle == conn->conn_handle)
            {
                s_app_acl_peripheral_info.peripheral_info[i].conn_handle = BLE_CONN_HANDLE_INVALID;
                s_app_acl_peripheral_info.count--;
                break;
            }
        }
    }
    ble_host_gap_adv_start();
}

int ble_host_v2_peripheral_init(void)
{
    ble_host_gap_adv_set_adv_ind_param(150, adv_data, scan_rsp_data);

    blc_svc_addCoreGroup();
    blc_svc_addDisGroup();
    blc_svc_calculateDatabaseHash();
    blc_svc_setDeviceName(BLE_PERIPHERAL_DEVICE_NAME);

    s_app_acl_peripheral_info.count = 0;
    for (int i = 0; i < APP_BLE_ACL_PERIPHERAL_MAX_COUNT; i++)
    {
        s_app_acl_peripheral_info.peripheral_info[i].conn_handle = BLE_CONN_HANDLE_INVALID;
    }

    ble_host_acl_conn_register_user_data(BLE_HOST_APP_DATA1_USER_ID, &s_app_acl_callbacks);
    ble_host_smp_initial(BLE_HOST_SMP_LEGACY_JUST_WORKS_INIT_PARAMS);
    ble_host_smp_store_init(4, 0);
    TLK_LOG_INFO(ble_host_v2_demo, "[APP] BLE ACL peripheral initialized");
    return 0;
}

void ble_host_v2_peripheral_start(void)
{
    ble_host_gap_adv_start();
}

int main(void)
{
    TLK_LOG_INFO(ble_host_v2_demo, "BLE HOST v2 starting...");

    ble_host_v1_init(NULL, NULL);
    tlk_core_interrupt_enable();

    ble_host_v2_peripheral_init();
    ble_host_v2_peripheral_start();

    // Main loop
    while (1)
    {
        tlk_ble_host_loop();
    }

    return 0;
}
