target_include_directories(${TARGET_NAME} PRIVATE ./drivers/include)
set(SOC_DRIVER_SOURCES_PATH "drivers/src")

add_sources(${SOC_DRIVER_SOURCES_PATH}/tlk_mspi.c)
add_sources(${SOC_DRIVER_SOURCES_PATH}/tlk_platform.c)

if (NOT IS_DIRECTORY ${TARGET_LIBRARY_DIR})
    add_sources_ifdef(CONFIG_TLK_SYSTEM_TIMER_ADVANCED_MODE ${SOC_DRIVER_SOURCES_PATH}/tlk_advanced_stimer.c)

    # Library target
    telink_libraries_include_directories(PRIVATE ./drivers/include)

    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${SOC_DRIVER_SOURCES_PATH}/tlk_advanced_stimer.c)
endif()
