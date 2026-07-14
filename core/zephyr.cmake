# SPDX-License-Identifier: Apache-2.0
#
# UniSDK Zephyr module — Core drivers
#
# This file provides Zephyr build integration for the core UniSDK drivers:
# include paths, always-compiled driver sources, conditional driver sources
# (gated by Kconfig), errata sources (GLOB), and critical printout.
#
# Two modes:
#   Dev mode  — compile sources via zephyr_sources()
#   SDK mode  — link prebuilt libtelink-unisdk-<SOC_SERIES>[-(pm|rf)].a
#
# Variables expected from the caller (set in zephyr/CMakeLists.txt):
#   UNISDK_BASE  - path to the unisdk root directory

# ============================================================================
# 1. Path variables — matching core/CMakeLists.txt style
#    CORE_DRV_DIR  = "${CONFIG_TLK_CORE}/drivers"  (same as DRIVERS_PATH)
#    CORE_COMMON   = "common"                       (same as COMMON_PATH)
# ============================================================================
set(CORE_BASE       ${UNISDK_BASE}/core)
set(CORE_DRV_DIR    ${CONFIG_TLK_CORE}/drivers)
set(CORE_COMMON     common)

# ============================================================================
# 2. Include paths — needed in both modes
# ============================================================================
zephyr_include_directories(
    ${CORE_BASE}/include                 # for "tlk_stimer.h", "tlk_pm.h", etc.
    ${CORE_BASE}/${CONFIG_TLK_CORE}      # for "properties/tlk_chip.h" (generated)
)

# Errata include path
zephyr_include_directories(
    ${CORE_BASE}/${CORE_DRV_DIR}/errata
)

# ============================================================================
# 3. Always-compiled sources — matching core/CMakeLists.txt lines 7, 12-29
#    These are OUTSIDE the dev-mode check, compiled in both modes.
# ============================================================================
set(CORE_DRV ${CORE_BASE}/${CORE_DRV_DIR})

# ------------------------------------------------------------------
# 3a. Always-compiled core drivers + tlk_trap.c
#     (core/CMakeLists.txt lines 7, 12-15)
# ------------------------------------------------------------------
zephyr_sources(${CORE_BASE}/${CORE_COMMON}/src/tlk_trap.c)
zephyr_sources(${CORE_DRV}/tlk_analog.c)
zephyr_sources(${CORE_DRV}/tlk_clock.c)
zephyr_sources(${CORE_DRV}/tlk_stimer.c)
zephyr_sources(${CORE_DRV}/tlk_power.c)

# ------------------------------------------------------------------
# 3b. Conditional drivers — matching core/CMakeLists.txt lines 17-29
#     (add_sources_ifdef, outside SDK check)
# ------------------------------------------------------------------
zephyr_sources_ifdef(CONFIG_TLK_ADC            ${CORE_DRV}/tlk_adc.c)
zephyr_sources_ifdef(CONFIG_TLK_DMA            ${CORE_DRV}/tlk_dma.c)
zephyr_sources_ifdef(CONFIG_TLK_GPIO           ${CORE_DRV}/tlk_gpio.c)
zephyr_sources_ifdef(CONFIG_TLK_MTIMER         ${CORE_DRV}/tlk_mtimer.c)
zephyr_sources_ifdef(CONFIG_TLK_PLIC           ${CORE_DRV}/tlk_plic.c)
zephyr_sources_ifdef(CONFIG_TLK_PLIC_SW        ${CORE_DRV}/tlk_plic_sw.c)
zephyr_sources_ifdef(CONFIG_TLK_SPI            ${CORE_DRV}/tlk_spi.c)
zephyr_sources_ifdef(CONFIG_TLK_WDT            ${CORE_DRV}/tlk_watchdog.c)
zephyr_sources_ifdef(CONFIG_TLK_DEBUG_INTERFACES ${CORE_DRV}/tlk_jtag_sws.c)

# --- UART ---
if(CONFIG_TLK_UART)
    unisdk_assert_file_exists(${CORE_DRV}/tlk_uart.c)
    zephyr_sources(${CORE_DRV}/tlk_uart.c)
endif()

# --- I2C ---
if(CONFIG_TLK_I2C)
    unisdk_assert_file_exists(${CORE_DRV}/tlk_i2c.c)
    unisdk_assert_file_exists(${UNISDK_BASE}/core/common/src/tlk_i2c.c)
    zephyr_sources(
        ${CORE_DRV}/tlk_i2c.c
        ${UNISDK_BASE}/core/common/src/tlk_i2c.c
    )
endif()

# --- USB ---
if(CONFIG_TLK_USB)
    unisdk_assert_file_exists(${CORE_DRV}/tlk_usb.c)
    zephyr_sources(${CORE_DRV}/tlk_usb.c)
endif()

# --- PM ---
if(CONFIG_TLK_PM)
    zephyr_sources(${CORE_DRV}/tlk_pm.c)
endif()

zephyr_sources_ifdef(CONFIG_TLK_CRITICAL_PRINTOUT
    ${CORE_BASE}/${CORE_COMMON}/src/tlk_critical_print.c
)

# ============================================================================
# 4. Dev mode — compile dev-only sources (mirroring core/CMakeLists.txt lines 31-50)
# ============================================================================
if(NOT EXISTS "${UNISDK_BASE}/libraries")
    # _tlk_sys.c — core/CMakeLists.txt line 32
    zephyr_sources(${CORE_DRV}/_tlk_sys.c)

    # _tlk_pm.c — core/CMakeLists.txt line 34 (inside SDK check)
    if(CONFIG_TLK_PM)
        zephyr_sources(${CORE_DRV}/_tlk_pm.c)
    endif()

    # RF sources — core/CMakeLists.txt lines 36-39 (inside SDK check)
    if(CONFIG_TLK_RF)
        zephyr_sources(${CORE_BASE}/${CORE_COMMON}/src/_tlk_rf.c)
        zephyr_sources(${CORE_DRV}/_tlk_rf_init.c)
        zephyr_sources(${CORE_DRV}/_tlk_rf_settle.c)
        zephyr_sources(${CORE_DRV}/_tlk_rf.c)
    endif()

    # ------------------------------------------------------------------
    # 4a. Errata sources (GLOB — errata files are added/removed frequently)
    # ------------------------------------------------------------------
    file(GLOB_RECURSE ERRATA_SRCS "${CORE_DRV}/errata/*.c")
    zephyr_sources(${ERRATA_SRCS})

endif()
