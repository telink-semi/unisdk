target_include_directories(${TARGET_NAME} PRIVATE .)

file(GLOB_RECURSE SRC "*.c")
add_sources("${SRC}")
