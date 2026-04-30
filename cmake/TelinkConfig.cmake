# SPDX-License-Identifier: Apache-2.0

# This file provides Telink Config Package functionality.
set(TELINK_PACKAGE_INCLUDED TRUE)
set(TARGET_NAME Telink)

macro(include_boilerplate)
  # Set TELINK_BASE if not already set
  if(NOT DEFINED TELINK_BASE)
    if(DEFINED ENV{TELINK_BASE})
      set(TELINK_BASE $ENV{TELINK_BASE} CACHE PATH "Telink base directory")
    else()
      set(TELINK_BASE ${CMAKE_CURRENT_LIST_DIR}/.. CACHE PATH "Telink base directory")
    endif()
  endif()

  list(PREPEND CMAKE_MODULE_PATH ${TELINK_BASE}/cmake/modules)

  if(NOT DEFINED APPLICATION_SOURCE_DIR)
    set(APPLICATION_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH
        "Application Source Directory"
    )
  endif()

  if(NOT DEFINED APPLICATION_BINARY_DIR)
    set(APPLICATION_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH
        "Application Binary Directory"
    )
  endif()

  message(NOTICE "Loading Telink default modules")
  include(telink_default NO_POLICY_SCOPE)
endmacro()

# Include boilerplate code
include_boilerplate()