# SPDX-License-Identifier: Apache-2.0
#
# Cmake Configuration Time options in the yaml_helper.cmake:
#   GEN_ALL_BOARD_PROPERTIES - Generate property files for all boards, that belong to the selected SOC

include_guard(GLOBAL)

set(GENERATED_HEADERS_BASE_DIR "${GENERATED_BASE_DIR}/headers") # Directory to generate headers in
set(PROPERTIES_GEN_SCRIPT "scripts.generators.properties_gen")  # Script to generate property headers
set(REGISTERS_GEN_SCRIPT  "scripts.generators.registers_gen")   # Script to generate register headers

option(GEN_ALL_BOARD_PROPERTIES "Parse property files for all boards, that belong to the selected SOC" OFF)


# Create a list of all property YAML files for selected CORE, SOC, BOARD
set(PROPERTY_YAML_FILES "")

macro(add_property_yaml_files PROPERTY_SOURCES)
    list(GET ${PROPERTY_SOURCES} 0 yaml_dir)

    if(EXISTS "${yaml_dir}")
        file(GLOB yaml_files "${yaml_dir}/*.yaml")
        list(APPEND PROPERTY_YAML_FILES ${yaml_files})
    endif()
endmacro()


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


# Property sources
#
# core/soc live under packages/<CONFIG_TLK_CORE>/... (the restructure
# that introduced packages/ - one package can host more than one
# soc_series, so soc_series nests under its package). boards/ stays flat:
# a board isn't itself a core-family artifact, and boards/<board>/ is
# where the pinmux data for that board already lives.
set(CORE_PROPERTY_SOURCES
    "${TELINK_BASE}/packages/${CONFIG_TLK_CORE}/properties"
    "${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE}/properties"
    "${PROPERTIES_GEN_SCRIPT}"
)

set(SOC_PROPERTY_SOURCES
    "${TELINK_BASE}/packages/${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES}/properties"
    "${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES}/properties"
    "${PROPERTIES_GEN_SCRIPT}"
)

if(CONFIG_TLK_BOARD_IS_SELECTED)
    set(BOARD_PROPERTY_SOURCES
        "${TELINK_BASE}/packages/${CONFIG_TLK_CORE}/boards/${CONFIG_TLK_BOARD}/properties"
        "${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE}/boards/${CONFIG_TLK_BOARD}/properties"
        "${PROPERTIES_GEN_SCRIPT}"
    )
endif()

# Register sources
set(CORE_REGISTER_SOURCES
    "${TELINK_BASE}/packages/${CONFIG_TLK_CORE}/registers"
    "${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE}/registers"
    "${REGISTERS_GEN_SCRIPT}"
)

set(SOC_REGISTER_SOURCES
    "${TELINK_BASE}/packages/${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES}/registers"
    "${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES}/registers"
    "${REGISTERS_GEN_SCRIPT}"
)


# list of all property YAML files
add_property_yaml_files(CORE_PROPERTY_SOURCES)
add_property_yaml_files(SOC_PROPERTY_SOURCES)

if(CONFIG_TLK_BOARD_IS_SELECTED)
    add_property_yaml_files(BOARD_PROPERTY_SOURCES)
endif()


# list of all headers that have to be generated
add_headers_to_gen(CORE_PROPERTY_SOURCES)
add_headers_to_gen(SOC_PROPERTY_SOURCES)
add_headers_to_gen(CORE_REGISTER_SOURCES)
add_headers_to_gen(SOC_REGISTER_SOURCES)

if(GEN_ALL_BOARD_PROPERTIES)
    file(GLOB BOARD_DIRS LIST_DIRECTORIES true "${TELINK_BASE}/packages/${CONFIG_TLK_CORE}/boards/*")

    foreach(BOARD_DIR ${BOARD_DIRS})
        get_filename_component(BOARD_NAME ${BOARD_DIR} NAME)

        set(BOARD_${BOARD_NAME}_PROPERTY_SOURCES
            "${BOARD_DIR}/properties"
            "${GENERATED_HEADERS_BASE_DIR}/packages/${CONFIG_TLK_CORE}/boards/${BOARD_NAME}/properties"
            "${PROPERTIES_GEN_SCRIPT}"
        )

        add_headers_to_gen(BOARD_${BOARD_NAME}_PROPERTY_SOURCES)
    endforeach()
else()
    if(CONFIG_TLK_BOARD_IS_SELECTED)
        add_headers_to_gen(BOARD_PROPERTY_SOURCES)
    endif()
endif()

# Custom target which depends on all headers that have to be generated for selected CORE, SOC, BOARD
add_custom_target(
    generate_headers
    DEPENDS ${HEADERS_TO_GENERATE}
    COMMENT "Generating header files from YAML files"
)
