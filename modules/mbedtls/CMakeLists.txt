set(USE_SHARED_MBEDTLS_LIBRARY OFF CACHE INTERNAL "" FORCE)
set(ENABLE_TESTING OFF CACHE INTERNAL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE INTERNAL "" FORCE)

set(GEN_FILES ON CACHE BOOL "" FORCE)

# tlk_mbedtls_config.h trims mbedtls down to just what MCUboot's PSA signature verification
# needs (SHA-256 + ECDSA-P256 over one curve, no TLS/RSA/other curves/PEM). Scoped to bootloader
# builds only -- any other consumer of this mbedtls target gets the unmodified upstream config.
if(CONFIG_TLK_MCUBOOT_MODE_BOOTLOADER)
	set(MBEDTLS_USER_CONFIG_FILE  "${CMAKE_CURRENT_SOURCE_DIR}/tlk_mbedtls_config.h"
		CACHE STRING "" FORCE
	)
	set(TF_PSA_CRYPTO_USER_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/tlk_mbedtls_config.h"
		CACHE STRING "" FORCE
	)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(repo)

target_sources(${TARGET_NAME} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_entropy.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_ecp_wrap.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_ecp_backend.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/mbedtls_multithread.c"
)

target_link_libraries(${TARGET_NAME} PRIVATE
	mbedtls
)

target_link_options(${TARGET_NAME} PRIVATE
	"-Wl,--wrap,mbedtls_ecp_check_pubkey"
	"-Wl,--wrap,mbedtls_ecp_mul"
	"-Wl,--wrap,mbedtls_ecp_mul_restartable"
	"-Wl,--wrap,mbedtls_ecp_muladd"
	"-Wl,--wrap,mbedtls_ecp_muladd_restartable"
	"-Wl,--wrap,mbedtls_ecp_gen_key"
	"-Wl,--wrap,mbedtls_ecp_gen_keypair"
	"-Wl,--wrap,mbedtls_ecp_gen_keypair_base"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=redundant-decls" CACHE STRING "" FORCE)
