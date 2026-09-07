set(DRIVER_SOURCES_PATH "src")

# SoC sources
add_sources(${DRIVER_SOURCES_PATH}/tlk_mspi.c)

if(NOT IS_SDK_MODE)
    # SoC sources
    add_sources(${DRIVER_SOURCES_PATH}/_tlk_otp.c)

    # SoC sources with condition
    add_sources_ifdef(CONFIG_TLK_SYSTEM_TIMER_ADVANCED_MODE ${DRIVER_SOURCES_PATH}/_tlk_advanced_stimer.c)

    # Library target
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${DRIVER_SOURCES_PATH}/_tlk_advanced_stimer.c)
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${DRIVER_SOURCES_PATH}/_tlk_otp.c)
endif()
