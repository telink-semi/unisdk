# SPDX-License-Identifier: Apache-2.0

# Add BLE Controller subsystem
if(CONFIG_TLK_BLE_CONTROLLER AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ble/controller)
    add_subdirectory(ble/controller)
endif()

# Add MCUboot subsystem
if(CONFIG_TLK_MCUBOOT AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/mcuboot)
    add_subdirectory(mcuboot)
endif()

# Add BLE host subsystem
if(CONFIG_TLK_BLE_HOST AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ble/host)
    add_subdirectory(ble/host/prj/unisdk)
endif()
