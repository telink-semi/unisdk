# SPDX-License-Identifier: Apache-2.0
#
# UniSDK Zephyr module — common library
#
# This file provides Zephyr build integration for the common UniSDK layer:
# mandatory sources, include paths, and compiler options that every
# UniSDK compilation unit requires.
#
# Variables expected from the caller (set in zephyr/CMakeLists.txt):
#   UNISDK_BASE  - path to the unisdk root directory

# ============================================================================
# 1. Include paths
# ============================================================================
zephyr_include_directories(
    ${UNISDK_BASE}                              # for "common/tlk_init.h", "core/include/tlk_xxx.h"
    ${UNISDK_BASE}/common/include               # for tlk_types.h, tlk_bit.h, etc.
)

# ============================================================================
# 2. Compiler options — forced includes required by all UniSDK code
# ============================================================================
set(UNISDK_COMMON_INC ${UNISDK_BASE}/common/include)

zephyr_compile_options(
    "--imacros=${UNISDK_COMMON_INC}/tlk_soc_addr.h"
    "--imacros=${UNISDK_COMMON_INC}/tlk_bit.h"
    "--imacros=${UNISDK_COMMON_INC}/tlk_compiler.h"
    "--imacros=${UNISDK_COMMON_INC}/tlk_config_macro.h"
    $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-include ${UNISDK_COMMON_INC}/tlk_types.h>
)

# ============================================================================
# 3. Common sources
# ============================================================================
zephyr_sources(
    ${UNISDK_BASE}/common/src/tlk_sdk_version.c
    ${UNISDK_BASE}/common/src/tlk_init.c
    ${UNISDK_BASE}/common/src/tlk_loop.c
)
