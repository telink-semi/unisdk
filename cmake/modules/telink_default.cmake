# SPDX-License-Identifier: Apache-2.0
#
# Copyright (c) 2021, Nordic Semiconductor ASA
include_guard(GLOBAL)

# The code line below defines the real minimum supported CMake version.
#
# Unfortunately CMake requires the toplevel CMakeLists.txt file to define the
# required version, not even invoking it from a CMake module is sufficient.
# It is however permitted to have multiple invocations of cmake_minimum_required.
cmake_minimum_required(VERSION 3.20.0)

message(STATUS "Application: ${APPLICATION_SOURCE_DIR}")
# Save the sample/project directory name for informational purposes
get_filename_component(TLK_SAMPLE_NAME "${APPLICATION_SOURCE_DIR}" NAME)
message(STATUS "Sample: ${TLK_SAMPLE_NAME}")
# Default output binary name is "Telink" — use CONFIG_TLK_USE_CUSTOM_ARTIFACT_NAME
# to override with the sample name or a custom name (see root CMakeLists.txt)
set(TLK_ARTIFACT_NAME "Telink")

# Different CMake versions can have very subtle differences, for
# instance CMake 3.21 links object files in a different order compared
# to CMake 3.20; this produces different binaries.
message(STATUS "CMake version: ${CMAKE_VERSION}")

# Test and error-out if we are affected by the PyPI CMake 3.22.1 / 3.22.2 bug
if(${CMAKE_VERSION} VERSION_EQUAL 3.22.1 OR ${CMAKE_VERSION} VERSION_EQUAL 3.22.2)
  # It seems only pip-installed builds are affected so we test to see if we are affected
  cmake_path(GET TELINK_BASE PARENT_PATH test_cmake_path)
  if(TELINK_BASE STREQUAL test_cmake_path)
    message(FATAL_ERROR "The CMake version ${CMAKE_VERSION} installed suffers"
            " the \n 'cmake_path(... PARENT_PATH)' bug, see: \n"
	    "https://gitlab.kitware.com/cmake/cmake/-/issues/23187\n"
	    "https://github.com/scikit-build/cmake-python-distributions/issues/221\n"
	    "Please install another CMake version or use a build of CMake that"
	    " does not come from PyPI."
    )
  endif()
endif()

list(APPEND telink_cmake_modules python)
list(APPEND telink_cmake_modules extensions)
list(APPEND telink_cmake_modules west)
list(APPEND telink_cmake_modules kconfig)
list(APPEND telink_cmake_modules compiler)
list(APPEND telink_cmake_modules headers_gen)
list(APPEND telink_cmake_modules pinmux)
list(APPEND telink_cmake_modules linker)
list(APPEND telink_cmake_modules library)


foreach(component ${SUB_COMPONENTS})
  if(NOT ${component} IN_LIST telink_cmake_modules)
    message(FATAL_ERROR
      "Subcomponent '${component}' not default module for Telink CMake build system.\n"
      "Please choose one or more valid components: ${telink_cmake_modules}"
    )
  endif()
endforeach()

foreach(module IN LISTS telink_cmake_modules)
  # Ensures any module of type `${module}` are properly expanded to list before
  # passed on the `include(${module})`.
  # This is done twice to support cases where the content of `${module}` itself
  # contains a variable, like `${BOARD_DIR}`.
  string(CONFIGURE "${module}" module)
  string(CONFIGURE "${module}" module)
  include(${module})

  list(REMOVE_ITEM SUB_COMPONENTS ${module})
  if(DEFINED SUB_COMPONENTS AND NOT SUB_COMPONENTS)
    # All requested Telink CMake modules have been loaded, so let's return.
    return()
  endif()
endforeach()

if(NOT DEFINED TELINK_ROOT_INCLUDED)
  # Include root directory CMakeLists.txt
  add_subdirectory(${TELINK_BASE} ${CMAKE_BINARY_DIR}/telink)
endif()
