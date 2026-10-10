# SPDX-License-Identifier: Apache-2.0
#
# Cmake Configuration Time options in the kconfig.cmake:
#   SOC_SERIES - SOC_SERIES to be used
#   SOC - SOC to be used
#   BOARD - BOARD to be used

include_guard(GLOBAL)

set(MENUCONFIG_SCRIPT ${TELINK_BASE}/scripts/menuconfig.py) # Menuconfig script

# === Configuration file: chip.config ===
set(CHIP_CONFIG_FILE ${CMAKE_BINARY_DIR}/chip.config)
set(CHIP_GEN_SCRIPT ${TELINK_BASE}/scripts/generators/chip_config_gen.py)

# At least one MCU package is required: packages/Kconfig.mcu builds the "SoC Series" choice from
# packages/*/Kconfig.core, so without one chip.config stays empty and no core can be selected
file(GLOB MCU_KCONFIG_CORES ${TELINK_BASE}/packages/*/Kconfig.core)
if(NOT MCU_KCONFIG_CORES)
    message(FATAL_ERROR
        "No MCU package installed in ${TELINK_BASE}/packages.\n"
        "Install one, e.g.:\n"
        "  python scripts/tlkpkg/registry_client.py install-mcu <CORE> --target-root ${TELINK_BASE}\n"
        "or from the Component Registry in the Telink UniSDK VS Code extension.")
endif()

# Create a list of the parameters passed for the CMake configuration (SOC_SERIES, SOC, BOARD),
# and unset them, as they need to be processed only once-either - to set up the hardware (if chip.config is missing),
# or to modify the hardware configuration (if chip.config is present)
set(CHIP_CONFIG_FLAGS "")

if(SOC_SERIES)
    message(STATUS "Option SOC_SERIES: ${SOC_SERIES}")
    list(APPEND CHIP_CONFIG_FLAGS --soc-series ${SOC_SERIES})
endif()

if(SOC)
    message(STATUS "Option SOC: ${SOC}")
    list(APPEND CHIP_CONFIG_FLAGS --soc ${SOC})
endif()

if(BOARD)
    message(STATUS "Option BOARD: ${BOARD}")
    list(APPEND CHIP_CONFIG_FLAGS --board ${BOARD})
endif()

unset(SOC_SERIES CACHE)
unset(SOC CACHE)
unset(BOARD CACHE)


# Custom target for generating chip.config
add_custom_target(
    config_chip
    COMMAND ${CMAKE_COMMAND} -E env
            KCONFIG_CONFIG=${CHIP_CONFIG_FILE}
            ${PYTHON_EXECUTABLE} ${MENUCONFIG_SCRIPT} ${TELINK_BASE}/Kconfig.chip
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
    DEPENDS
        ${TELINK_BASE}/Kconfig.chip
        ${MENUCONFIG_SCRIPT}
    USES_TERMINAL
    COMMENT "Generating chip.config"
)


# Check if the chip.config is configured for the Cmake Configure Time OR chip.config update request by flags
if(NOT EXISTS ${CHIP_CONFIG_FILE} OR CHIP_CONFIG_FLAGS)
    if(NOT EXISTS ${CHIP_CONFIG_FILE})
        message(STATUS "chip.config is not configured")
        message(STATUS "Preparing default chip.config")
    else()
        message(STATUS "chip.config update request by flags")
    endif()

    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} ${CHIP_GEN_SCRIPT}
                ${TELINK_BASE}/Kconfig.chip ${CHIP_CONFIG_FILE} ${CHIP_CONFIG_FLAGS}
        WORKING_DIRECTORY ${TELINK_BASE}
        RESULT_VARIABLE RESULT
    )

    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "chip.config generation failed")
    endif()
endif()

if(NOT EXISTS ${CHIP_CONFIG_FILE})
    message(FATAL_ERROR "chip.config is still not configured")
endif()


# Parse the chip.config configuration for Cmake variables
parse_configs(${CHIP_CONFIG_FILE})

# Print the selected CORE
if(CONFIG_TLK_CORE)
    message(STATUS "Using ${CONFIG_TLK_CORE} core")
    message(STATUS "Using ${CONFIG_TLK_SOC_SERIES} SOC series")
else()
    message(FATAL_ERROR "Core is not selected")
endif()

# Print the selected SOC
if(CONFIG_TLK_SOC)
    message(STATUS "Using ${CONFIG_TLK_SOC} SOC")
else()
    message(FATAL_ERROR "SOC is not selected")
endif()

# Print the selected BOARD
if(CONFIG_TLK_BOARD_IS_SELECTED)
    message(STATUS "Using ${CONFIG_TLK_BOARD} board")
else()
    message(STATUS "Board is not selected")
endif()

# === Configuration file: chip.config ===


# === Configuration file: build.config ===
set(BUILD_CONFIG_FILE ${CMAKE_BINARY_DIR}/build.config)
set(KCONFIG_APPLICATION_DIR "${APPLICATION_SOURCE_DIR}" CACHE INTERNAL "Application directory for Kconfig") # Application directory for Kconfig


# === Part 1: Kconfig files parsing
if(NOT IS_SDK_MODE)
    set(YAML_CACHE_GEN_SCRIPT "${TELINK_BASE}/scripts/generators/yaml_cache_gen.py") # Script to generate properties cache file
    set(KCONFIG_PARSER_SCRIPT "${TELINK_BASE}/scripts/parsers/parse_kconfig.py")     # Script to parse the Kconfig file

    if(CONFIG_TLK_BOARD_IS_SELECTED)
        set(GENERATED_KCONFIG_BASE_DIR "${GENERATED_BASE_DIR}/kconfig/${CONFIG_TLK_SOC_SERIES}/${CONFIG_TLK_BOARD}" CACHE INTERNAL "Directory to generate kconfigs in")   # Directory to generate kconfigs in
        set(KCONFIG_CACHE_DIR          "${GENERATED_BASE_DIR}/_attributes/${CONFIG_TLK_SOC_SERIES}/${CONFIG_TLK_BOARD}") # Directory for Kconfig auto-generation attributes
    else()
        set(GENERATED_KCONFIG_BASE_DIR "${GENERATED_BASE_DIR}/kconfig/${CONFIG_TLK_SOC_SERIES}" CACHE INTERNAL "Directory to generate kconfigs in")     # Directory to generate kconfigs in
        set(KCONFIG_CACHE_DIR          "${GENERATED_BASE_DIR}/_attributes/${CONFIG_TLK_SOC_SERIES}") # Directory for Kconfig auto-generation attributes
    endif()

    set(PROPERTIES_CACHE_FILE "${KCONFIG_CACHE_DIR}/properties.pkl") # Cache file containing parsed data from all property YAML files


    # Include YAML helpers
    include(yaml_helper)

    # Custom command to automatically update the parsed properties cache if any CORE, SOC, or BOARD YAML file changes
    add_custom_command(
        OUTPUT ${PROPERTIES_CACHE_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${KCONFIG_CACHE_DIR}"
        COMMAND ${PYTHON_EXECUTABLE} ${YAML_CACHE_GEN_SCRIPT}
            ${PROPERTIES_CACHE_FILE}
            ${PROPERTY_YAML_FILES}
        DEPENDS
            ${YAML_CACHE_GEN_SCRIPT}
            ${PROPERTY_YAML_FILES}
        COMMENT "Generating properties cache file"
        VERBATIM
        WORKING_DIRECTORY ${TELINK_BASE}
    )


    # Create a list of all Kconfig files that have to be parsed for selected CORE, SOC, BOARD
    set(KCONFIGS_TO_PARSE "")

    macro(add_kconfigs_to_parse SOURCES)
        list(GET ${SOURCES} 0 src_dir)
        list(GET ${SOURCES} 1 out_dir)

        if(EXISTS "${src_dir}")
            file(GLOB kconfig_files "${src_dir}/Kconfig*")

            foreach(kconfig_file IN LISTS kconfig_files)
                get_filename_component(file_name "${kconfig_file}" NAME)
                set(kconfig_out_file "${out_dir}/${file_name}")

                add_custom_command(
                    OUTPUT "${kconfig_out_file}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${out_dir}"
                    COMMAND ${PYTHON_EXECUTABLE} ${KCONFIG_PARSER_SCRIPT}
                        "${PROPERTIES_CACHE_FILE}"
                        "${kconfig_file}"
                        "${kconfig_out_file}"
                    DEPENDS
                        ${KCONFIG_PARSER_SCRIPT}
                        ${kconfig_file}
                        ${PROPERTIES_CACHE_FILE}
                    COMMENT "Parsing ${kconfig_out_file}"
                    VERBATIM
                    WORKING_DIRECTORY "${TELINK_BASE}"
                )

                list(APPEND KCONFIGS_TO_PARSE "${kconfig_out_file}")
            endforeach()
        endif()
    endmacro()


    # Kconfig sources
    set(API_KCONFIG_SOURCES
        "${TELINK_BASE}/core/api"
        "${GENERATED_KCONFIG_BASE_DIR}/core/api"
    )

    set(CORE_KCONFIG_SOURCES
        "${TELINK_BASE}/core/configs"
        "${GENERATED_KCONFIG_BASE_DIR}/core/configs"
    )

    set(SYSTEM_KCONFIG_SOURCES
        "${TELINK_BASE}/system"
        "${GENERATED_KCONFIG_BASE_DIR}/system"
    )

    add_kconfigs_to_parse(API_KCONFIG_SOURCES)
    add_kconfigs_to_parse(CORE_KCONFIG_SOURCES)
    add_kconfigs_to_parse(SYSTEM_KCONFIG_SOURCES)


    # Custom target for parsing Kconfig files
    add_custom_target(
        parse_kconfig
        DEPENDS
            ${TELINK_BASE}/Kconfig.build
            ${KCONFIGS_TO_PARSE}
        COMMENT "Parsing Kconfig files"
    )

else()
    # Parsed at export time and installed with the MCU package (scripts/tlkpkg/mcu/mcu_export.py)
    if(CONFIG_TLK_BOARD_IS_SELECTED)
        set(GENERATED_KCONFIG_BASE_DIR "${TELINK_BASE}/packages/${CONFIG_TLK_CORE}/kconfig/${CONFIG_TLK_SOC_SERIES}/${CONFIG_TLK_BOARD}" CACHE INTERNAL "Directory with generated kconfigs") # Directory with generated kconfigs
    else()
        set(GENERATED_KCONFIG_BASE_DIR "${TELINK_BASE}/packages/${CONFIG_TLK_CORE}/kconfig/${CONFIG_TLK_SOC_SERIES}" CACHE INTERNAL "Directory with generated kconfigs") # Directory with generated kconfigs
    endif()
endif()


# === Part 2: build.config functionality
set(PRJ_CONF_FILE "${APPLICATION_SOURCE_DIR}/prj.conf")
set(BUILD_CONFIG_PREREQUISITES "")

if(NOT IS_SDK_MODE)
    list(APPEND BUILD_CONFIG_PREREQUISITES parse_kconfig)
endif()


# Custom target to parse prj.conf file
if(NOT EXISTS ${BUILD_CONFIG_FILE} AND EXISTS ${PRJ_CONF_FILE})
    message(STATUS "Using default sample configs from: ${PRJ_CONF_FILE}")

    add_custom_target(
        _parse_prj_conf
        COMMAND ${CMAKE_COMMAND} -E env
            KCONFIG_BINARY_DIR=${GENERATED_KCONFIG_BASE_DIR}
            KCONFIG_APPLICATION_DIR=${KCONFIG_APPLICATION_DIR}
            ${PYTHON_EXECUTABLE} -c
            "import sys, kconfiglib; k=kconfiglib.Kconfig(r'${TELINK_BASE}/Kconfig.build'); k.load_config(sys.argv[1]); k.write_config(sys.argv[2])"
            "${PRJ_CONF_FILE}"
            "${BUILD_CONFIG_FILE}"
        WORKING_DIRECTORY ${TELINK_BASE}
        DEPENDS
            ${TELINK_BASE}/Kconfig.build
        VERBATIM
    )

    list(APPEND BUILD_CONFIG_PREREQUISITES _parse_prj_conf)
endif()


# Custom target for generating build.config
add_custom_target(
    config_build
    COMMAND ${CMAKE_COMMAND} -E env
            KCONFIG_CONFIG=${BUILD_CONFIG_FILE}
            KCONFIG_BINARY_DIR=${GENERATED_KCONFIG_BASE_DIR}
            KCONFIG_APPLICATION_DIR=${KCONFIG_APPLICATION_DIR}
            ${PYTHON_EXECUTABLE} ${MENUCONFIG_SCRIPT} ${TELINK_BASE}/Kconfig.build
    WORKING_DIRECTORY ${TELINK_BASE}
    DEPENDS
        ${TELINK_BASE}/Kconfig.build
        ${MENUCONFIG_SCRIPT}
    VERBATIM
    USES_TERMINAL
    COMMENT "Generating build.config file via menuconfig GUI"
)

# Prerequisites are wired with add_dependencies, never with a nested
# `cmake --build` of this build directory: on Windows a nested ninja that has
# to regenerate build.ninja fails ("failed recompaction: Permission denied")
# because the outer ninja keeps .ninja_log open.
add_dependencies(config_build ${BUILD_CONFIG_PREREQUISITES})


# Non-interactive counterpart of config_build: (re)write build.config from
# the application's prj.conf (or plain Kconfig defaults without one), no
# menuconfig. Run by the Makefile's cmake/reconfigure targets whenever
# build.config is missing, so a fresh or just-switched application builds
# with its own prj.conf right away.
set(BUILD_CONFIG_DEFAULT_INPUTS "")

if(EXISTS ${PRJ_CONF_FILE})
    set(BUILD_CONFIG_DEFAULT_INPUTS "${PRJ_CONF_FILE}")
endif()

add_custom_target(
    config_build_default
    COMMAND ${CMAKE_COMMAND} -E env
        KCONFIG_BINARY_DIR=${GENERATED_KCONFIG_BASE_DIR}
        KCONFIG_APPLICATION_DIR=${KCONFIG_APPLICATION_DIR}
        ${PYTHON_EXECUTABLE} -c
        "import sys, kconfiglib; k=kconfiglib.Kconfig(r'${TELINK_BASE}/Kconfig.build'); [k.load_config(f) for f in sys.argv[2:]]; k.write_config(sys.argv[1])"
        "${BUILD_CONFIG_FILE}"
        ${BUILD_CONFIG_DEFAULT_INPUTS}
    WORKING_DIRECTORY ${TELINK_BASE}
    VERBATIM
    COMMENT "Generating build.config from the application's prj.conf"
)

if(NOT IS_SDK_MODE)
    add_dependencies(config_build_default parse_kconfig)
endif()


# Parse the build.config for Cmake variables
parse_configs(${BUILD_CONFIG_FILE})

# === Configuration file: build.config ===


# All configuration files
set(CONFIG_FILES
    ${CHIP_CONFIG_FILE}
    ${BUILD_CONFIG_FILE}
)

# If any file from CONFIG_FILES changes - autoautomatically reconfigure the CMake
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CONFIG_FILES})


# === Auto-generated file: autoconf.h ===
set(AUTOCONF_H ${CMAKE_BINARY_DIR}/autoconf.h)                               # C header file with all configs (CONFIG_FILES)
set(AUTOCONF_GEN_SCRIPT "${TELINK_BASE}/scripts/generators/config_h_gen.py") # Script to generate configs header from the config files


# Custom command to automatically update the AUTOCONF_H if any file from CONFIG_FILES changes
add_custom_command(
    OUTPUT  ${AUTOCONF_H}
    COMMAND ${PYTHON_EXECUTABLE} ${AUTOCONF_GEN_SCRIPT}
        ${AUTOCONF_H}
        ${CONFIG_FILES}
    DEPENDS ${CONFIG_FILES}
    COMMENT "Generating autoconf.h"
    VERBATIM
)

# Custom target for automatically update the AUTOCONF_H
add_custom_target(
    generate_autoconf
    DEPENDS ${AUTOCONF_H}
    COMMENT "Generating autoconf.h"
)
# === Auto-generated file: autoconf.h ===


list(APPEND FILES_TO_CLEAN ${CONFIG_FILES} ${AUTOCONF_H})
