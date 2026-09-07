file(GLOB CRYPTO_SKE_SRC "src/aes.c")
file(GLOB CRYPTO_PKE_SRC "src/pke/*.c")
file(GLOB CRYPTO_TRNG_SRC "src/trng.c")

if(NOT IS_SDK_MODE)
    add_sources_ifdef(CONFIG_TLK_CRYPTO_SKE "${CRYPTO_SKE_SRC}")
    add_sources_ifdef(CONFIG_TLK_CRYPTO_PKE "${CRYPTO_PKE_SRC}")
    add_sources_ifdef(CONFIG_TLK_CRYPTO_TRNG "${CRYPTO_TRNG_SRC}")

    # Library target
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${CRYPTO_SKE_SRC})
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${CRYPTO_PKE_SRC})
    target_sources(${TARGET_SOC_LIBRARY} PRIVATE ${CRYPTO_TRNG_SRC})
endif()
