#ifndef MCUBOOT_CONFIG_H
#define MCUBOOT_CONFIG_H

#include "properties/tlk_chip.h"

#define MCUBOOT_SIGN_EC256
#define MCUBOOT_USE_PSA_CRYPTO
#define MCUBOOT_SHA256

#define MCUBOOT_IMAGE_NUMBER 1

#define MCUBOOT_HAVE_LOGGING 1

#define MCUBOOT_USE_FLASH_AREA_GET_SECTORS

#if defined(CONFIG_TLK_MCUBOOT_MODE_BOOTLOADER) && (UNISDK_CHIP_MEMORY_DRAM_SIZE <= 32) &&         \
    (CONFIG_TLK_MCUBOOT_SLOT_SIZE == 0x000B0000)
#define MCUBOOT_PORT_SLOT_SIZE 0x20000U
#else
#define MCUBOOT_PORT_SLOT_SIZE CONFIG_TLK_MCUBOOT_SLOT_SIZE
#endif

#define MCUBOOT_MAX_IMG_SECTORS (MCUBOOT_PORT_SLOT_SIZE / 4096U)

#ifdef CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY
#define MCUBOOT_SERIAL
#define MCUBOOT_SERIAL_RECOVERY

#define MCUBOOT_SERIAL_DIRECT_IMAGE_UPLOAD

#define MCUBOOT_SERIAL_IMG_GRP_HASH
#define MCUBOOT_SERIAL_IMG_GRP_IMAGE_STATE

#define MCUBOOT_PERUSER_MGMT_GROUP_ENABLED 0
#endif

#define MCUBOOT_WATCHDOG_FEED()                                                                    \
    do                                                                                             \
    {                                                                                              \
    } while (0)
#define MCUBOOT_CPU_IDLE()                                                                         \
    do                                                                                             \
    {                                                                                              \
    } while (0)

#endif /* MCUBOOT_CONFIG_H */
