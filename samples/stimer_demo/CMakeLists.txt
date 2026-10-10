# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Telink REQUIRED HINTS $ENV{TELINK_BASE})
project(GPIO_Demo LANGUAGES C CXX)

if(CONFIG_TLK_ALLOW_CPP_WRAPPERS)
    #build cpp sources when cpp wrappers are enabled
    file(GLOB CPP_SOURCES "*.cpp")
    target_sources(Telink PRIVATE ${CPP_SOURCES})
else()
    #build c sources whn cpp wrappers are disabled
    file(GLOB C_SOURCES "*.c")
    target_sources(Telink PRIVATE ${C_SOURCES})
endif()
