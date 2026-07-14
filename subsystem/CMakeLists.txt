# SPDX-License-Identifier: Apache-2.0

# Add BLE Controller subsystem
if(CONFIG_TLK_BLE_CONTROLLER AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ble/controller)
    add_subdirectory(ble/controller)
endif()
