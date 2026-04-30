# SPDX-License-Identifier: Apache-2.0

include_guard(GLOBAL)

function(set_ifndef variable value)
  if(NOT ${variable})
    set(${variable} ${value} ${ARGN} PARENT_SCOPE)
  endif()
endfunction()

function(add_sources_ifdef config src)
    if(${config})
        target_sources(${TARGET_NAME} PRIVATE ${src})
    endif()
endfunction()

function(add_sources src)
    target_sources(${TARGET_NAME} PRIVATE ${src})
endfunction()

function(json_to_list JSON_ARRAY_STRING OUTPUT_LIST)
    string(JSON ARRAY_TYPE TYPE "${JSON_ARRAY_STRING}")
    if(NOT ARRAY_TYPE STREQUAL "ARRAY")
        message(FATAL_ERROR "Error: The input is not a valid JSON array, is ${ARRAY_TYPE}.")
    endif()

    string(JSON ARRAY_LENGTH LENGTH "${JSON_ARRAY_STRING}")
    if(ARRAY_LENGTH GREATER 0)
        math(EXPR LAST_INDEX "${ARRAY_LENGTH} - 1")
        foreach(INDEX RANGE 0 ${LAST_INDEX})
            string(JSON ELEMENT GET "${JSON_ARRAY_STRING}" ${INDEX})
            string(CONFIGURE "${ELEMENT}" ELEMENT)
            string(REPLACE " " ";" ELEMENT "${ELEMENT}")
            separate_arguments(ELEMENT UNIX_COMMAND "${ELEMENT}")
            foreach(ELEMENT_ITEM ${ELEMENT})
                list(APPEND LOCAL_OUTPUT_LIST ${ELEMENT_ITEM})
            endforeach()
        endforeach()
    endif()

    if(DEFINED ARGV2)
        string(JOIN " " LOCAL_OUTPUT_STRING ${LOCAL_OUTPUT_LIST})
        set(${ARGV2} "${LOCAL_OUTPUT_STRING}" PARENT_SCOPE)
    endif()

    set(${OUTPUT_LIST} "${LOCAL_OUTPUT_LIST}" PARENT_SCOPE)

endfunction()