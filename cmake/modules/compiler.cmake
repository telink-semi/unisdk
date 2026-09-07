# Telink Compiler Configuration

include_guard(GLOBAL)

# Priority 0: Detect stale cache when SoC series or toolchain type changed.
# This runs on EVERY cmake invocation, including re-configures triggered by
# `make config`. Without this, switching SoC via Kconfig menuconfig would
# silently use the old toolchain, causing binary corruption.
#
# When the SoC series or toolchain type has changed, unset the
# TLK_TOOLCHAIN_* cached guard variables so that toolchain discovery
# re-runs with the correct settings in this CMake invocation
# CMake's cache conflict detection.
if((DEFINED TLK_TOOLCHAIN_SOC_SERIES_CACHED
        AND DEFINED CONFIG_TLK_SOC_SERIES
        AND NOT "${TLK_TOOLCHAIN_SOC_SERIES_CACHED}" STREQUAL "${CONFIG_TLK_SOC_SERIES}")
        OR
   (DEFINED TLK_TOOLCHAIN_TYPE_CACHED
        AND DEFINED CONFIG_TLK_TOOLCHAIN_TYPE
        AND NOT "${TLK_TOOLCHAIN_TYPE_CACHED}" STREQUAL "${CONFIG_TLK_TOOLCHAIN_TYPE}"))
    unset(TLK_TOOLCHAIN_DISCOVERED CACHE)
    unset(TLK_TOOLCHAIN_SOC_SERIES_CACHED CACHE)
    unset(TLK_TOOLCHAIN_TYPE_CACHED CACHE)
    unset(TLK_TOOLCHAIN_PATH_CACHED CACHE)
    unset(TLK_TOOLCHAIN_PREFIX_CACHED CACHE)
endif()
include(extensions)

# Set compiler before project() is called
# Use a custom guard variable (TLK_TOOLCHAIN_DISCOVERED) instead of
# checking CMAKE_C_COMPILER, so that Priority 0 can trigger re-discovery
# by simply unsetting the guard without touching CMake internal variables.
if(NOT DEFINED TLK_TOOLCHAIN_DISCOVERED)
    # Step 0: Determine toolchain type (default: andes)
    if(NOT DEFINED CONFIG_TLK_TOOLCHAIN_TYPE)
        set(CONFIG_TLK_TOOLCHAIN_TYPE "andes")
    endif()
    message(STATUS "Toolchain type: ${CONFIG_TLK_TOOLCHAIN_TYPE}")

    # Step 1: Determine TELINK_TOOLCHAIN_PATH
    # Priority 1: Cached toolchain path (skip Python if SOC series and type match)
    if(DEFINED TLK_TOOLCHAIN_SOC_SERIES_CACHED AND DEFINED CONFIG_TLK_SOC_SERIES
            AND DEFINED TLK_TOOLCHAIN_TYPE_CACHED
            AND "${TLK_TOOLCHAIN_SOC_SERIES_CACHED}" STREQUAL "${CONFIG_TLK_SOC_SERIES}"
            AND "${TLK_TOOLCHAIN_TYPE_CACHED}" STREQUAL "${CONFIG_TLK_TOOLCHAIN_TYPE}")
        set(TELINK_TOOLCHAIN_PATH "${TLK_TOOLCHAIN_PATH_CACHED}")
        set(TLK_TOOLCHAIN_PREFIX "${TLK_TOOLCHAIN_PREFIX_CACHED}")
        set(TLK_TOOLCHAIN_EXTRA_FLAGS "${TLK_TOOLCHAIN_EXTRA_FLAGS_CACHED}")
        message(STATUS "Using cached toolchain path for ${CONFIG_TLK_SOC_SERIES} (${CONFIG_TLK_TOOLCHAIN_TYPE}): ${TELINK_TOOLCHAIN_PATH}")
    # Priority 2: Auto-discovery via Python script
    else()
        if(NOT DEFINED CONFIG_TLK_SOC_SERIES)
            message(FATAL_ERROR "CONFIG_TLK_SOC_SERIES is not defined. Kconfig may not have run.")
        endif()
        message(STATUS "Discovering toolchain for SoC series: ${CONFIG_TLK_SOC_SERIES} (type: ${CONFIG_TLK_TOOLCHAIN_TYPE})")

        set(TOOLCHAIN_DISCOVERY_SCRIPT "${TELINK_BASE}/scripts/toolchain_discovery.py")
        if(NOT EXISTS "${TOOLCHAIN_DISCOVERY_SCRIPT}")
            message(FATAL_ERROR "Toolchain discovery script not found: ${TOOLCHAIN_DISCOVERY_SCRIPT}")
        endif()

        if(DEFINED ENV{TELINK_TOOLCHAIN_PATH})
            set(_TLK_TOOLCHAIN_ROOT_ARGS "--toolchain-root" "$ENV{TELINK_TOOLCHAIN_PATH}")
        endif()

        execute_process(
            COMMAND ${PYTHON_EXECUTABLE}
                    "${TOOLCHAIN_DISCOVERY_SCRIPT}"
                    --soc-series ${CONFIG_TLK_SOC_SERIES}
                    --toolchain-type ${CONFIG_TLK_TOOLCHAIN_TYPE}
                    --output json
                    ${_TLK_TOOLCHAIN_ROOT_ARGS}
            OUTPUT_VARIABLE TOOLCHAIN_JSON
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_VARIABLE TOOLCHAIN_ERROR
            RESULT_VARIABLE TOOLCHAIN_RESULT
        )

        if(NOT TOOLCHAIN_RESULT EQUAL 0)
            message(FATAL_ERROR
                "Toolchain discovery failed: ${TOOLCHAIN_ERROR}\n"
                "Please install the appropriate toolchain for ${CONFIG_TLK_SOC_SERIES} (type: ${CONFIG_TLK_TOOLCHAIN_TYPE}) and set "
                "the TELINK_TOOLCHAIN_PATH environment variable."
            )
        endif()

        # Parse JSON output to extract toolchain_path and prefix
        string(JSON TELINK_TOOLCHAIN_PATH GET "${TOOLCHAIN_JSON}" "toolchain_path")
        string(JSON TLK_TOOLCHAIN_PREFIX GET "${TOOLCHAIN_JSON}" "prefix")
        message(STATUS "Discovered toolchain path: ${TELINK_TOOLCHAIN_PATH} (prefix: ${TLK_TOOLCHAIN_PREFIX})")

        # Some toolchain builds (e.g. the hardware-FPU "v5f" variant) need extra
        # -march/-mabi flags that don't match the SoC-generic -mcpu default, so
        # that the correct multilib (matching libgcc/libc) is selected. Read
        # them from soc.yaml via the discovery script's JSON output.
        set(TLK_TOOLCHAIN_EXTRA_FLAGS "")
        string(JSON TOOLCHAIN_EXTRA_FLAGS_JSON ERROR_VARIABLE _extra_flags_err GET "${TOOLCHAIN_JSON}" "extra_flags")
        if(NOT _extra_flags_err)
            json_to_list(${TOOLCHAIN_EXTRA_FLAGS_JSON} TLK_TOOLCHAIN_EXTRA_FLAGS)
        endif()
    endif()

    # Step 2: Verify the toolchain path and set all compiler/tool variables
    if(WIN32)
        set(TOOLCHAIN_SUFFIX ".exe")
    else()
        set(TOOLCHAIN_SUFFIX "")
    endif()

    if(EXISTS "${TELINK_TOOLCHAIN_PATH}/bin/${TLK_TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_SUFFIX}")
        set(CMAKE_C_COMPILER "${TELINK_TOOLCHAIN_PATH}/bin/${TLK_TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_SUFFIX}" CACHE FILEPATH "C compiler" FORCE)
        set(CMAKE_CXX_COMPILER ${TELINK_TOOLCHAIN_PATH}/bin/${TLK_TOOLCHAIN_PREFIX}g++${TOOLCHAIN_SUFFIX} CACHE FILEPATH "C++ compiler" FORCE)
        set(CMAKE_ASM_COMPILER ${TELINK_TOOLCHAIN_PATH}/bin/${TLK_TOOLCHAIN_PREFIX}gcc${TOOLCHAIN_SUFFIX} CACHE FILEPATH "ASM compiler" FORCE)
        set(CMAKE_OBJDUMP ${TELINK_TOOLCHAIN_PATH}/bin/${TLK_TOOLCHAIN_PREFIX}objdump${TOOLCHAIN_SUFFIX} CACHE FILEPATH "Objdump tool" FORCE)
        set(CMAKE_OBJCOPY ${TELINK_TOOLCHAIN_PATH}/bin/${TLK_TOOLCHAIN_PREFIX}objcopy${TOOLCHAIN_SUFFIX} CACHE FILEPATH "Objcopy tool" FORCE)
        set(CMAKE_PRINTSIZE ${TELINK_TOOLCHAIN_PATH}/bin/${TLK_TOOLCHAIN_PREFIX}size${TOOLCHAIN_SUFFIX} CACHE FILEPATH "Printsize tool" FORCE)
        message(STATUS "Using Telink RISC-V GCC compiler from: ${CMAKE_C_COMPILER}")
    else()
        message(FATAL_ERROR
            "Toolchain path '${TELINK_TOOLCHAIN_PATH}/bin' does not contain ${TLK_TOOLCHAIN_PREFIX}gcc.\n"
            "Please check your toolchain installation."
        )
    endif()

    # Step 3: Cache the discovered path into CMakeCache
    # This saves re-running Python discovery on subsequent cmake re-configures
    # as long as the SoC series and toolchain type have not changed.
    #
    # SOC series and toolchain type are cached unconditionally (even when
    # TELINK_TOOLCHAIN_PATH is set via env var) so that Priority 0 can
    # detect SOC changes on subsequent cmake re-configures.
    set(TLK_TOOLCHAIN_SOC_SERIES_CACHED "${CONFIG_TLK_SOC_SERIES}" CACHE STRING "SoC series for cached toolchain path" FORCE)
    set(TLK_TOOLCHAIN_TYPE_CACHED "${CONFIG_TLK_TOOLCHAIN_TYPE}" CACHE STRING "Toolchain type for cached toolchain path" FORCE)
    if(NOT DEFINED ENV{TELINK_TOOLCHAIN_PATH})
        set(TLK_TOOLCHAIN_PATH_CACHED "${TELINK_TOOLCHAIN_PATH}" CACHE PATH "Discovered Telink toolchain path" FORCE)
        set(TLK_TOOLCHAIN_PREFIX_CACHED "${TLK_TOOLCHAIN_PREFIX}" CACHE STRING "Toolchain prefix for cached toolchain path" FORCE)
        message(STATUS "Cached toolchain path for ${CONFIG_TLK_SOC_SERIES} (${CONFIG_TLK_TOOLCHAIN_TYPE}): ${TELINK_TOOLCHAIN_PATH}")
    endif()


    # Always persist extra_flags so they remain available on later reconfigures.
    set(TLK_TOOLCHAIN_EXTRA_FLAGS_CACHED
        "${TLK_TOOLCHAIN_EXTRA_FLAGS}"
        CACHE STRING "Extra compiler/linker flags for cached toolchain path" FORCE
    )

    # Mark toolchain as discovered so subsequent cmake re-configures
    # skip re-discovery as long as the SOC hasn't changed.
    set(TLK_TOOLCHAIN_DISCOVERED TRUE CACHE INTERNAL "Toolchain has been discovered" FORCE)
endif()

# Restore extra_flags from cache on reconfigures that skip toolchain discovery.
if(NOT DEFINED TLK_TOOLCHAIN_EXTRA_FLAGS AND DEFINED TLK_TOOLCHAIN_EXTRA_FLAGS_CACHED)
    set(TLK_TOOLCHAIN_EXTRA_FLAGS "${TLK_TOOLCHAIN_EXTRA_FLAGS_CACHED}")
endif()

# Enable compile_commands.json generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Necessary for the clangd
set(CC_LINK   "${TELINK_BASE}/compile_commands.json")
set(CC_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")

add_custom_command(
    OUTPUT ${CC_LINK}
    DEPENDS ${CC_SOURCE}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CC_SOURCE} ${CC_LINK}
    VERBATIM
)

add_custom_target(copy_compile_commands ALL
    DEPENDS ${CC_LINK}
)

# Set RISC-V target for Telink chips
set(CMAKE_SYSTEM_PROCESSOR riscv32)
set(CMAKE_SYSTEM_NAME Generic)

# Compiler flags configuration
set(CMAKE_C_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_RELEASE "")

# Disable Windows-specific library linking
set(CMAKE_C_STANDARD_LIBRARIES_INIT "")
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "")

# Function to apply compiler options to a target
function(telink_apply_compiler_options target)
    # Apply macro includes
    target_compile_options(${target} PRIVATE
        "--imacros=${AUTOCONF_H}"
        "--imacros=${PINMUX_H}"
        "--imacros=${GIT_VERSION_H}"

        "-${CONFIG_TLK_OPTIMIZATION_LEVEL}"
    )

    # C++-specific compiler options for embedded systems
    target_compile_options(${target} PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:
            -fno-exceptions
            -fno-rtti
            -Os
            -Wall
            -Wextra
        >
    )

    # Read build settings from JSON (select file based on toolchain type)
    if(CONFIG_TLK_TOOLCHAIN_TYPE STREQUAL "zephyr")
        set(SETTINGS_FILE "${TELINK_BASE}/common/build_settings_zephyr.json")
    else()
        set(SETTINGS_FILE "${TELINK_BASE}/common/build_settings.json")
    endif()
    file(READ "${SETTINGS_FILE}" TARGET_JSON)

    # Apply ASM options
    string(JSON ASM_OPTS GET ${TARGET_JSON} asm_compile_options)
    if(ASM_OPTS)
        json_to_list(${ASM_OPTS} ASM_OPTS_LIST ASM_OPTS_STR)
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:ASM>: ${ASM_OPTS_LIST}>)
    endif()

    # Apply C options
    string(JSON C_OPTS GET ${TARGET_JSON} c_compile_options)
    if(C_OPTS)
        json_to_list(${C_OPTS} C_OPTS_LIST C_OPTS_STR)
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:C>: ${C_OPTS_LIST}>)
    endif()

    # Apply C++ options
    string(JSON CPP_OPTS GET ${TARGET_JSON} cxx_compile_options)
    if(CPP_OPTS)
        json_to_list(${CPP_OPTS} CPP_OPTS_LIST CPP_OPTS_STR)
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>: ${CPP_OPTS_LIST}>)
    endif()

    # Apply toolchain-specific extra flags (e.g. -march/-mabi for the hardware-FPU
    # toolchain variant) last, so they override the ABI implied by -mcpu above.
    if(TLK_TOOLCHAIN_EXTRA_FLAGS)
        target_compile_options(${target} PRIVATE ${TLK_TOOLCHAIN_EXTRA_FLAGS})
    endif()
endfunction()
