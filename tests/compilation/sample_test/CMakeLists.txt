# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Telink REQUIRED HINTS $ENV{TELINK_BASE})
project(test_simple)

# Add source files
target_sources(Telink PRIVATE
    src/main.c
)
