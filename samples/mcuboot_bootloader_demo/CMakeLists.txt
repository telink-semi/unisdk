# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Telink REQUIRED HINTS $ENV{TELINK_BASE})
project(MCUboot_Bootloader LANGUAGES C CXX)

file(GLOB C_SOURCES "*.c")
target_sources(Telink PRIVATE ${C_SOURCES})
