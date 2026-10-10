cmake_minimum_required(VERSION 3.20.0)

set(AUTOCONF_H "${CMAKE_BINARY_DIR}/autoconf.h")
set(PINMUX_H "${CMAKE_BINARY_DIR}/pinmux.h")


# === Additional code ===

# Set TELINK_BASE if not already set
if(NOT DEFINED TELINK_BASE)
    set(TELINK_BASE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Add cmake modules to path
list(APPEND CMAKE_MODULE_PATH ${TELINK_BASE}/cmake/modules)

set(TELINK_ROOT_INCLUDED TRUE)

if(NOT DEFINED TELINK_PACKAGE_INCLUDED)
    # Load Telink default modules
    find_package(Telink REQUIRED HINTS ${TELINK_BASE})
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
# === Additional code ===


# === Main Executable ===

# Executable target settings
add_executable(${TARGET_NAME} ${SOURCES})

set_target_properties(${TARGET_NAME} PROPERTIES
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    OUTPUT_NAME "${TLK_ARTIFACT_NAME}"
)

# Ensure the target depends on preprocess_linker target defined in linker.cmake
add_dependencies(${TARGET_NAME} preprocess_linker)

# Ensure the target depends on generate_git_version target defined in version.cmake
add_dependencies(${TARGET_NAME} generate_git_version)

# Apply compiler and linker options using the functions from respective modules
telink_apply_compiler_options(${TARGET_NAME})
telink_apply_linker_options(${TARGET_NAME})
target_include_directories(${TARGET_NAME} PRIVATE .)

add_dependencies(${TARGET_NAME} generate_autoconf)
add_dependencies(${TARGET_NAME} check_pinmux)

# Add standard include paths
# During development: manually written headers are located here (alongside .yaml files)
# In the exported SDK: all generated headers are copied here, so these paths are sufficient
target_include_directories(${TARGET_NAME} PRIVATE ./core)                                            # Path to root headers (e.g. api/include/*.h)
target_include_directories(${TARGET_NAME} PRIVATE ./packages/${CONFIG_TLK_CORE})                 # Path to core header files from YAML files
target_include_directories(${TARGET_NAME} PRIVATE ./packages/${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES}) # Path to soc series header files from YAML files
target_include_directories(${TARGET_NAME} PRIVATE ./boards/${CONFIG_TLK_BOARD})   # Path to boards header files from YAML files

# Development mode only: include the temporary directory with headers generated from .yaml files
if(NOT IS_SDK_MODE)
    add_dependencies(${TARGET_NAME} generate_headers)

    target_include_directories(${TARGET_NAME} PRIVATE ${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE})      # Path to generated core header files from YAML files
    target_include_directories(${TARGET_NAME} PRIVATE ${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES}) # Path to generated soc series header files from YAML files
    target_include_directories(${TARGET_NAME} PRIVATE ${GENERATED_HEADERS_BASE_DIR}/boards/${CONFIG_TLK_BOARD})   # Path to generated boards header files from YAML files

endif()


# Library target settings

# Base name for all SOC libraries. Must follow the pattern "telink-unisdk-<SOC_SERIES>":
# scripts/tlkpkg/mcu/mcu_export.py collects them by it.
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
if(NOT IS_SDK_MODE)
    telink_libraries_include_directories(PRIVATE .)
    telink_libraries_include_directories(PRIVATE ./core)                                            # Path to root headers (e.g. api/include/*.h)

    telink_libraries_include_directories(PRIVATE ./packages/${CONFIG_TLK_CORE})                 # Path to core header files from YAML files
    telink_libraries_include_directories(PRIVATE ./packages/${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES}) # Path to soc series header files from YAML files
    telink_libraries_include_directories(PRIVATE ./boards/${CONFIG_TLK_BOARD})   # Path to boards header files from YAML files

    telink_libraries_include_directories(PRIVATE ${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE})      # Path to generated core header files from YAML files
    telink_libraries_include_directories(PRIVATE ${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES}) # Path to generated soc series header files from YAML files
    telink_libraries_include_directories(PRIVATE ${GENERATED_HEADERS_BASE_DIR}/boards/${CONFIG_TLK_BOARD})   # Path to generated boards header files from YAML files
endif()

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(packages)
add_subdirectory(system)
add_subdirectory(modules)
add_subdirectory(subsystem)

# TODO: Why here??

# Application-local modules: an out-of-tree application (built via
# find_package(Telink) HINTS $ENV{TELINK_BASE}, see cmake/TelinkConfig.cmake)
# may have its own modules/ directory - e.g. a module installed with
# scripts/tlkpkg/registry_client.py install-module --target-root <app>. Its modules are
# discovered the same way as the SDK's own (modules/Kconfig sources their
# Kconfig), without duplicating when the application *is* this SDK checkout
# (APPLICATION_SOURCE_DIR == TELINK_BASE). An application whose modules/ has
# its own CMakeLists.txt (written by older tlkpkg module installers) keeps
# using that instead.
if(DEFINED APPLICATION_SOURCE_DIR
   AND NOT APPLICATION_SOURCE_DIR STREQUAL TELINK_BASE
   AND IS_DIRECTORY ${APPLICATION_SOURCE_DIR}/modules)
    if(EXISTS ${APPLICATION_SOURCE_DIR}/modules/CMakeLists.txt)
        add_subdirectory(${APPLICATION_SOURCE_DIR}/modules ${CMAKE_BINARY_DIR}/app_modules)
    else()
        telink_add_modules(${APPLICATION_SOURCE_DIR}/modules ${CMAKE_BINARY_DIR}/app_modules)
    endif()
endif()

#! Close the linker group opened above. The resolver now repeatedly
#! scans all archives inside the group until every symbol is resolved.
target_link_libraries(${TARGET_NAME} PRIVATE "-Wl,--end-group")


# Collect the sources from all registered SOC libraries
telink_declare_libraries_target(${TARGET_SOC_LIBRARY})

if(NOT IS_SDK_MODE)
    add_dependencies(build_soc_libraries generate_autoconf) # TODO: FIX and review libs functionality !!!
endif()
