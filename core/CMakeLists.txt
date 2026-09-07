set(COMMON_SOURCES_PATH "common/src")
set(DRIVERS_PATH "${CONFIG_TLK_CORE}/drivers")

# Common sources
target_sources(${TARGET_NAME} PRIVATE ${COMMON_SOURCES_PATH}/tlk_trap.c)

# Common sources with condition
add_sources_ifdef(CONFIG_TLK_CRITICAL_PRINTOUT ${COMMON_SOURCES_PATH}/tlk_critical_print.c)
add_sources_ifdef(CONFIG_TLK_DMA ${COMMON_SOURCES_PATH}/tlk_dma.c)
add_sources_ifdef(CONFIG_TLK_I2C ${COMMON_SOURCES_PATH}/tlk_i2c.c)
add_sources_ifdef(CONFIG_TLK_MTIMER ${COMMON_SOURCES_PATH}/tlk_mtimer.c)
add_sources_ifdef(CONFIG_TLK_PLIC ${COMMON_SOURCES_PATH}/tlk_plic.c)
add_sources_ifdef(CONFIG_TLK_PLIC_SW ${COMMON_SOURCES_PATH}/tlk_plic_sw.c)
add_sources_ifdef(CONFIG_TLK_PMP ${COMMON_SOURCES_PATH}/tlk_pmp.c)
add_sources_ifdef(CONFIG_TLK_SPI ${COMMON_SOURCES_PATH}/tlk_spi.c)
add_sources_ifdef(CONFIG_TLK_UART ${COMMON_SOURCES_PATH}/tlk_uart.c)
add_sources(${COMMON_SOURCES_PATH}/tlk_mcu_info.c)

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
    # Common sources with condition
    add_sources_ifdef(CONFIG_TLK_RF ${COMMON_SOURCES_PATH}/_tlk_rf.c)

    # Сore sources
    target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/_tlk_sys.c)

    # Сore sources with condition
    add_sources_ifdef(CONFIG_TLK_PM ${DRIVERS_PATH}/_tlk_pm.c)
    add_sources_ifdef(CONFIG_TLK_RF ${DRIVERS_PATH}/_tlk_rf.c)
    add_sources_ifdef(CONFIG_TLK_RF ${DRIVERS_PATH}/_tlk_rf_init.c)
    add_sources_ifdef(CONFIG_TLK_RF ${DRIVERS_PATH}/_tlk_rf_calibration.c)

    # Library targets
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_pm.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${COMMON_SOURCES_PATH}/_tlk_rf.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_rf.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_rf_init.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_rf_calibration.c)
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${DRIVERS_PATH}/_tlk_sys.c)
endif()

add_subdirectory(cpp_wrappers)
add_subdirectory(${DRIVERS_PATH}/errata)
