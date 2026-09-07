# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

# Stale *.pinmux files from a previous application are removed by the
# Makefile's APP_CHANGE_CLEAN when switching APP. They cannot be cleaned here:
# switching the source directory requires clearing the CMake cache (an APP
# switch through make clears it, and CMake's internal re-run clears it too),
# so a source-dir comparison would never have both sides of the cache intact.
include(extensions)
include(python)

if(${CONFIG_TLK_HAS_BOARD_SELECTED})
    set(PINMUX_PREFIX "${CONFIG_TLK_BOARD}.board")
else()
    set(PINMUX_PREFIX "${CONFIG_TLK_SOC}.soc")
endif()

set(DOTPINMUX "${CMAKE_BINARY_DIR}/${PINMUX_PREFIX}.pinmux")
set(PINMUX_H "${CMAKE_BINARY_DIR}/pinmux.h")


# Remove existing .pinmux files only when the sample changes
if(FIRST_CONFIGURATION)
    file(GLOB_RECURSE pinmux_files "${CMAKE_BINARY_DIR}/*.pinmux")

    if(pinmux_files)
        file(REMOVE ${pinmux_files})
    endif()

    if(EXISTS "${PINMUX_H}")
        file(REMOVE "${PINMUX_H}")
    endif()
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


# Custom target for automatically pinmux header check
add_custom_target(check_pinmux DEPENDS ${PINMUX_H})

list(APPEND FILES_TO_CLEAN ${PINMUX_H})
