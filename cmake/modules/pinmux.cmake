# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

include(extensions)
include(python)

# Remove existing .pinmux files only when the source directory changes
if(NOT "${CMAKE_SOURCE_DIR}" STREQUAL "$CACHE{PREVIOUS_SOURCE_DIR}")
    file(GLOB_RECURSE pinmux_files "${CMAKE_BINARY_DIR}/*.pinmux")
    if(pinmux_files)
        file(REMOVE ${pinmux_files})
    endif()
endif()
set(PREVIOUS_SOURCE_DIR "${CMAKE_SOURCE_DIR}" CACHE INTERNAL "")

if(${CONFIG_TLK_HAS_BOARD_SELECTED})
    set(PINMUX_PREFIX "${CONFIG_TLK_BOARD}.board")
else()
    set(PINMUX_PREFIX "${CONFIG_TLK_SOC}.soc")
endif()

set(PINMUX_H ${CMAKE_BINARY_DIR}/pinmux.h)
set(DOTPINMUX ${CMAKE_BINARY_DIR}/${PINMUX_PREFIX}.pinmux)

# Generate empty pinmux header if it doesn't exist
if((NOT EXISTS ${PINMUX_H}) AND EXISTS ${DOTCONFIG})
    file(TOUCH ${PINMUX_H})
endif()

# Create a custom target for generating .pinmux (replaces add_custom_command)
add_custom_target(
    generate_dotpinmux
    COMMAND ${PYTHON_EXECUTABLE} -m scripts.pinmux --project_dir=${APPLICATION_SOURCE_DIR} --build_dir=${CMAKE_BINARY_DIR}
    WORKING_DIRECTORY ${TELINK_BASE}
    VERBATIM
    DEPENDS ${DOTCONFIG}
    USES_TERMINAL
    COMMENT "Generating .pinmux file..."
)

# Update .pinmux if needed
add_custom_target(
    update_dotpinmux
    COMMAND ${PYTHON_EXECUTABLE} -m scripts.pinmux --project_dir=${APPLICATION_SOURCE_DIR} --build_dir=${CMAKE_BINARY_DIR} --try-skip-ui
    WORKING_DIRECTORY ${TELINK_BASE}
    VERBATIM
    DEPENDS ${DOTCONFIG}
    USES_TERMINAL
    COMMENT "Updating .pinmux file..."
)

add_custom_target(
    autogenerate_dotpinmux
    COMMAND ${PYTHON_EXECUTABLE} -m scripts.pinmux --project_dir=${APPLICATION_SOURCE_DIR} --build_dir=${CMAKE_BINARY_DIR} --no-ui
    WORKING_DIRECTORY ${TELINK_BASE}
    VERBATIM
    DEPENDS ${DOTCONFIG}
    COMMENT "Autogenerating .pinmux file..."
)
