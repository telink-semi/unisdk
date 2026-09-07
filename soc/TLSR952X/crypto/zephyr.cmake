# SPDX-License-Identifier: Apache-2.0
#
# UniSDK Zephyr module — TLSR952X crypto
#
# Mirrors soc/TLSR952X/crypto/CMakeLists.txt.
#
# Variables expected from the caller:
#   SOC_CHIP_BASE  - base path for this SoC chip

if(CONFIG_TLK_CRYPTO)

# Source globs — matching the native CMakeLists.txt patterns
file(GLOB CRYPTO_COMMON_SRC "${SOC_CHIP_BASE}/crypto/src/crypto_common/*.c")
file(GLOB CRYPTO_TRNG_SRC  "${SOC_CHIP_BASE}/crypto/src/trng.c")
file(GLOB CRYPTO_PKE_SRC   "${SOC_CHIP_BASE}/crypto/src/pke/*.c")
file(GLOB CRYPTO_SKE_SRC   "${SOC_CHIP_BASE}/crypto/src/aes.c")

# Common sources — always compiled when CONFIG_TLK_CRYPTO is enabled
zephyr_sources(${CRYPTO_COMMON_SRC})

# Conditional crypto modules
zephyr_sources_ifdef(CONFIG_TLK_CRYPTO_TRNG "${CRYPTO_TRNG_SRC}")
zephyr_sources_ifdef(CONFIG_TLK_CRYPTO_PKE  "${CRYPTO_PKE_SRC}")
zephyr_sources_ifdef(CONFIG_TLK_CRYPTO_SKE  "${CRYPTO_SKE_SRC}")

endif()
