set(TARGET_LIBRARY_BUILD_DIR "${CMAKE_BINARY_DIR}/libraries") # Output directory for libraries buiding
set(TELINK_SOC_LIBS "") # Accumulates all SOC library targets registered via telink_add_library()

# Declares a static SOC library.
# Dev mode:     always creates and builds the library target.
# Exported SDK: links the prebuilt .a only if CONDITION config variable is set (absent = always link).
#
# Usage: telink_add_library(<name> [CONDITION <config_var>])
macro(telink_add_library LIB_NAME)
    if(NOT IS_SDK_MODE)
        add_library(${LIB_NAME} STATIC EXCLUDE_FROM_ALL)

        message(STATUS "DEV SRC: Library to build: ${LIB_NAME}")

        set_target_properties(${LIB_NAME} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY ${TARGET_LIBRARY_BUILD_DIR}
        )

        add_dependencies(${LIB_NAME} generate_headers)
        # telink_apply_compiler_options() below force-includes GIT_VERSION_H via --imacros;
        # without this dependency nothing generates that header before compiling this target.
        add_dependencies(${LIB_NAME} generate_git_version)

        add_custom_command(TARGET ${LIB_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} -g $<TARGET_FILE:${LIB_NAME}>
            COMMAND ${CMAKE_RANLIB} $<TARGET_FILE:${LIB_NAME}>
            COMMENT "DEV SRC: Stripping and reindexing ${LIB_NAME}"
        )

        telink_apply_compiler_options(${LIB_NAME})

        list(APPEND TELINK_SOC_LIBS ${LIB_NAME})
    else()
        cmake_parse_arguments(LINK "" "CONDITION" "" ${ARGN})

        # If CONDITION not provided → always link.
        # If provided → check the value of that config variable (e.g. CONFIG_TLK_PM = y/n).
        if (NOT DEFINED LINK_CONDITION OR DEFINED ${LINK_CONDITION})
            message(STATUS "SDK: Linking library: ${LIB_NAME}")

            target_link_libraries(${TARGET_NAME} PRIVATE
                "${LIBRARY_BASE_DIR}/lib${LIB_NAME}.a"
            )
        endif()
    endif()
endmacro()

# Applies include directories to all registered SOC libraries.
# Usage: telink_libraries_include_directories(PRIVATE <path> ...)
macro(telink_libraries_include_directories)
    if(NOT IS_SDK_MODE)
        foreach(_lib IN LISTS TELINK_SOC_LIBS)
            target_include_directories(${_lib} ${ARGN})
        endforeach()
    endif()
endmacro()


# Applies compile options to all registered SOC libraries.
# Usage: telink_libraries_compile_options(PRIVATE <options> ...)
macro(telink_libraries_compile_options)
    if(NOT IS_SDK_MODE)
        foreach(_lib IN LISTS TELINK_SOC_LIBS)
            target_compile_options(${_lib} ${ARGN})
        endforeach()
    endif()
endmacro()


# Creates an "umbrella" target 'build_soc_libraries' that builds all registered SOC libraries at once,
# then collects their sources, deduplicates them, and writes the result to
# <file_prefix>_sources.txt in the libraries build directory (used by sdk_exporter.sh).
#
# Call once after all telink_add_library() declarations.
# Usage: telink_create_soc_libraries_target(<file_prefix>)
#        cmake --build build --target build_soc_libraries
macro(telink_declare_libraries_target SOC_LIB_NAME)
    if(NOT IS_SDK_MODE)
        add_custom_target(build_soc_libraries
            DEPENDS ${TELINK_SOC_LIBS}
            COMMENT "DEV SRC: Building all SOC libraries"
        )

        set(_all_sources "")

        foreach(_lib IN LISTS TELINK_SOC_LIBS)
            get_target_property(_srcs ${_lib} SOURCES)

            if (_srcs)
                list(APPEND _all_sources ${_srcs})
            endif()
        endforeach()

        list(REMOVE_DUPLICATES _all_sources)
        list(JOIN _all_sources "\n" _content)

        file(WRITE
            "${TARGET_LIBRARY_BUILD_DIR}/${SOC_LIB_NAME}_sources.txt"
            "${_content}\n"
        )

        unset(_all_sources)
    endif()
endmacro()
