# Telink Compiler Configuration

include_guard(GLOBAL)

# Set compiler before project() is called
if(NOT DEFINED CMAKE_C_COMPILER)
    if(DEFINED ENV{TELINK_TOOLCHAIN_PATH})
        set(TELINK_TOOLCHAIN_PATH "$ENV{TELINK_TOOLCHAIN_PATH}")
        if(WIN32)
            set(TOOLCHAIN_SUFFIX ".exe")
        else()
            set(TOOLCHAIN_SUFFIX "")
        endif()
        if(EXISTS "${TELINK_TOOLCHAIN_PATH}/bin/riscv32-elf-gcc${TOOLCHAIN_SUFFIX}")
            set(CMAKE_C_COMPILER "${TELINK_TOOLCHAIN_PATH}/bin/riscv32-elf-gcc${TOOLCHAIN_SUFFIX}" CACHE FILEPATH "C compiler" FORCE)
            set(CMAKE_CXX_COMPILER ${TELINK_TOOLCHAIN_PATH}/bin/riscv32-elf-g++${TOOLCHAIN_SUFFIX} CACHE FILEPATH "C++ compiler" FORCE)
            set(CMAKE_ASM_COMPILER ${TELINK_TOOLCHAIN_PATH}/bin/riscv32-elf-gcc${TOOLCHAIN_SUFFIX} CACHE FILEPATH "ASM compiler" FORCE)
            set(CMAKE_OBJDUMP ${TELINK_TOOLCHAIN_PATH}/bin/riscv32-elf-objdump${TOOLCHAIN_SUFFIX} CACHE FILEPATH "Objdump tool" FORCE)
            set(CMAKE_OBJCOPY ${TELINK_TOOLCHAIN_PATH}/bin/riscv32-elf-objcopy${TOOLCHAIN_SUFFIX} CACHE FILEPATH "Objcopy tool" FORCE)
            set(CMAKE_PRINTSIZE ${TELINK_TOOLCHAIN_PATH}/bin/riscv32-elf-size${TOOLCHAIN_SUFFIX} CACHE FILEPATH "Printsize tool" FORCE)
            message(STATUS "Using Telink RISC-V GCC compiler from TELINK_TOOLCHAIN_PATH: ${CMAKE_C_COMPILER}")
        endif()
    endif()
    
    if(NOT DEFINED CMAKE_C_COMPILER)
        message(FATAL_ERROR 
            "No RISC-V GCC compiler found. Please set TELINK_TOOLCHAIN_PATH environment variable\n"
            "to point to your Telink toolchain directory (e.g., D:/TelinkIoTStudio/RDS/V5.3.x/toolchains/nds32le-elf-mculib-v5f)"
        )
    endif()
endif()

# Enable compile_commands.json generation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    
    # Read build settings from JSON
    set(SETTINGS_FILE "${TELINK_BASE}/common/build_settings.json")
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
endfunction()
