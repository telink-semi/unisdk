# SPDX-License-Identifier: Apache-2.0
#
# Pristine (config/build cleanup) capability for the Telink build system.
#
# This file serves two modes, dispatched via CMAKE_SCRIPT_MODE_FILE:
#
#   1. cmake -P script mode (cmake -DBINARY_DIR=<dir> -DSOURCE_DIR=<dir>
#      -P <this file>): performs the actual cleanup, with safety checks
#      (absolute-path validation, refusal of in-source builds), mirroring
#      Zephyr's approach.
#
#   2. include mode (configure): registers the 'pristine' and 'pristine-config'
#      build targets.
#
#      - 'pristine'        - remove EVERYTHING inside the build directory, so the
#                            next CMake configure starts from scratch.
#      - 'pristine-config' - remove only the generated configuration files
#                            (chip.config, build.config, .config, autoconf.h,
#                            Kconfig/, pinmux.h) while keeping the CMake cache,
#                            compiled artifacts and user *.pinmux files.
#
# The automatic regeneration of derived configs on SOC/BOARD changes is
# handled at configure time in kconfig.cmake (see the "Automatic derived-config
# regeneration" section there).

include_guard(GLOBAL)

# =============================================================================
# cmake -P script mode: perform the cleanup.
# =============================================================================
if(CMAKE_SCRIPT_MODE_FILE STREQUAL CMAKE_CURRENT_LIST_FILE)
  macro(print_usage)
    message("
usage: cmake -DBINARY_DIR=<build-path> -DSOURCE_DIR=<source-path>
             -P ${CMAKE_SCRIPT_MODE_FILE}

mandatory arguments:
  -DBINARY_DIR=<build-path>:  Absolute path to the build directory to pristine
  -DSOURCE_DIR=<source-path>: Absolute path to the source directory used when
                              creating <build-path>
")
    # Making the usage itself a fatal error messes up the formatting when printing.
    message(FATAL_ERROR "")
  endmacro()

  if(NOT DEFINED BINARY_DIR OR NOT DEFINED SOURCE_DIR)
    print_usage()
  endif()

  if(NOT IS_ABSOLUTE ${BINARY_DIR} OR NOT IS_ABSOLUTE ${SOURCE_DIR})
    print_usage()
  endif()

  get_filename_component(BINARY_DIR ${BINARY_DIR} REALPATH)
  get_filename_component(SOURCE_DIR ${SOURCE_DIR} REALPATH)

  string(FIND ${SOURCE_DIR} ${BINARY_DIR} INDEX)
  if(NOT INDEX EQUAL -1)
    message(FATAL_ERROR "Refusing to run pristine in in-source build folder.")
  endif()

  # The build directory itself is kept (contents only are removed) instead of
  # removing the whole directory, because Ninja needs to write its
  # .ninja_log / .ninja_deps after the target finishes; removing the directory
  # that Ninja is running in could make those writes fail and turn a successful
  # clean into a failing build. Removing all contents achieves the same pristine
  # state for the next configure.
  file(GLOB build_dir_contents ${BINARY_DIR}/*)
  foreach(file ${build_dir_contents})
    if(EXISTS ${file})
       file(REMOVE_RECURSE ${file})
    endif()
  endforeach(file)

  return()
endif()

# =============================================================================
# Include mode (configure): register the cleanup targets.
# =============================================================================

# 'pristine' target: invoke this file in script mode to perform the cleanup.
add_custom_target(
  pristine
  COMMAND ${CMAKE_COMMAND} -DBINARY_DIR=${CMAKE_BINARY_DIR}
          -DSOURCE_DIR=${APPLICATION_SOURCE_DIR}
          -P ${CMAKE_CURRENT_LIST_FILE}
  WORKING_DIRECTORY ${TELINK_BASE}
  VERBATIM
  COMMENT "Pristine: removing all build artifacts in ${CMAKE_BINARY_DIR}"
)

# 'pristine-config' target.
#
# Resets all configuration files so the next configure regenerates them from
# scratch, while keeping the CMake cache and compiled artifacts (fast
# reconfiguration). *.pinmux files are kept as well: they are named by
# board/soc prefix (e.g. TLSR9528A_EVK.board.pinmux) and can be reused when
# the hardware is unchanged.
set(TLK_CONFIG_FILES
    ${CMAKE_BINARY_DIR}/chip.config
    ${CMAKE_BINARY_DIR}/build.config
    ${CMAKE_BINARY_DIR}/.config
    ${CMAKE_BINARY_DIR}/autoconf.h
    ${CMAKE_BINARY_DIR}/pinmux.h)
add_custom_target(
  pristine-config
  COMMAND ${CMAKE_COMMAND} -E remove -f ${TLK_CONFIG_FILES}
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Kconfig
  WORKING_DIRECTORY ${TELINK_BASE}
  VERBATIM
  COMMENT "Pristine-config: removing generated configuration files (chip.config, build.config, .config, autoconf.h, Kconfig/, pinmux.h)"
)
