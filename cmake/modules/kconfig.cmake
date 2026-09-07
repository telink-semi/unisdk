# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

set(MENUCONFIG_SCRIPT ${TELINK_BASE}/scripts/menuconfig.py) # Menuconfig script

# SOC/BOARD may be passed on the command line (e.g. `make cmake BOARD=TL3218X_EVK`)
# to preselect the chip.config choice symbols on first-time, non-interactive
# generation below. They only affect chip.config the first time it's generated;
# once chip.config exists, edit it directly (or delete it) to change SOC/BOARD.
set(SOC "" CACHE STRING "SoC to preselect when generating chip.config (e.g. TL3218X)")
set(BOARD "" CACHE STRING "Board to preselect when generating chip.config (e.g. TL3218X_EVK)")
if(SOC OR BOARD)
    message(STATUS "Requested SOC='${SOC}' BOARD='${BOARD}' for chip.config generation")
endif()

# === Configuration file: chip.config ===
set(CHIP_CONFIG_FILE ${CMAKE_BINARY_DIR}/chip.config)

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


# TODO: FIX!!!
# Check if the chip.config is configured for the Cmake Configure Time
if(NOT EXISTS ${CHIP_CONFIG_FILE})
    message(STATUS "chip.config is not configured")

    # The SOC/BOARD choices in soc/Kconfig and boards/Kconfig are 'optional'
    # (BOARD) or have no forced default matching the caller's intent, so a
    # plain non-interactive kconfiglib pass won't select them on its own. If
    # SOC/BOARD were requested, seed a partial config that kconfiglib loads
    # before writing the final chip.config, so the choice comes out selected
    # instead of silently being left unset.
    set(_TLK_CHIP_SEED_FILE "${CMAKE_BINARY_DIR}/chip.config.seed")
    set(_TLK_CHIP_SEED_ARGS "")
    if(SOC OR BOARD)
        set(_TLK_CHIP_SEED_LINES "")
        if(SOC)
            string(APPEND _TLK_CHIP_SEED_LINES "CONFIG_TLK_SOC_${SOC}=y\n")
        endif()
        if(BOARD)
            string(APPEND _TLK_CHIP_SEED_LINES "CONFIG_TLK_BOARD_${BOARD}=y\n")
        endif()
        file(WRITE "${_TLK_CHIP_SEED_FILE}" "${_TLK_CHIP_SEED_LINES}")
        set(_TLK_CHIP_SEED_ARGS "${_TLK_CHIP_SEED_FILE}")
    endif()

    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} -c
            "import kconfiglib, sys; kconf = kconfiglib.Kconfig(sys.argv[1]); kconf.load_config(sys.argv[3], replace=False) if len(sys.argv) > 3 and sys.argv[3] else None; kconf.write_config(sys.argv[2])"
            "${TELINK_BASE}/Kconfig.chip"
            "${CHIP_CONFIG_FILE}"
            "${_TLK_CHIP_SEED_ARGS}"
        WORKING_DIRECTORY "${TELINK_BASE}"
        RESULT_VARIABLE RESULT
    )

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
if(CONFIG_TLK_HAS_BOARD_SELECTED)
    message(STATUS "Using ${CONFIG_TLK_BOARD} board")
else()
    message(STATUS "Board is not selected")
    if(BOARD)
        message(WARNING
            "BOARD='${BOARD}' was requested but is not selected in "
            "${CHIP_CONFIG_FILE}. Either the name is wrong/incompatible with "
            "the selected SOC, or chip.config already existed from an earlier "
            "run (BOARD/SOC only take effect the first time chip.config is "
            "generated). Delete ${CHIP_CONFIG_FILE} and reconfigure, or edit "
            "it directly, to fix the board selection.")
    endif()
endif()

# === Configuration file: chip.config ===


# === Configuration file: build.config ===
set(BUILD_CONFIG_FILE ${CMAKE_BINARY_DIR}/build.config)


# === Part 1: Kconfig files parsing
if(NOT IS_SDK_MODE)
    option(PARSE_ALL_SAMPLE_KCONFIGS "Parse Kconfig files for all samples, not only the selected one" OFF)

    set(YAML_CACHE_GEN_SCRIPT "${TELINK_BASE}/scripts/generators/yaml_cache_gen.py") # Script to generate properties cache file
    set(KCONFIG_PARSER_SCRIPT "${TELINK_BASE}/scripts/parsers/parse_kconfig.py")     # Script to parse the Kconfig file

    if(CONFIG_TLK_HAS_BOARD_SELECTED)
        set(GENERATED_KCONFIG_BASE_DIR "${GENERATED_BASE_DIR}/kconfig/${CONFIG_TLK_SOC_SERIES}/${CONFIG_TLK_BOARD}" CACHE INTERNAL "Directory to generate kconfigs in")   # Directory to generate kconfigs in
        set(KCONFIG_CACHE_DIR          "${GENERATED_BASE_DIR}/_attributes/${CONFIG_TLK_SOC_SERIES}/${CONFIG_TLK_BOARD}") # Directory for Kconfig auto-generation attributes
    else()
        set(GENERATED_KCONFIG_BASE_DIR "${GENERATED_BASE_DIR}/kconfig/${CONFIG_TLK_SOC_SERIES}" CACHE INTERNAL "Directory to generate kconfigs in")     # Directory to generate kconfigs in
        set(KCONFIG_CACHE_DIR          "${GENERATED_BASE_DIR}/_attributes/${CONFIG_TLK_SOC_SERIES}") # Directory for Kconfig auto-generation attributes
    endif()

    set(KCONFIG_APPLICATION_DIR "${GENERATED_KCONFIG_BASE_DIR}/samples/${TLK_SAMPLE_NAME}" CACHE INTERNAL "Application directory for Kconfig") # Application directory for Kconfig
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


    # Kconfig sources
    set(API_KCONFIG_SOURCES
        "${TELINK_BASE}/api"
        "${GENERATED_KCONFIG_BASE_DIR}/api"
    )

    set(CORE_KCONFIG_SOURCES
        "${TELINK_BASE}/core/configs"
        "${GENERATED_KCONFIG_BASE_DIR}/core/configs"
    )

    set(SYSTEM_KCONFIG_SOURCES
        "${TELINK_BASE}/system"
        "${GENERATED_KCONFIG_BASE_DIR}/system"
    )


    # Create a list of all Kconfig files that have to be parsed for selected CORE, SOC, BOARD
    set(KCONFIGS_TO_PARSE "")

    # Command to parse Kconfig files
    set(PARSE_KCONFIGS_COMMAND "")

    macro(add_kconfigs_to_parse SOURCES)
        list(GET ${SOURCES} 0 src_dir)
        list(GET ${SOURCES} 1 out_dir)

        if(EXISTS "${src_dir}")
            file(GLOB kconfig_files "${src_dir}/Kconfig*")

            foreach(kconfig_file IN LISTS kconfig_files)
                get_filename_component(file_name "${kconfig_file}" NAME)
                set(kconfig_out_file "${out_dir}/${file_name}")

                # TODO: FIX!!!
                set(CURRENT_PARSE_COMMAND
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${out_dir}"
                    COMMAND ${PYTHON_EXECUTABLE} ${KCONFIG_PARSER_SCRIPT}
                        "${PROPERTIES_CACHE_FILE}"
                        "${kconfig_file}"
                        "${kconfig_out_file}"
                )

                add_custom_command(
                    OUTPUT "${kconfig_out_file}"
                    ${CURRENT_PARSE_COMMAND}
                    DEPENDS
                        ${KCONFIG_PARSER_SCRIPT}
                        ${kconfig_file}
                        ${PROPERTIES_CACHE_FILE}
                    COMMENT "Parsing ${kconfig_out_file}"
                    VERBATIM
                    WORKING_DIRECTORY "${TELINK_BASE}"
                )

                list(APPEND KCONFIGS_TO_PARSE "${kconfig_out_file}")
                list(APPEND PARSE_KCONFIGS_COMMAND ${CURRENT_PARSE_COMMAND})

            endforeach()
        endif()
    endmacro()

    add_kconfigs_to_parse(API_KCONFIG_SOURCES)
    add_kconfigs_to_parse(CORE_KCONFIG_SOURCES)
    add_kconfigs_to_parse(SYSTEM_KCONFIG_SOURCES)

    if(PARSE_ALL_SAMPLE_KCONFIGS)
        # Parse Kconfigs for every sample subfolder under samples/
        file(GLOB sample_dirs LIST_DIRECTORIES true "${TELINK_BASE}/samples/*")

        foreach(sample_dir IN LISTS sample_dirs)
            if(IS_DIRECTORY "${sample_dir}")
                get_filename_component(sample_name "${sample_dir}" NAME)

                set(SAMPLE_${sample_name}_KCONFIG_SOURCES
                    "${TELINK_BASE}/samples/${sample_name}"
                    "${GENERATED_KCONFIG_BASE_DIR}/samples/${sample_name}"
                )

                add_kconfigs_to_parse(SAMPLE_${sample_name}_KCONFIG_SOURCES)
            endif()
        endforeach()
    else()
        # Parse Kconfigs only for the currently selected sample
        set(SAMPLE_KCONFIG_SOURCES
            "${TELINK_BASE}/samples/${TLK_SAMPLE_NAME}"
            "${GENERATED_KCONFIG_BASE_DIR}/samples/${TLK_SAMPLE_NAME}"
        )

        add_kconfigs_to_parse(SAMPLE_KCONFIG_SOURCES)
    endif()


    # Custom target for parsing Kconfig files
    add_custom_target(
        parse_kconfig
        DEPENDS
            ${TELINK_BASE}/Kconfig.build
            ${KCONFIGS_TO_PARSE}
        COMMENT "Parsing Kconfig files"
    )

else()
    if(CONFIG_TLK_HAS_BOARD_SELECTED)
        set(GENERATED_KCONFIG_BASE_DIR "${TELINK_BASE}/kconfig/${CONFIG_TLK_SOC_SERIES}/${CONFIG_TLK_BOARD}" CACHE INTERNAL "Directory with generated kconfigs") # Directory with generated kconfigs
    else()
        set(GENERATED_KCONFIG_BASE_DIR "${TELINK_BASE}/kconfig/${CONFIG_TLK_SOC_SERIES}" CACHE INTERNAL "Directory with generated kconfigs") # Directory with generated kconfigs
    endif()

    set(KCONFIG_APPLICATION_DIR "${GENERATED_KCONFIG_BASE_DIR}/samples/${TLK_SAMPLE_NAME}" CACHE INTERNAL "Application directory for Kconfig") # Application directory for Kconfig
endif()


# === Part 2: build.config functionality
set(BUILD_CONFIG_DEPENDS
    ${TELINK_BASE}/Kconfig.build
    ${MENUCONFIG_SCRIPT}
)

if(NOT IS_SDK_MODE)
    list(APPEND BUILD_CONFIG_DEPENDS ${KCONFIGS_TO_PARSE})
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
    VERBATIM
    DEPENDS ${BUILD_CONFIG_DEPENDS}
    USES_TERMINAL
    COMMENT "Generating build.config file via menuconfig GUI"
)


# TODO: FIX!!!
# Check if the build.config is configured for the Cmake Configure Time
if(NOT EXISTS ${BUILD_CONFIG_FILE})
    message(STATUS "build.config is not configured")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${KCONFIG_CACHE_DIR}"
        COMMAND ${PYTHON_EXECUTABLE} ${YAML_CACHE_GEN_SCRIPT}
            ${PROPERTIES_CACHE_FILE}
            ${PROPERTY_YAML_FILES}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE RESULT
    )

    run_commands_separately(PARSE_KCONFIGS_COMMAND "${TELINK_BASE}")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E env
            "KCONFIG_BINARY_DIR=${GENERATED_KCONFIG_BASE_DIR}"
            "KCONFIG_APPLICATION_DIR=${KCONFIG_APPLICATION_DIR}"
            ${PYTHON_EXECUTABLE} -c
                "import kconfiglib, sys; kconf = kconfiglib.Kconfig(sys.argv[1]); kconf.write_config(sys.argv[2])"
                "${TELINK_BASE}/Kconfig.build"
                "${BUILD_CONFIG_FILE}"
        WORKING_DIRECTORY "${TELINK_BASE}"
        RESULT_VARIABLE RESULT
    )
endif()

if(NOT EXISTS ${BUILD_CONFIG_FILE})
    message(FATAL_ERROR "build.config is still not configured")
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

list(APPEND FILES_TO_CLEAN ${CONFIG_FILES} ${AUTOCONF_H})


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


# Main config target that orchestrates all configuration steps
add_custom_target(
    config

    # Generate chip.config (this will start menuconfig)
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target config_chip

    # Generate build.config (this will start menuconfig)
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target config_build

    # Generate autoconf.h by parsing CONFIG_FILES
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target generate_autoconf

    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target update_dotpinmux

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
    USES_TERMINAL
    COMMENT "Run full configuration process with menuconfig"
)
