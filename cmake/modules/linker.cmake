# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

include(extensions)
include(python)

# Linker configuration
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_SKIP_RPATH True)
set(CMAKE_LINK_LIBRARY_SUFFIX "")

# Read the settings
set(SETTINGS_FILE "${TELINK_BASE}/common/build_settings.json")
file(READ "${SETTINGS_FILE}" TARGET_JSON)

string(JSON LINKER_OPTS GET ${TARGET_JSON} linker_options)

if(LINKER_OPTS) 
    json_to_list(${LINKER_OPTS} LINKER_OPTS_LIST)
endif()

# Set path to hardware-specific properties "chip" header file.
# Macros from this header are needed for common linker script.

if(CONFIG_TLK_CORE)
    if(NOT IS_DIRECTORY "${TELINK_BASE}/libraries")
        set(CHIP_H ${GENERATED_BASE_DIR}/core/${CONFIG_TLK_CORE}/properties/tlk_chip.h)
    else()
        set(CHIP_H ${TELINK_BASE}/core/${CONFIG_TLK_CORE}/properties/tlk_chip.h)
    endif()

    message(STATUS "Detected Chip Header: ${CHIP_H}")
else()
    message(FATAL_ERROR 
        "CONFIG_TLK_CORE is not defined! \n"
        "This variable is required to set the path for properties/chip.h \n"
        "Please check if it exists in your .config file or if the Kconfig parsing was successful."
    )
endif()

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/flash_boot.ld
    COMMAND ${CMAKE_C_COMPILER} -E -P -x c --imacros=${AUTOCONF_H} --imacros=${CHIP_H} ${TELINK_BASE}/core/common/startup/flash_boot.link -o ${CMAKE_BINARY_DIR}/flash_boot.ld
    DEPENDS ${TELINK_BASE}/core/common/startup/flash_boot.link ${CHIP_H} ${AUTOCONF_H}
    COMMENT "Preprocessing linker script"
)

add_custom_target(preprocess_linker ALL
    DEPENDS ${CMAKE_BINARY_DIR}/flash_boot.ld ${CHIP_H} ${AUTOCONF_H}
)

# Add linker options for output files
list(APPEND LINKER_OPTS_LIST "-${CONFIG_TLK_OPTIMIZATION_LEVEL}")
list(APPEND LINKER_OPTS_LIST "-T${CMAKE_BINARY_DIR}/flash_boot.ld")
list(APPEND LINKER_OPTS_LIST "-Wl,-Map=${CMAKE_BINARY_DIR}/${TARGET_NAME}.map")

# Function to apply linker options to a target
function(telink_apply_linker_options target)
    set_property(TARGET ${target} APPEND PROPERTY
        LINK_DEPENDS ${CMAKE_BINARY_DIR}/flash_boot.ld
    )
    target_link_options(${target} PRIVATE ${LINKER_OPTS_LIST})
    
    # Add post-build commands
    string(JSON OBJ_COPY GET ${TARGET_JSON} obj_copy)
    if(OBJ_COPY)
        json_to_list(${OBJ_COPY} OBJ_COPY_LIST)
        add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_OBJCOPY} ${OBJ_COPY_LIST} ${CMAKE_BINARY_DIR}/${target}.elf ${CMAKE_BINARY_DIR}/${target}.bin)
    endif()

    string(JSON OBJ_DUMP GET ${TARGET_JSON} obj_dump)
    if(OBJ_DUMP)  
        json_to_list(${OBJ_DUMP} OBJ_DUMP_LIST)
        add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_OBJDUMP} ${OBJ_DUMP_LIST} ${CMAKE_BINARY_DIR}/${target}.elf > ${CMAKE_BINARY_DIR}/${target}.lst)
    endif()

    string(JSON PRINT_SIZE GET ${TARGET_JSON} print_size)
    if(PRINT_SIZE)  
        json_to_list(${PRINT_SIZE} PRINT_SIZE_LIST)
        add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_PRINTSIZE} ${PRINT_SIZE_LIST} ${CMAKE_BINARY_DIR}/${target}.elf)
    endif()

    # Add firmware check script
    set(CHECK_FW_SCRIPT_PATH "${TELINK_BASE}/scripts/tl_check_fw.sh")
    if(NOT WIN32)
        file(CHMOD "${CHECK_FW_SCRIPT_PATH}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
    endif()
    add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CHECK_FW_SCRIPT_PATH} ${CMAKE_BINARY_DIR}/${target}.elf ${PROJECT_NAME})
endfunction()

