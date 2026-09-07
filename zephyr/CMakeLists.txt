# SPDX-License-Identifier: Apache-2.0
#
# UniSDK Zephyr module integration CMakeLists.txt
#
# This file integrates the Telink unified SDK (UniSDK) into Zephyr's
# build system.  It is the single entry point declared in module.yml.
#
# Architecture:
#   This file is intentionally kept as a thin orchestrator.  Build
#   logic for loosely-coupled subsystems (BLE controller, etc.) and
#   logically-grouped driver families lives in separate .cmake files
#   within each module directory, which are included below.  This keeps
#   concerns separated, makes dependencies explicit, and allows
#   individual subsystems to be versioned or swapped independently.
#
# Variables set by Zephyr's module system:
#   ZEPHYR_CURRENT_MODULE_DIR  - path to the unisdk root directory
#   ZEPHYR_CURRENT_MODULE_NAME - "unisdk"

if(CONFIG_HAS_UNISDK)

# ============================================================================
# 1. Variable setup
# ============================================================================
set(UNISDK_BASE      ${ZEPHYR_CURRENT_MODULE_DIR})
set(UNISDK_GEN_DIR   ${CMAKE_BINARY_DIR}/unisdk_generated)

# ============================================================================
# 2. Generate C headers from YAML property and register descriptions
# ============================================================================
# UniSDK uses YAML files to describe hardware properties and registers.
# These must be converted to C header files BEFORE any compilation starts,
# because tlk_soc_addr.h (included via --imacros) unconditionally includes
# "properties/tlk_chip.h".  The headers are generated at CMake configure time
# via execute_process() so they exist before the first compilation unit.
#
# The scripts are pure YAML->C converters with no Kconfig dependency.

set(UNISDK_GEN_SCRIPT_DIR ${UNISDK_BASE}/scripts/generators)

# Helper macro: generate a single C header from a YAML file at configure time
macro(unisdk_gen_yaml_at_configure_time SCRIPT YAML_DIR OUT_DIR)
    if(EXISTS "${YAML_DIR}")
        file(MAKE_DIRECTORY "${OUT_DIR}")
        file(GLOB yaml_files "${YAML_DIR}/*.yaml")
        foreach(yaml_file IN LISTS yaml_files)
            get_filename_component(file_name "${yaml_file}" NAME_WE)
            set(header_file "${OUT_DIR}/tlk_${file_name}.h")

            execute_process(
                COMMAND ${PYTHON_EXECUTABLE}
                        -m "scripts.generators.${SCRIPT}"
                        "${yaml_file}" "${header_file}"
                WORKING_DIRECTORY ${UNISDK_BASE}
                RESULT_VARIABLE gen_result
                OUTPUT_VARIABLE gen_output
                ERROR_VARIABLE  gen_error
            )
            if(NOT gen_result EQUAL 0)
                message(WARNING "UniSDK: failed to generate ${header_file}: ${gen_error}")
            endif()
        endforeach()
    endif()
endmacro()

# --- core properties (chip, gpio, pm, uart, etc.) ---
set(CORE_PROP_YAML_DIR   ${UNISDK_BASE}/core/${CONFIG_TLK_CORE}/properties)
set(CORE_PROP_OUT_DIR    ${UNISDK_GEN_DIR}/core/${CONFIG_TLK_CORE}/properties)

# --- core registers ---
set(CORE_REG_YAML_DIR    ${UNISDK_BASE}/core/${CONFIG_TLK_CORE}/registers)
set(CORE_REG_OUT_DIR     ${UNISDK_GEN_DIR}/core/${CONFIG_TLK_CORE}/registers)

# --- SoC properties (rf, etc.) ---
set(SOC_PROP_YAML_DIR    ${UNISDK_BASE}/soc/${CONFIG_TLK_SOC_SERIES}/properties)
set(SOC_PROP_OUT_DIR     ${UNISDK_GEN_DIR}/soc/${CONFIG_TLK_SOC_SERIES}/properties)

# --- SoC registers ---
set(SOC_REG_YAML_DIR     ${UNISDK_BASE}/soc/${CONFIG_TLK_SOC_SERIES}/registers)
set(SOC_REG_OUT_DIR      ${UNISDK_GEN_DIR}/soc/${CONFIG_TLK_SOC_SERIES}/registers)

unisdk_gen_yaml_at_configure_time(properties_gen "${CORE_PROP_YAML_DIR}" "${CORE_PROP_OUT_DIR}")
unisdk_gen_yaml_at_configure_time(registers_gen  "${CORE_REG_YAML_DIR}"  "${CORE_REG_OUT_DIR}")
unisdk_gen_yaml_at_configure_time(properties_gen "${SOC_PROP_YAML_DIR}"  "${SOC_PROP_OUT_DIR}")
unisdk_gen_yaml_at_configure_time(registers_gen  "${SOC_REG_YAML_DIR}"   "${SOC_REG_OUT_DIR}")

# Board properties (if a board is selected), but not required. Because
# the board properties for unisdk are not used in the Zephyr.

# ============================================================================
# 3. Generated headers (cross-module)
# ============================================================================

# Generated headers (from YAML)
zephyr_include_directories(
    ${UNISDK_GEN_DIR}/core/${CONFIG_TLK_CORE}
    ${UNISDK_GEN_DIR}/soc/${CONFIG_TLK_SOC_SERIES}
)

# Board include path，but not required. Because the board properties for unisdk are not used in the Zephyr.

# ============================================================================
# 4. Module integrations
#
# Each module's zephyr.cmake handles both modes internally:
#   - Dev mode  (libraries/ does NOT exist): compile sources via zephyr_sources()
#   - SDK mode  (libraries/     exists):     link prebuilt .a to the interface
#
# This keeps library naming and linking logic co-located with the module
# that owns the code.
# ============================================================================
set(_core_lib_dir "${UNISDK_BASE}/libraries")
set(_core_lib_base "telink-unisdk-${CONFIG_TLK_SOC_SERIES}")

# Create the module interface target so that target_link_libraries works
# for both the core SDK libs and subsystem modules (BLE controller, etc.).
add_library(${ZEPHYR_CURRENT_MODULE_NAME}_interface INTERFACE)
target_link_libraries(zephyr_interface INTERFACE ${ZEPHYR_CURRENT_MODULE_NAME}_interface)

# Internal helper: like telink_add_library(.. CONDITION ..) but for Zephyr
macro(tlk_link_sdk suffix)
    set(_link_cond TRUE)
    if(ARGC GREATER 2 AND "${ARGV1}" STREQUAL "CONDITION")
        set(_link_cond ${ARGV2})
    endif()
    if(${_link_cond})
        set(_lib_path "${_core_lib_dir}/lib${_core_lib_base}${suffix}.a")
        if(EXISTS "${_lib_path}")
            target_link_libraries(${ZEPHYR_CURRENT_MODULE_NAME}_interface
                                    INTERFACE "${_lib_path}")
            message(STATUS "UniSDK SDK: Linking ${_lib_path}")
        endif()
    endif()
endmacro()

# Common - always needed (compiler options, base includes, common sources)
include(${UNISDK_BASE}/common/zephyr.cmake)

# Core drivers - always needed
include(${UNISDK_BASE}/core/zephyr.cmake)

# SoC-specific drivers - always needed
include(${UNISDK_BASE}/soc/zephyr.cmake)

# API layer - optional modules
include(${UNISDK_BASE}/api/zephyr.cmake)

# BLE controller (independently-versioned subsystem)
if(CONFIG_BT_TLX)
    include(${UNISDK_BASE}/subsystem/ble/controller/zephyr.cmake)
endif()

# Main SOC lib — linked LAST so the single-pass BFD linker can resolve
# backward references from the BLE controller.
tlk_link_sdk("-zephyr")

# ============================================================================
# 5. Kconfig preprocessing (optional)
#
#     Preprocess native UniSDK core/configs/Kconfig.* files that use
#     ${...} YAML macro references and for-range loops (UniSDK Kconfig
#     preprocessor extensions).  The generated output is written to
#     ${UNISDK_GEN_DIR}/kconfig/ and can be referenced from zephyr/Kconfig
#     via source "$(KCONFIG_BINARY_DIR)/..." or orsource.
#
#     This step is optional: if it fails, the hand-written per-module
#     Kconfig.zephyr files serve as fallback.
# ============================================================================
if(CONFIG_TLK_CORE AND CONFIG_TLK_SOC_SERIES)
    set(UNISDK_KCONFIG_GEN_DIR ${UNISDK_GEN_DIR}/kconfig)

    # Write a temporary chip.config so that preprocess_kconfig.py
    # can read the CONFIG_TLK_CORE / CONFIG_TLK_SOC_SERIES values.
    # (The script normally reads this from the native build directory.)
    set(_chip_config ${UNISDK_KCONFIG_GEN_DIR}/chip.config)
    file(WRITE ${_chip_config} "CONFIG_TLK_CORE=\"${CONFIG_TLK_CORE}\"\n")
    file(APPEND ${_chip_config} "CONFIG_TLK_SOC_SERIES=\"${CONFIG_TLK_SOC_SERIES}\"\n")
    if(CONFIG_TLK_BOARD)
        file(APPEND ${_chip_config} "CONFIG_TLK_BOARD=\"${CONFIG_TLK_BOARD}\"\n")
    endif()

    execute_process(
        COMMAND ${PYTHON_EXECUTABLE}
                ${UNISDK_BASE}/scripts/preprocess_kconfig.py
                --root-dir ${UNISDK_BASE}
                --build-dir ${UNISDK_KCONFIG_GEN_DIR}
        WORKING_DIRECTORY ${UNISDK_BASE}
        RESULT_VARIABLE _kconfig_preproc_result
        OUTPUT_VARIABLE _kconfig_preproc_output
        ERROR_VARIABLE  _kconfig_preproc_error
    )
    if(_kconfig_preproc_result EQUAL 0)
        message(STATUS "UniSDK: Kconfig preprocessing succeeded -> ${UNISDK_KCONFIG_GEN_DIR}")
    else()
        message(STATUS "UniSDK: Kconfig preprocessing skipped (${_kconfig_preproc_result}), "
                       "using hand-written Kconfig.zephyr files")
    endif()
endif()

endif() # CONFIG_HAS_UNISDK
