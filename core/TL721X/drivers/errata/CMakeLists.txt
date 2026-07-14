target_include_directories(${TARGET_NAME} PRIVATE .)

file(GLOB_RECURSE SRC "*.c" "*.S")
add_sources("${SRC}")
