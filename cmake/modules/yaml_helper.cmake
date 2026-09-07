# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

set(GENERATED_HEADERS_BASE_DIR "${GENERATED_BASE_DIR}/headers") # Directory to generate headers in
set(PROPERTIES_GEN_SCRIPT "scripts.generators.properties_gen")  # Script to generate property headers
set(REGISTERS_GEN_SCRIPT  "scripts.generators.registers_gen")   # Script to generate register headers


# Property sources
set(CORE_PROPERTY_SOURCES
    "${TELINK_BASE}/core/${CONFIG_TLK_CORE}/properties"
    "${GENERATED_HEADERS_BASE_DIR}/core/${CONFIG_TLK_CORE}/properties"
    "${PROPERTIES_GEN_SCRIPT}"
)

set(SOC_PROPERTY_SOURCES
    "${TELINK_BASE}/soc/${CONFIG_TLK_SOC_SERIES}/properties"
    "${GENERATED_HEADERS_BASE_DIR}/soc/${CONFIG_TLK_SOC_SERIES}/properties"
    "${PROPERTIES_GEN_SCRIPT}"
)

if(CONFIG_TLK_HAS_BOARD_SELECTED)
    set(BOARD_PROPERTY_SOURCES
        "${TELINK_BASE}/boards/${CONFIG_TLK_BOARD}/properties"
        "${GENERATED_HEADERS_BASE_DIR}/boards/${CONFIG_TLK_BOARD}/properties"
        "${PROPERTIES_GEN_SCRIPT}"
    )
endif()

# Register sources
set(CORE_REGISTER_SOURCES
    "${TELINK_BASE}/core/${CONFIG_TLK_CORE}/registers"
    "${GENERATED_HEADERS_BASE_DIR}/core/${CONFIG_TLK_CORE}/registers"
    "${REGISTERS_GEN_SCRIPT}"
)

set(SOC_REGISTER_SOURCES
    "${TELINK_BASE}/soc/${CONFIG_TLK_SOC_SERIES}/registers"
    "${GENERATED_HEADERS_BASE_DIR}/soc/${CONFIG_TLK_SOC_SERIES}/registers"
    "${REGISTERS_GEN_SCRIPT}"
)


# Create a list of all property YAML files for selected CORE, SOC, BOARD
set(PROPERTY_YAML_FILES "")

macro(add_property_yaml_files PROPERTY_SOURCES)
    list(GET ${PROPERTY_SOURCES} 0 yaml_dir)

    if(EXISTS "${yaml_dir}")
        file(GLOB yaml_files "${yaml_dir}/*.yaml")
        list(APPEND PROPERTY_YAML_FILES ${yaml_files})
    endif()
endmacro()

add_property_yaml_files(CORE_PROPERTY_SOURCES)
add_property_yaml_files(SOC_PROPERTY_SOURCES)

if(CONFIG_TLK_HAS_BOARD_SELECTED)
    add_property_yaml_files(BOARD_PROPERTY_SOURCES)
endif()
