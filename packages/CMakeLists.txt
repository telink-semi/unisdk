set(DRIVERS_PATH "${CONFIG_TLK_CORE}/drivers")

# Сore sources
target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_analog.c)
target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_clock.c)
target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_power.c)
target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_stimer.c)

# Сore sources with condition
add_sources_ifdef(CONFIG_TLK_ADC ${DRIVERS_PATH}/tlk_adc.c)
add_sources_ifdef(CONFIG_TLK_AUDIO ${DRIVERS_PATH}/tlk_audio.c)
add_sources_ifdef(CONFIG_TLK_DMA ${DRIVERS_PATH}/tlk_dma.c)
add_sources_ifdef(CONFIG_TLK_GPIO ${DRIVERS_PATH}/tlk_gpio.c)
add_sources_ifdef(CONFIG_TLK_I2C ${DRIVERS_PATH}/tlk_i2c.c)
add_sources_ifdef(CONFIG_TLK_I2S ${DRIVERS_PATH}/tlk_i2s.c)
add_sources_ifdef(CONFIG_TLK_DEBUG_INTERFACES ${DRIVERS_PATH}/tlk_debug.c)
add_sources_ifdef(CONFIG_TLK_PM ${DRIVERS_PATH}/tlk_pm.c)
add_sources_ifdef(CONFIG_TLK_RANDOM ${DRIVERS_PATH}/tlk_random.c)
add_sources_ifdef(CONFIG_TLK_SPI ${DRIVERS_PATH}/tlk_spi.c)
add_sources_ifdef(CONFIG_TLK_UART ${DRIVERS_PATH}/tlk_uart.c)
add_sources_ifdef(CONFIG_TLK_USB ${DRIVERS_PATH}/tlk_usb.c)
add_sources_ifdef(CONFIG_TLK_WDT ${DRIVERS_PATH}/tlk_watchdog.c)

if(NOT IS_SDK_MODE)
    # Сore sources
    target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/_tlk_sys.c)

    # Сore sources with condition
    add_sources_ifdef(CONFIG_TLK_PM ${DRIVERS_PATH}/_tlk_pm.c)
    add_sources_ifdef(CONFIG_TLK_RF ${DRIVERS_PATH}/_tlk_rf.c)
    add_sources_ifdef(CONFIG_TLK_RF ${DRIVERS_PATH}/_tlk_rf_init.c)
    add_sources_ifdef(CONFIG_TLK_RF ${DRIVERS_PATH}/_tlk_rf_calibration.c)

    # Library targets
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_pm.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_rf.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_rf_init.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_rf_calibration.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_sys.c)
endif()

add_subdirectory(${CONFIG_TLK_CORE}/drivers/errata)
add_subdirectory(${CONFIG_TLK_CORE}/soc/${CONFIG_TLK_SOC_SERIES})
