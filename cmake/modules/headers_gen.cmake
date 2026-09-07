# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

# Create a list of all headers that have to be generated for selected CORE, SOC, BOARD
set(HEADERS_TO_GENERATE "")

macro(add_headers_to_gen SOURCES)
    list(GET ${SOURCES} 0 src_dir)
    list(GET ${SOURCES} 1 out_dir)
    list(GET ${SOURCES} 2 script)

    string(REPLACE "." "/" script_path "${script}")

    if(EXISTS "${src_dir}")
        file(GLOB yaml_files "${src_dir}/*.yaml")

        foreach(yaml_file IN LISTS yaml_files)
            get_filename_component(file_name "${yaml_file}" NAME_WE)
            set(header_file "${out_dir}/tlk_${file_name}.h")

            add_custom_command(
                OUTPUT "${header_file}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${out_dir}"
                COMMAND ${PYTHON_EXECUTABLE} -m "${script}" "${yaml_file}" "${header_file}"
                DEPENDS
                    "${yaml_file}"
                    "${TELINK_BASE}/${script_path}.py"
                COMMENT "Generating ${header_file}"
                VERBATIM
                WORKING_DIRECTORY "${TELINK_BASE}"
            )

            list(APPEND HEADERS_TO_GENERATE "${header_file}")
        endforeach()
    endif()
endmacro()

add_headers_to_gen(CORE_PROPERTY_SOURCES)
add_headers_to_gen(SOC_PROPERTY_SOURCES)
add_headers_to_gen(CORE_REGISTER_SOURCES)
add_headers_to_gen(SOC_REGISTER_SOURCES)

if(CONFIG_TLK_HAS_BOARD_SELECTED)
    add_headers_to_gen(BOARD_PROPERTY_SOURCES)
endif()


# Custom target which depends on all headers that have to be generated for selected CORE, SOC, BOARD
add_custom_target(
    generate_headers
    DEPENDS ${HEADERS_TO_GENERATE}
    COMMENT "Generating header files from YAML files"
)
