# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)
message(STATUS "Setting up headers generation for build time")

set(PROPERTIES_GEN "properties_gen")
set(REGISTERS_GEN  "registers_gen")
set(GENERATED_BASE_DIR  "${CMAKE_BINARY_DIR}/generated")
set(HEADERS_TO_GENERATE "")

macro(add_headers_to_gen SCRIPT SRC_DIR OUT_DIR)
    if(EXISTS "${SRC_DIR}")
        file(GLOB yaml_files "${SRC_DIR}/*.yaml")

        foreach(yaml_file IN LISTS yaml_files)
            get_filename_component(file_name "${yaml_file}" NAME_WE)
            set(header_file "${OUT_DIR}/tlk_${file_name}.h")

            add_custom_command(
                OUTPUT "${header_file}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}"
                COMMAND ${PYTHON_EXECUTABLE} -m "scripts.generators.${SCRIPT}" "${yaml_file}" "${header_file}"
                DEPENDS "${yaml_file}" "${TELINK_BASE}/scripts/generators/${SCRIPT}.py"
                COMMENT "Generating ${header_file}"
                VERBATIM
                WORKING_DIRECTORY ${TELINK_BASE}
            )

            list(APPEND HEADERS_TO_GENERATE "${header_file}")
        endforeach()
        endif()
endmacro()

set(CORE_PROPERTIES_REL_PATH "core/${CONFIG_TLK_CORE}/properties")
set(CORE_REGISTERS_REL_PATH "core/${CONFIG_TLK_CORE}/registers")
set(SOC_PROPERTIES_REL_PATH "soc/${CONFIG_TLK_SOC_SERIES}/properties")
set(SOC_REGISTERS_REL_PATH "soc/${CONFIG_TLK_SOC_SERIES}/registers")

add_headers_to_gen("${PROPERTIES_GEN}" "${TELINK_BASE}/${CORE_PROPERTIES_REL_PATH}" "${GENERATED_BASE_DIR}/${CORE_PROPERTIES_REL_PATH}")
add_headers_to_gen("${REGISTERS_GEN}"  "${TELINK_BASE}/${CORE_REGISTERS_REL_PATH}"  "${GENERATED_BASE_DIR}/${CORE_REGISTERS_REL_PATH}")

add_headers_to_gen("${PROPERTIES_GEN}" "${TELINK_BASE}/${SOC_PROPERTIES_REL_PATH}"  "${GENERATED_BASE_DIR}/${SOC_PROPERTIES_REL_PATH}")
add_headers_to_gen("${REGISTERS_GEN}"  "${TELINK_BASE}/${SOC_REGISTERS_REL_PATH}"   "${GENERATED_BASE_DIR}/${SOC_REGISTERS_REL_PATH}")

if (CONFIG_TLK_HAS_BOARD_SELECTED)
    set(BOARDS_PROPERTIES_REL_PATH "boards/${CONFIG_TLK_BOARD}")

    add_headers_to_gen("${PROPERTIES_GEN}" "${TELINK_BASE}/${BOARDS_PROPERTIES_REL_PATH}" "${GENERATED_BASE_DIR}/${BOARDS_PROPERTIES_REL_PATH}")
endif()

add_custom_target(
    generate_headers
    DEPENDS ${HEADERS_TO_GENERATE}
    COMMENT "Generating header files from YAML files"
)
