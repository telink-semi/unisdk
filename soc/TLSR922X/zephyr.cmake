# SPDX-License-Identifier: Apache-2.0
#
# UniSDK Zephyr module — TLSR922X SoC drivers
#
# Mirrors soc/TLSR922X/CMakeLists.txt (no crypto subdirectory).
#
# Variables expected from the caller:
#   UNISDK_BASE  - path to the unisdk root directory

# ============================================================================
# 1. Path variables
# ============================================================================
set(SOC_CHIP_BASE ${UNISDK_BASE}/soc/${CONFIG_TLK_SOC_SERIES})

# ============================================================================
# 2. Include paths
# ============================================================================
zephyr_include_directories(
    ${SOC_CHIP_BASE}
    ${SOC_CHIP_BASE}/drivers/include
)

# ============================================================================
# 3. Always-compiled sources (mirroring add_sources outside SDK check)
# ============================================================================
# tlk_mspi.c — always added to TARGET_NAME (line 4)
zephyr_sources(${SOC_CHIP_BASE}/drivers/src/tlk_mspi.c)

# ============================================================================
# 4. Dev-mode sources (mirroring sources inside if (NOT IS_DIRECTORY ...))
# ============================================================================
# _tlk_advanced_stimer.c — added in dev mode only
if(NOT EXISTS "${UNISDK_BASE}/libraries")
    zephyr_sources_ifdef(CONFIG_TLK_SYSTEM_TIMER_ADVANCED_MODE
        ${SOC_CHIP_BASE}/drivers/src/_tlk_advanced_stimer.c)
endif()

# Note: TLSR922X has no crypto subdirectory.
