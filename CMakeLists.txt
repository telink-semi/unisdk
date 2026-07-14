cmake_minimum_required(VERSION 3.20.0)

set(AUTOCONF_H "${CMAKE_BINARY_DIR}/autoconf.h")
set(PINMUX_H "${CMAKE_BINARY_DIR}/pinmux.h")

# Set TELINK_BASE if not already set
if(NOT DEFINED TELINK_BASE)
    set(TELINK_BASE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Add cmake modules to path
list(APPEND CMAKE_MODULE_PATH ${TELINK_BASE}/cmake/modules)

set(TELINK_ROOT_INCLUDED TRUE)

if(NOT DEFINED TELINK_PACKAGE_INCLUDED)
    # Load Telink default modules
    find_package(Telink REQUIRED HINTS $ENV{TELINK_BASE})
endif()


# Set project name
project(${TARGET_NAME} LANGUAGES ASM C CXX)

# Add extensions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(extensions)

# Include the sources
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/common/startup/cstartup_flash.S")

if (CONFIG_TLK_PM_RAM_RETENTION_ENABLE)
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/common/startup/tl_context.S")
endif()


# Override TLK_ARTIFACT_NAME based on Kconfig setting
if(CONFIG_TLK_USE_CUSTOM_ARTIFACT_NAME)
    if(CONFIG_TLK_CUSTOM_ARTIFACT_NAME)
        set(TLK_ARTIFACT_NAME "${CONFIG_TLK_CUSTOM_ARTIFACT_NAME}")
    else()
        get_filename_component(TLK_ARTIFACT_NAME "${APPLICATION_SOURCE_DIR}" NAME)
    endif()
endif()

message(STATUS "Output artifact: ${TLK_ARTIFACT_NAME}")

# Executable target settings
add_executable(${TARGET_NAME} ${SOURCES})

set_target_properties(${TARGET_NAME} PROPERTIES
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    OUTPUT_NAME "${TLK_ARTIFACT_NAME}"
)

# Ensure the target depends on preprocess_linker target defined in linker.cmake
add_dependencies(${TARGET_NAME} preprocess_linker)

# Apply compiler and linker options using the functions from respective modules
telink_apply_compiler_options(${TARGET_NAME})
telink_apply_linker_options(${TARGET_NAME})
target_include_directories(${TARGET_NAME} PRIVATE .)

# Add standard include paths
# During development: manually written headers are located here (alongside .yaml files)
# In the exported SDK: all generated headers are copied here, so these paths are sufficient
target_include_directories(${TARGET_NAME} PRIVATE ./core/${CONFIG_TLK_CORE})      # Path to core header files from YAML files
target_include_directories(${TARGET_NAME} PRIVATE ./soc/${CONFIG_TLK_SOC_SERIES}) # Path to soc series header files from YAML files
target_include_directories(${TARGET_NAME} PRIVATE ./boards/${CONFIG_TLK_BOARD})   # Path to boards header files from YAML files

# Development mode only: include the temporary directory with headers generated from .yaml files
if (NOT IS_DIRECTORY ${TARGET_LIBRARY_DIR})
    add_dependencies(${TARGET_NAME} generate_headers)

    target_include_directories(${TARGET_NAME} PRIVATE ${GENERATED_BASE_DIR}/core/${CONFIG_TLK_CORE})      # Path to generated core header files from YAML files
    target_include_directories(${TARGET_NAME} PRIVATE ${GENERATED_BASE_DIR}/soc/${CONFIG_TLK_SOC_SERIES}) # Path to generated soc series header files from YAML files
    target_include_directories(${TARGET_NAME} PRIVATE ${GENERATED_BASE_DIR}/boards/${CONFIG_TLK_BOARD})   # Path to generated boards header files from YAML files

endif()


# Library target settings

# Base name for all SOC libraries. Must follow the pattern "telink-unisdk-<SOC_SERIES>".
#
# sdk_exporter.sh relies on this pattern to copy all SOC libraries at once:
#   cp build/libraries/libtelink-unisdk-${soc_series}*.a ...
if(CONFIG_TLK_TOOLCHAIN_TYPE STREQUAL "zephyr")
    set(TARGET_SOC_LIBRARY "telink-unisdk-${CONFIG_TLK_SOC_SERIES}-zephyr")
else()
    set(TARGET_SOC_LIBRARY "telink-unisdk-${CONFIG_TLK_SOC_SERIES}")
endif()

#! Wrap the interdependent static libraries in a linker group so the
#! single-pass BFD linker can resolve backward references between them
#! (e.g. BLE HAL → RF/SKE symbols provided by the SOC libraries).
#! The group is closed after the subsystem (BLE) libraries are linked.
target_link_libraries(${TARGET_NAME} PRIVATE "-Wl,--start-group")

#! Main SOC lib last — provides symbols for all driver libs above
telink_add_library(${TARGET_SOC_LIBRARY})

# Set the include directories to the libraries
if (NOT IS_DIRECTORY ${TARGET_LIBRARY_DIR})
    telink_libraries_include_directories(PRIVATE .)

    telink_libraries_include_directories(PRIVATE ./core/${CONFIG_TLK_CORE})      # Path to core header files from YAML files
    telink_libraries_include_directories(PRIVATE ./soc/${CONFIG_TLK_SOC_SERIES}) # Path to soc series header files from YAML files
    telink_libraries_include_directories(PRIVATE ./boards/${CONFIG_TLK_BOARD})   # Path to boards header files from YAML files

    telink_libraries_include_directories(PRIVATE ${GENERATED_BASE_DIR}/core/${CONFIG_TLK_CORE})      # Path to generated core header files from YAML files
    telink_libraries_include_directories(PRIVATE ${GENERATED_BASE_DIR}/soc/${CONFIG_TLK_SOC_SERIES}) # Path to generated soc series header files from YAML files
    telink_libraries_include_directories(PRIVATE ${GENERATED_BASE_DIR}/boards/${CONFIG_TLK_BOARD})   # Path to generated boards header files from YAML files
endif()

add_subdirectory(api)
add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(soc)
add_subdirectory(system)
add_subdirectory(modules)
add_subdirectory(subsystem)

#! Close the linker group opened above. The resolver now repeatedly
#! scans all archives inside the group until every symbol is resolved.
target_link_libraries(${TARGET_NAME} PRIVATE "-Wl,--end-group")


# Collect the sources from all registered SOC libraries
telink_declare_libraries_target(${TARGET_SOC_LIBRARY})
