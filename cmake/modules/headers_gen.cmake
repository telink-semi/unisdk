# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)
message(STATUS "Setting up headers generation for build time")

set(PROPERTIES_GEN_SCRIPT "${TELINK_BASE}/scripts/generators/properties_gen.py")
set(REGISTERS_GEN_SCRIPT  "${TELINK_BASE}/scripts/generators/registers_gen.py")
set(GENERATED_BASE_DIR    "${CMAKE_BINARY_DIR}/generated")
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
                COMMAND ${PYTHON_EXECUTABLE} "${SCRIPT}" "${yaml_file}" "${header_file}"
                DEPENDS "${yaml_file}" "${SCRIPT}"
                COMMENT "Generating ${header_file}"
                VERBATIM
            )

            list(APPEND HEADERS_TO_GENERATE "${header_file}")
        endforeach()
        endif()
endmacro()

add_headers_to_gen("${PROPERTIES_GEN_SCRIPT}" "${TELINK_BASE}/core/${CONFIG_TLK_CORE}/properties" "${GENERATED_BASE_DIR}/core/${CONFIG_TLK_CORE}/properties")
add_headers_to_gen("${REGISTERS_GEN_SCRIPT}"  "${TELINK_BASE}/core/${CONFIG_TLK_CORE}/registers"  "${GENERATED_BASE_DIR}/core/${CONFIG_TLK_CORE}/registers")

add_headers_to_gen("${PROPERTIES_GEN_SCRIPT}" "${TELINK_BASE}/soc/${CONFIG_TLK_SOC_SERIES}/properties" "${GENERATED_BASE_DIR}/soc/${CONFIG_TLK_SOC_SERIES}/properties")
add_headers_to_gen("${REGISTERS_GEN_SCRIPT}"  "${TELINK_BASE}/soc/${CONFIG_TLK_SOC_SERIES}/registers"  "${GENERATED_BASE_DIR}/soc/${CONFIG_TLK_SOC_SERIES}/registers")

if (CONFIG_TLK_HAS_BOARD_SELECTED)
    add_headers_to_gen("${PROPERTIES_GEN_SCRIPT}" "${TELINK_BASE}/boards/${CONFIG_TLK_BOARD}" "${GENERATED_BASE_DIR}/boards/${CONFIG_TLK_BOARD}")
endif()

add_custom_target(
    generate_headers
    DEPENDS ${HEADERS_TO_GENERATE}
    COMMENT "Generating header files from YAML files"
)