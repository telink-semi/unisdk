set(DRIVERS_PATH "${CONFIG_TLK_CORE}/drivers")
set(COMMON_PATH "common")

add_subdirectory(cpp_wrappers)
add_subdirectory(${DRIVERS_PATH}/errata)

add_sources_ifdef(CONFIG_TLK_CRITICAL_PRINTOUT ${COMMON_PATH}/src/critical_print.c)

target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_analog.c)
target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_clic.c)
target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_clock.c)
target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_stimer.c)
target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_power.c)

add_sources_ifdef(CONFIG_TLK_ADC ${DRIVERS_PATH}/tlk_adc.c)
add_sources_ifdef(CONFIG_TLK_DMA ${DRIVERS_PATH}/tlk_dma.c)
add_sources_ifdef(CONFIG_TLK_GPIO ${DRIVERS_PATH}/tlk_gpio.c)
add_sources_ifdef(CONFIG_TLK_MTIMER ${DRIVERS_PATH}/tlk_mtimer.c)
add_sources_ifdef(CONFIG_TLK_PLIC ${DRIVERS_PATH}/tlk_plic.c)
add_sources_ifdef(CONFIG_TLK_PLIC_SW ${DRIVERS_PATH}/tlk_plic_sw.c)
add_sources_ifdef(CONFIG_TLK_DEBUG_INTERFACES ${DRIVERS_PATH}/tlk_jtag_sws.c)
add_sources_ifdef(CONFIG_TLK_PM ${DRIVERS_PATH}/tlk_pm.c)
add_sources_ifdef(CONFIG_TLK_SPI ${DRIVERS_PATH}/tlk_spi.c)
add_sources_ifdef(CONFIG_TLK_UART ${DRIVERS_PATH}/tlk_uart.c)
add_sources_ifdef(CONFIG_TLK_WDT ${DRIVERS_PATH}/tlk_watchdog.c)
add_sources_ifdef(CONFIG_TLK_USB ${DRIVERS_PATH}/tlk_usb.c)

if (NOT IS_DIRECTORY ${TARGET_LIBRARY_DIR})
    target_sources(${TARGET_NAME} PRIVATE ${DRIVERS_PATH}/tlk_sys.c)

    add_sources_ifdef(CONFIG_TLK_PM ${DRIVERS_PATH}/_tlk_pm.c)
    
    add_sources_ifdef(CONFIG_TLK_RF ${COMMON_PATH}/src/tlk_rf.c)
    add_sources_ifdef(CONFIG_TLK_RF ${DRIVERS_PATH}/tlk_rf_init.c)
    add_sources_ifdef(CONFIG_TLK_RF ${DRIVERS_PATH}/tlk_rf.c)

    # Library target
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${DRIVERS_PATH}/tlk_sys.c)
    
    target_sources("${TARGET_SOC_LIBRARY}-pm" PRIVATE ${DRIVERS_PATH}/_tlk_pm.c)

    target_sources("${TARGET_SOC_LIBRARY}-rf" PRIVATE ${COMMON_PATH}/src/tlk_rf.c)
    target_sources("${TARGET_SOC_LIBRARY}-rf" PRIVATE ${DRIVERS_PATH}/tlk_rf_init.c)
    target_sources("${TARGET_SOC_LIBRARY}-rf" PRIVATE ${DRIVERS_PATH}/tlk_rf.c)
endif()
