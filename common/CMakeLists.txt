set(COMMON_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(COMMON_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_sources(${TARGET_NAME} PRIVATE "${COMMON_SRC_PATH}/tlk_sdk_version.c")
target_sources(${TARGET_NAME} PRIVATE "${COMMON_SRC_PATH}/tlk_init.c")
target_sources(${TARGET_NAME} PRIVATE "${COMMON_SRC_PATH}/tlk_loop.c")

set(COMMON_COMPILE_OPTIONS
    "--imacros=${COMMON_INCLUDE_PATH}/tlk_soc_addr.h"
    "--imacros=${COMMON_INCLUDE_PATH}/tlk_bit.h"
    "--imacros=${COMMON_INCLUDE_PATH}/tlk_compiler.h"
    "--imacros=${COMMON_INCLUDE_PATH}/tlk_config_macro.h"
    $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-include ${COMMON_INCLUDE_PATH}/tlk_types.h>
)

target_compile_options(${TARGET_NAME} PRIVATE ${COMMON_COMPILE_OPTIONS})
telink_libraries_compile_options(PRIVATE ${COMMON_COMPILE_OPTIONS})