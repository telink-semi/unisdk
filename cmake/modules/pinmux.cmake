# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

# Stale *.pinmux files from a previous application are removed by the
# Makefile's APP_CHANGE_CLEAN when switching APP. They cannot be cleaned here:
# switching the source directory requires clearing the CMake cache (an APP
# switch through make clears it, and CMake's internal re-run clears it too),
# so a source-dir comparison would never have both sides of the cache intact.
include(extensions)
include(python)

if(${CONFIG_TLK_BOARD_IS_SELECTED})
    set(PINMUX_PREFIX "${CONFIG_TLK_BOARD}.board")
else()
    set(PINMUX_PREFIX "${CONFIG_TLK_SOC}.soc")
endif()

set(DOTPINMUX "${CMAKE_BINARY_DIR}/${PINMUX_PREFIX}.pinmux")
set(PINMUX_H "${CMAKE_BINARY_DIR}/pinmux.h")

# Inputs of the pinmux tool (see scripts/pinmux/data/pinmux_manager.py):
# chip.config/build.config (CONFIG_FILES, from kconfig.cmake) decide the
# target and which pin functions are needed, and the application's own
# .pinmux files override the defaults. Only files that exist can be listed -
# CMake re-globs them on reconfigure. The build directory's .pinmux is left
# out: the tool writes it itself.
set(PINMUX_DEPENDS ${CONFIG_FILES})
foreach(project_pinmux ".pinmux" "${PINMUX_PREFIX}.pinmux")
    if(EXISTS "${APPLICATION_SOURCE_DIR}/${project_pinmux}")
        list(APPEND PINMUX_DEPENDS "${APPLICATION_SOURCE_DIR}/${project_pinmux}")
    endif()
endforeach()


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
    DEPENDS ${PINMUX_DEPENDS}
    USES_TERMINAL
    COMMENT "Generating .pinmux file..."
)

# Update .pinmux if needed
add_custom_target(
    update_dotpinmux
    COMMAND ${PYTHON_EXECUTABLE} -m scripts.pinmux --project_dir=${APPLICATION_SOURCE_DIR} --build_dir=${CMAKE_BINARY_DIR} --try-skip-ui
    WORKING_DIRECTORY ${TELINK_BASE}
    VERBATIM
    DEPENDS ${PINMUX_DEPENDS}
    USES_TERMINAL
    COMMENT "Updating .pinmux file..."
)

# Auto-generate PINMUX_H as a real build product: a plain build must not require
# the interactive `make config`/`make pinmux` step first. This runs the pinmux
# tool non-interactively (--no-ui): projects with no mandatory (unassigned)
# pin functions get a valid header written automatically; projects that do
# require pin assignment fail the build here with a clear pinmux error instead
# of a generic "missing header" failure.
add_custom_command(
    OUTPUT ${PINMUX_H}
    COMMAND ${PYTHON_EXECUTABLE} -m scripts.pinmux --project_dir=${APPLICATION_SOURCE_DIR} --build_dir=${CMAKE_BINARY_DIR} --no-ui
    WORKING_DIRECTORY ${TELINK_BASE}
    VERBATIM
    DEPENDS ${PINMUX_DEPENDS}
    COMMENT "Autogenerating pinmux.h..."
)

add_custom_target(
    autogenerate_dotpinmux
    DEPENDS ${PINMUX_H}
    COMMENT "Autogenerating .pinmux file..."
)

# Custom target for automatically pinmux header check
add_custom_target(check_pinmux DEPENDS ${PINMUX_H})

list(APPEND FILES_TO_CLEAN ${PINMUX_H})
