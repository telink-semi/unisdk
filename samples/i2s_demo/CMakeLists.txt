# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Telink REQUIRED HINTS $ENV{TELINK_BASE})
project(I2S_Demo LANGUAGES C CXX)

# Add source files
file(GLOB C_SOURCES "*.c")

# Add C++ files only if CONFIG_TLK_ALLOW_CPP_WRAPPERS is enabled
if(CONFIG_TLK_ALLOW_CPP_WRAPPERS)
    # Remove main.c from C sources
    list(REMOVE_ITEM C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.c")
    file(GLOB CPP_SOURCES "*.cpp")
    target_sources(Telink PRIVATE ${CPP_SOURCES})
endif()

target_sources(Telink PRIVATE ${C_SOURCES})
