set(COMMON_SOURCES_PATH "common/src")

# Common sources
target_sources(${TARGET_NAME} PRIVATE ${COMMON_SOURCES_PATH}/tlk_trap.c)

# Common sources with condition
add_sources_ifdef(CONFIG_TLK_CRITICAL_PRINT ${COMMON_SOURCES_PATH}/tlk_critical_print.c)
add_sources_ifdef(CONFIG_TLK_DMA ${COMMON_SOURCES_PATH}/tlk_dma.c)
add_sources_ifdef(CONFIG_TLK_I2C ${COMMON_SOURCES_PATH}/tlk_i2c.c)
add_sources_ifdef(CONFIG_TLK_MTIMER ${COMMON_SOURCES_PATH}/tlk_mtimer.c)
add_sources_ifdef(CONFIG_TLK_PLIC ${COMMON_SOURCES_PATH}/tlk_plic.c)
add_sources_ifdef(CONFIG_TLK_PLIC_SW ${COMMON_SOURCES_PATH}/tlk_plic_sw.c)
add_sources_ifdef(CONFIG_TLK_PMP ${COMMON_SOURCES_PATH}/tlk_pmp.c)
add_sources_ifdef(CONFIG_TLK_SPI ${COMMON_SOURCES_PATH}/tlk_spi.c)
add_sources_ifdef(CONFIG_TLK_UART ${COMMON_SOURCES_PATH}/tlk_uart.c)
add_sources(${COMMON_SOURCES_PATH}/tlk_mcu_info.c)

if(NOT IS_SDK_MODE)
    # Common sources with condition
    add_sources_ifdef(CONFIG_TLK_RF ${COMMON_SOURCES_PATH}/_tlk_rf.c)

    # Library targets
    target_sources("${TARGET_SOC_LIBRARY}" PRIVATE ${COMMON_SOURCES_PATH}/_tlk_rf.c)
endif()

add_subdirectory(api)
add_subdirectory(cpp_wrappers)
