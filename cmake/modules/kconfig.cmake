# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

include(extensions)
include(python)

# 1. Define core file paths
set(CHIP_CONFIG_FILE ${CMAKE_BINARY_DIR}/chip.config)          # Configuration file
set(BUILD_CONFIG_FILE ${CMAKE_BINARY_DIR}/build.config)        # Configuration file
set(DOTCONFIG ${CMAKE_BINARY_DIR}/.config)                     # Configuration file (merged from chip.config and build.config)
set(AUTOCONF_H ${CMAKE_BINARY_DIR}/autoconf.h)                 # Precompiled header file generated after parsing
set(MENUCONFIG_SCRIPT ${TELINK_BASE}/scripts/menuconfig.py)    # Menuconfig script
set(PARSE_SCRIPT ${TELINK_BASE}/scripts/parse_config.py)       # Parsing script
set(PREPROCESS_SCRIPT ${TELINK_BASE}/scripts/preprocess_kconfig.py)  # Preprocessing script
set(KCONFIG_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/Kconfig)    # Kconfig binary directory for source includes
set(KCONFIG_APPLICATION_DIR ${CMAKE_CURRENT_SOURCE_DIR})       # Application directory for Kconfig

# Collect all files that should be cleaned automatically
set(KCONFIG_CLEAN_FILES
    ${CHIP_CONFIG_FILE}
    ${BUILD_CONFIG_FILE}
    ${DOTCONFIG}
    ${AUTOCONF_H}
    ${PINMUX_H}
    ${KCONFIG_BINARY_DIR}
)

# Process SOC and BOARD parameters if provided
if(DEFINED SOC)
  message(STATUS "SOC parameter detected: ${SOC}")
  set(SOC_CONFIG_ARG "SOC=${SOC}")
else()
  set(SOC_CONFIG_ARG "")
endif()

if(DEFINED BOARD)
  message(STATUS "BOARD parameter detected: ${BOARD}")
  set(BOARD_CONFIG_ARG "BOARD=${BOARD}")
else()
  set(BOARD_CONFIG_ARG "")
endif()

# Validate SOC and BOARD combination using dynamic Kconfig parsing
if(SOC_CONFIG_ARG AND BOARD_CONFIG_ARG)
  message(STATUS "Validating SOC and BOARD combination...")
  
  # Extract SOC and BOARD names from arguments
  string(REGEX REPLACE "SOC=" "" SOC_NAME "${SOC_CONFIG_ARG}")
  string(REGEX REPLACE "BOARD=" "" BOARD_NAME "${BOARD_CONFIG_ARG}")
  
  # Run validation using our kconfig_parser
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} 
            ${TELINK_BASE}/scripts/kconfig_parser.py
            --kconfig ${TELINK_BASE}/Kconfig.chip
            --validate ${SOC_NAME} ${BOARD_NAME}
    WORKING_DIRECTORY ${TELINK_BASE}
    RESULT_VARIABLE VALIDATION_RESULT
    OUTPUT_VARIABLE VALIDATION_OUTPUT
    ERROR_VARIABLE VALIDATION_ERROR
  )
  
  if(NOT VALIDATION_RESULT EQUAL 0)
    message(FATAL_ERROR "SOC/BOARD validation failed: ${VALIDATION_ERROR}")
  else()
    message(STATUS "SOC/BOARD validation passed: ${VALIDATION_OUTPUT}")
  endif()
endif()

# Set additional clean files for the directory in one call
set_directory_properties(
    PROPERTIES ADDITIONAL_CLEAN_FILES "${KCONFIG_CLEAN_FILES}"
)

# First, ensure .config file exists and is generated during configuration phase if missing
if(NOT EXISTS ${DOTCONFIG})
  message(STATUS "Auto-generating configuration files...")

  # Create .config directly during configuration phase if it doesn't exist
  if(NOT EXISTS ${CHIP_CONFIG_FILE})
    # Apply SOC and BOARD configurations using Python setter if provided
    if(SOC_CONFIG_ARG OR BOARD_CONFIG_ARG)
      message(STATUS "Applying SOC/BOARD configurations using Python setter...")
      
      # Build command arguments
      set(SETTER_ARGS "")
      if(SOC_CONFIG_ARG)
        string(REGEX REPLACE "SOC=" "" SOC_NAME "${SOC_CONFIG_ARG}")
        list(APPEND SETTER_ARGS "--soc" "${SOC_NAME}")
      endif()
      if(BOARD_CONFIG_ARG)
        string(REGEX REPLACE "BOARD=" "" BOARD_NAME "${BOARD_CONFIG_ARG}")
        list(APPEND SETTER_ARGS "--board" "${BOARD_NAME}")
      endif()
      
      execute_process(
        COMMAND ${PYTHON_EXECUTABLE} 
                ${TELINK_BASE}/scripts/soc_board_setter.py
                --kconfig ${TELINK_BASE}/Kconfig.chip
                --telink-base ${TELINK_BASE}
                --config-file ${CHIP_CONFIG_FILE}
                ${SETTER_ARGS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE SETTER_RESULT
        OUTPUT_VARIABLE SETTER_OUTPUT
        ERROR_VARIABLE SETTER_ERROR
      )
      
      if(NOT SETTER_RESULT EQUAL 0)
        message(FATAL_ERROR "SOC/BOARD setter failed: ${SETTER_ERROR}")
      else()
        message(STATUS "SOC/BOARD setter applied successfully: ${SETTER_OUTPUT}")
      endif()
    else()
      execute_process(
        COMMAND ${CMAKE_COMMAND} -E env KCONFIG_BINARY_DIR=${KCONFIG_BINARY_DIR} KCONFIG_APPLICATION_DIR=${KCONFIG_APPLICATION_DIR} ${PYTHON_EXECUTABLE}
        ${TELINK_BASE}/scripts/kconfig.py
        --telink-base=${TELINK_BASE}
        ${TELINK_BASE}/Kconfig.chip
        ${CHIP_CONFIG_FILE}
        WORKING_DIRECTORY ${TELINK_BASE}
        )
    endif()
  endif()
  
  # Updating the Kconfig files, that heed to be parsed
  if(NOT IS_DIRECTORY "${TELINK_BASE}/libraries")
    message(STATUS "DEV SRC: Running Kconfig preprocessor")
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE}
        ${PREPROCESS_SCRIPT}
        --root-dir=${TELINK_BASE}
        --build-dir=${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  else()
    message(STATUS "SDK: Copying pre-processed Kconfig files")
    
    file(STRINGS "${CHIP_CONFIG_FILE}" _soc_line REGEX "^CONFIG_TLK_SOC_SERIES=")
    string(REGEX REPLACE ".*CONFIG_TLK_SOC_SERIES=\"?([A-Za-z0-9_]+)\"?.*" "\\1" _soc_series "${_soc_line}")

    if(_soc_series)
      set(SOC_PREPROCESSED_KCONFIG_DIR "${TELINK_BASE}/kconfig/${_soc_series}")
      
      file(COPY "${SOC_PREPROCESSED_KCONFIG_DIR}/" DESTINATION "${KCONFIG_BINARY_DIR}")
      message(STATUS "SDK: Kconfig files copied for SOC series: ${_soc_series}")
    
    else()
      message(FATAL_ERROR "SDK: Could not determine CONFIG_TLK_SOC_SERIES from ${CHIP_CONFIG_FILE}")
    endif()
  endif()

  # Create build.config directly during configuration phase if it doesn't exist
  if(NOT EXISTS ${BUILD_CONFIG_FILE})
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E env KCONFIG_BINARY_DIR=${KCONFIG_BINARY_DIR} KCONFIG_APPLICATION_DIR=${KCONFIG_APPLICATION_DIR} ${PYTHON_EXECUTABLE}
      ${TELINK_BASE}/scripts/kconfig.py
      --telink-base=${TELINK_BASE}
      ${TELINK_BASE}/Kconfig.build
      ${BUILD_CONFIG_FILE}
      WORKING_DIRECTORY ${TELINK_BASE}
      )
  endif()

  # Create .config directly during configuration phase if it doesn't exist
  file(WRITE ${DOTCONFIG} "# Auto-generated .config file during CMake configuration\n")
  # Append content from build.config and chip.config if they exist
  file(READ ${CHIP_CONFIG_FILE} CHIP_CONFIG_CONTENT)
  file(APPEND ${DOTCONFIG} "${CHIP_CONFIG_CONTENT}")
  file(APPEND ${DOTCONFIG} "\n")
  file(READ ${BUILD_CONFIG_FILE} BUILD_CONFIG_CONTENT)
  file(APPEND ${DOTCONFIG} "${BUILD_CONFIG_CONTENT}")
endif()

# Ensure CMake detects changes in configuration files and reconfigures
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${DOTCONFIG})



# Create a custom target for generating chip.config
add_custom_target(
  generate_chip_config
  COMMAND ${CMAKE_COMMAND} -E env KCONFIG_CONFIG=${CHIP_CONFIG_FILE} KCONFIG_BINARY_DIR=${KCONFIG_BINARY_DIR} KCONFIG_APPLICATION_DIR=${KCONFIG_APPLICATION_DIR} ${PYTHON_EXECUTABLE} ${MENUCONFIG_SCRIPT} ${TELINK_BASE}/Kconfig.chip
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
  DEPENDS ${MENUCONFIG_SCRIPT} ${TELINK_BASE}/Kconfig.chip
  USES_TERMINAL
  COMMENT "Generate chip.config file"
)

if(NOT IS_DIRECTORY "${TELINK_BASE}/libraries")
  # Parse the Kconfig files, using yaml dict from the selected hardware, via the preprocess script
  add_custom_target(
    prepare_kconfig_files
    COMMAND ${PYTHON_EXECUTABLE} ${PREPROCESS_SCRIPT} 
            --root-dir=${TELINK_BASE} 
            --build-dir=${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
    DEPENDS ${CHIP_CONFIG_FILE}
    USES_TERMINAL
    COMMENT "DEV SRC: Running Kconfig preprocessor\n"
  )
else()
  # Reading the soc series in the chip.config
  file(STRINGS "${CHIP_CONFIG_FILE}" _soc_line REGEX "^CONFIG_TLK_SOC_SERIES=")
  string(REGEX REPLACE ".*CONFIG_TLK_SOC_SERIES=\"?([A-Za-z0-9_]+)\"?.*" "\\1" _soc_series "${_soc_line}")

  message(STATUS "SDK: SOC for which kconfigs will be copied: ${_soc_series}")

  # Path to the folder with pre-processed configs in exported SDK, for selected hardware
  set(SOC_PREPROCESSED_KCONFIG_DIR "${TELINK_BASE}/kconfig/${_soc_series}")

  # Copy existing pre-processed configs in exported SDK, for selected hardware into the KCONFIG_BINARY_DIR
  add_custom_target(
    prepare_kconfig_files
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${KCONFIG_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${SOC_PREPROCESSED_KCONFIG_DIR}" "${KCONFIG_BINARY_DIR}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
    DEPENDS ${CHIP_CONFIG_FILE}
    COMMENT "SDK: Copying pre-processed Kconfig files"
  )
endif()

# Create a custom target for generating build.config
add_custom_target(
  generate_build_config
  COMMAND ${CMAKE_COMMAND} -E env KCONFIG_CONFIG=${BUILD_CONFIG_FILE} KCONFIG_BINARY_DIR=${KCONFIG_BINARY_DIR} KCONFIG_APPLICATION_DIR=${KCONFIG_APPLICATION_DIR} ${PYTHON_EXECUTABLE} ${MENUCONFIG_SCRIPT} ${TELINK_BASE}/Kconfig.build
  WORKING_DIRECTORY ${TELINK_BASE}
  VERBATIM
  DEPENDS ${MENUCONFIG_SCRIPT} ${TELINK_BASE}/Kconfig.build
  USES_TERMINAL
  COMMENT "Generate build.config file"
)

# Create a custom target for updating .config
add_custom_target(
  update_dotconfig
  COMMAND ${CMAKE_COMMAND} -E remove -f ${DOTCONFIG}
  COMMAND ${CMAKE_COMMAND} -E echo "# Auto-generated by CMake" > ${DOTCONFIG}
  COMMAND ${CMAKE_COMMAND} -E cat ${BUILD_CONFIG_FILE} >> ${DOTCONFIG}
  COMMAND ${CMAKE_COMMAND} -E echo "" >> ${DOTCONFIG}
  COMMAND ${CMAKE_COMMAND} -E cat ${CHIP_CONFIG_FILE} >> ${DOTCONFIG}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
  DEPENDS ${BUILD_CONFIG_FILE} ${CHIP_CONFIG_FILE}
  COMMENT "Update .config file using cross-platform CMake commands"
)

# Create the parse_config target that depends on autoconf.h
add_custom_target(
  parse_config
  DEPENDS ${DOTCONFIG}  # Depends on .config and parsed header to ensure it's generated
  COMMENT "Completed .config parsing"
)

# Parse .config during build, generate precompiled header (executed before main program compilation)
file(STRINGS ${CMAKE_BINARY_DIR}/.config CONFIG_LINES)

file(WRITE "${AUTOCONF_H}" "/* Auto-generated from .config */\n")
foreach(line IN LISTS CONFIG_LINES)
    string(REGEX MATCH "^CONFIG_([A-Za-z0-9_]+)=" matched "${line}")
    if(matched)
        string(REGEX REPLACE "=" ";" parts "${line}")
        list(GET parts 0 name)
        list(GET parts 1 value)

        # Handle boolean (y/n) as 1/0
        if(value STREQUAL "y")
            set(${name} 1)
            file(APPEND "${AUTOCONF_H}" "#define ${name} 1\n")

        elseif(value STREQUAL "n")
            set(${name} 0)
            file(APPEND "${AUTOCONF_H}" "#define ${name} 0\n")

        # String with quotes: CONFIG_NAME="Value"
        elseif(value MATCHES "^\".*\"$")
            # Remove quotes to get bare strings
            string(REGEX REPLACE "^\"(.*)\"$" "\\1" value_stripped "${value}")
            set(${name} "${value_stripped}")
            # Pass as a define like CONFIG_NAME ABC
            file(APPEND "${AUTOCONF_H}" "#define ${name} ${value_stripped}\n")

        # Numbers or other values
        else()
            set(${name} ${value})
            file(APPEND "${AUTOCONF_H}" "#define ${name} ${value}\n")

        endif()
    endif()
endforeach()

# Print the current hardware
message(STATUS "Using ${CONFIG_TLK_CORE} core")
message(STATUS "Using ${CONFIG_TLK_SOC_SERIES} soc series")
message(STATUS "Using ${CONFIG_TLK_SOC} soc")
if (CONFIG_TLK_HAS_BOARD_SELECTED)
    message(STATUS "Using ${CONFIG_TLK_BOARD} board")
else()
    message(STATUS "Board is not selected")
endif()


# Main config target that orchestrates all configuration steps
add_custom_target(
    config
    # Generate chip.config (this will start menuconfig)
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target generate_chip_config

    # Reconfigure CMake to detect changes
    COMMAND ${CMAKE_COMMAND} ${CMAKE_BINARY_DIR}

    # For dev SDK: Parse the Kconfig files, using yaml dict from the selected hardware, via the preprocess script
    # For exported SDK: Copy existing pre-processed configs in exported SDK, for selected hardware into the KCONFIG_BINARY_DIR
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target prepare_kconfig_files
    
    # Generate build.config (this will start menuconfig)
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target generate_build_config
    
    # Update .config by merging build.config and chip.config
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target update_dotconfig
    
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target update_dotpinmux
    
    # Reconfigure CMake to detect changes
    COMMAND ${CMAKE_COMMAND} ${CMAKE_BINARY_DIR}
    
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
    DEPENDS ${MENUCONFIG_SCRIPT} ${TELINK_BASE}/Kconfig.chip ${TELINK_BASE}/Kconfig.build
    USES_TERMINAL
    COMMENT "Run full configuration process with menuconfig"
)