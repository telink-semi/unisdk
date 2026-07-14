if (CONFIG_TLK_CRYPTO)

target_include_directories(${TARGET_NAME} PRIVATE ./include)

file(GLOB CRYPTO_TRNG_SRC "src/trng.c")
file(GLOB CRYPTO_PKE_SRC "src/pke/*.c")
file(GLOB CRYPTO_SKE_SRC "src/aes.c")

if (NOT IS_DIRECTORY ${TARGET_LIBRARY_DIR})
    add_sources("${CRYPTO_COMMON_SRC}")
    add_sources_ifdef(CONFIG_TLK_CRYPTO_TRNG "${CRYPTO_TRNG_SRC}")
    add_sources_ifdef(CONFIG_TLK_CRYPTO_PKE "${CRYPTO_PKE_SRC}")
    add_sources_ifdef(CONFIG_TLK_CRYPTO_SKE "${CRYPTO_SKE_SRC}")
    # Library target
    telink_libraries_include_directories(PRIVATE ./include)

    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${CRYPTO_COMMON_SRC})
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${CRYPTO_TRNG_SRC})
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${CRYPTO_PKE_SRC})
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${CRYPTO_SKE_SRC})
endif()
endif()
