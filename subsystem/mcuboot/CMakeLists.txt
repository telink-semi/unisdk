# SPDX-License-Identifier: Apache-2.0

if(CONFIG_TLK_MCUBOOT_MODE_BOOTLOADER)
    set(BOOTUTIL_DIR ${TELINK_BASE}/modules/mcuboot/repo/boot/bootutil)
    set(BOOT_SERIAL_DIR ${TELINK_BASE}/modules/mcuboot/repo/boot/boot_serial)
    set(ZCBOR_DIR ${TELINK_BASE}/modules/mcuboot/repo/boot/zcbor)

    target_include_directories(${TARGET_NAME} PRIVATE
        include
        port/include
        ${BOOTUTIL_DIR}/include
        ${BOOT_SERIAL_DIR}/include
        ${ZCBOR_DIR}/include
    )

    target_sources(${TARGET_NAME} PRIVATE
        src/tlk_mcuboot.c
        port/flash_map_backend.c
        port/keys.c
    )

    if(CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY)
        target_sources(${TARGET_NAME} PRIVATE port/uart_boot_serial.c)
    endif()

    set(MCUBOOT_VENDORED_SOURCES
        ${BOOTUTIL_DIR}/src/bootutil_misc.c
        ${BOOTUTIL_DIR}/src/bootutil_public.c
        ${BOOTUTIL_DIR}/src/image_validate.c
        ${BOOTUTIL_DIR}/src/image_ecdsa.c
        ${BOOTUTIL_DIR}/src/loader.c
        ${BOOTUTIL_DIR}/src/swap_misc.c
        ${BOOTUTIL_DIR}/src/swap_scratch.c
        ${BOOTUTIL_DIR}/src/tlv.c
        ${BOOTUTIL_DIR}/src/fault_injection_hardening.c
    )

    if(CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY)
        list(APPEND MCUBOOT_VENDORED_SOURCES
            ${BOOT_SERIAL_DIR}/src/boot_serial.c
            ${BOOT_SERIAL_DIR}/src/zcbor_bulk.c
            ${ZCBOR_DIR}/src/zcbor_common.c
            ${ZCBOR_DIR}/src/zcbor_decode.c
            ${ZCBOR_DIR}/src/zcbor_encode.c
        )
    endif()

    set(MCUBOOT_PORT_SERIAL_SOURCES)
    if(CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY)
        set(MCUBOOT_PORT_SERIAL_SOURCES port/crc16.c port/base64.c)
    endif()

    add_library(mcuboot_bootutil STATIC
        ${MCUBOOT_VENDORED_SOURCES}
        ${MCUBOOT_PORT_SERIAL_SOURCES}
        port/mcuboot_boot_init.c
    )

    target_include_directories(mcuboot_bootutil PRIVATE
        $<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>
        port/include
        ${BOOTUTIL_DIR}/include
        ${BOOT_SERIAL_DIR}/include
        ${ZCBOR_DIR}/include
    )

    target_compile_options(mcuboot_bootutil PRIVATE
        "--imacros=${AUTOCONF_H}"
        "--imacros=${TELINK_BASE}/common/include/tlk_soc_addr.h"
        "--imacros=${TELINK_BASE}/common/include/tlk_bit.h"
        "--imacros=${TELINK_BASE}/common/include/tlk_compiler.h"
        "--imacros=${TELINK_BASE}/common/include/tlk_config_macro.h"
        "-include" "${TELINK_BASE}/common/include/tlk_types.h"
    )
    target_link_libraries(mcuboot_bootutil PRIVATE mbedtls)

    target_link_libraries(${TARGET_NAME} PRIVATE mcuboot_bootutil)
endif()

if(CONFIG_TLK_MCUBOOT_MODE_APP)
    set(BOOTUTIL_DIR ${TELINK_BASE}/modules/mcuboot/repo/boot/bootutil)

    target_include_directories(${TARGET_NAME} PRIVATE
        port/include
        ${BOOTUTIL_DIR}/include
    )

    target_sources(${TARGET_NAME} PRIVATE
        port/flash_map_backend.c
    )

    add_library(mcuboot_bootutil_public STATIC
        ${BOOTUTIL_DIR}/src/bootutil_public.c
    )

    target_include_directories(mcuboot_bootutil_public PRIVATE
        $<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>
        port/include
        ${BOOTUTIL_DIR}/include
    )
    target_compile_options(mcuboot_bootutil_public PRIVATE
        "--imacros=${AUTOCONF_H}"
        "--imacros=${TELINK_BASE}/common/include/tlk_soc_addr.h"
        "--imacros=${TELINK_BASE}/common/include/tlk_bit.h"
        "--imacros=${TELINK_BASE}/common/include/tlk_compiler.h"
        "--imacros=${TELINK_BASE}/common/include/tlk_config_macro.h"
        "-include" "${TELINK_BASE}/common/include/tlk_types.h"
    )

    target_link_libraries(${TARGET_NAME} PRIVATE mcuboot_bootutil_public)
endif()
