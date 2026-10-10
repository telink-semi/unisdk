# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Telink REQUIRED HINTS $ENV{TELINK_BASE})
project(USB_Demo LANGUAGES C CXX)

# Add source files
file(GLOB C_SOURCES "*.c")

# Add C++ files only if CONFIG_TLK_ALLOW_CPP_WRAPPERS is enabled
if(CONFIG_TLK_ALLOW_CPP_WRAPPERS)
    # Remove main.c from C sources if C++ mode is active
    list(REMOVE_ITEM C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.c")
    file(GLOB CPP_SOURCES "*.cpp")
    target_sources(Telink PRIVATE ${CPP_SOURCES})

    target_compile_options(Telink PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-enum-compare>
        # tinyUSB's tusb_types.h mixes enum and non-enum operands in a ternary;
        # GCC only gates that diagnostic behind the broad -Wextra bucket (no
        # dedicated sub-flag), and it's vendored code we can't edit.
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-extra>
    )
endif()

target_sources(Telink PRIVATE ${C_SOURCES})
