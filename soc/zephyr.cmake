# SPDX-License-Identifier: Apache-2.0
#
# UniSDK Zephyr module — SoC-specific drivers
#
# Delegates to the chip-specific zephyr.cmake, mirroring soc/CMakeLists.txt
# which delegates via add_subdirectory(${CONFIG_TLK_SOC_SERIES}).
#
# Variables expected from the caller (set in zephyr/CMakeLists.txt):
#   UNISDK_BASE  - path to the unisdk root directory

# ============================================================================
# 1. Delegate to chip-specific zephyr.cmake
# ============================================================================
include(${UNISDK_BASE}/soc/${CONFIG_TLK_SOC_SERIES}/zephyr.cmake)
