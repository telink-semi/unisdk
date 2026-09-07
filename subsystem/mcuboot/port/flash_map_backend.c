#include "include/flash_map_backend/flash_map_backend.h"
#include "include/sysflash/sysflash.h"
#include "mcuboot_config/mcuboot_config.h"
#include "properties/tlk_chip.h"
#include "storage.h"
#include <stdbool.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define FLASH_SECTOR_SIZE 4096U
#define FLASH_WRITE_ALIGN 1U
#define FLASH_ERASED_VAL  0xffU

#if (CONFIG_TLK_MCUBOOT_BOOTLOADER_SIZE % FLASH_SECTOR_SIZE) != 0
#error "CONFIG_TLK_MCUBOOT_BOOTLOADER_SIZE must be a multiple of the flash sector size (4096 bytes)"
#endif
#if (MCUBOOT_PORT_SLOT_SIZE % FLASH_SECTOR_SIZE) != 0
#error                                                                                             \
    "MCUBOOT_PORT_SLOT_SIZE (see mcuboot_config.h) must be a multiple of the flash sector size (4096 bytes)"
#endif
#if (CONFIG_TLK_MCUBOOT_SCRATCH_SIZE % FLASH_SECTOR_SIZE) != 0
#error "CONFIG_TLK_MCUBOOT_SCRATCH_SIZE must be a multiple of the flash sector size (4096 bytes)"
#endif

static const struct flash_area partitions[] = {
    {
        .fa_id        = FLASH_AREA_IMAGE_PRIMARY(0),
        .fa_device_id = 0,
        .fa_off       = UNISDK_CHIP_MEMORY_ROM_STARTADDR + CONFIG_TLK_MCUBOOT_BOOTLOADER_SIZE,
        .fa_size      = MCUBOOT_PORT_SLOT_SIZE,
    },
    {
        .fa_id        = FLASH_AREA_IMAGE_SECONDARY(0),
        .fa_device_id = 0,
        .fa_off       = UNISDK_CHIP_MEMORY_ROM_STARTADDR + CONFIG_TLK_MCUBOOT_BOOTLOADER_SIZE +
                        MCUBOOT_PORT_SLOT_SIZE,
        .fa_size      = MCUBOOT_PORT_SLOT_SIZE,
    },
    {
        .fa_id        = FLASH_AREA_IMAGE_SCRATCH,
        .fa_device_id = 0,
        .fa_off       = UNISDK_CHIP_MEMORY_ROM_STARTADDR + CONFIG_TLK_MCUBOOT_BOOTLOADER_SIZE +
                        (2 * MCUBOOT_PORT_SLOT_SIZE),
        .fa_size      = CONFIG_TLK_MCUBOOT_SCRATCH_SIZE,
    },
    {
        .fa_id        = FLASH_AREA_BOOTLOADER,
        .fa_device_id = 0,
        .fa_off       = UNISDK_CHIP_MEMORY_ROM_STARTADDR,
        .fa_size      = CONFIG_TLK_MCUBOOT_BOOTLOADER_SIZE,
    }};

int flash_area_open(uint8_t id, const struct flash_area** fa)
{
    if (fa == NULL)
    {
        return -1;
    }

    *fa = NULL;

    for (uint32_t i = 0; i < ARRAY_SIZE(partitions); i++)
    {
        if (partitions[i].fa_id == id)
        {
            *fa = &partitions[i];
            return 0;
        }
    }

    return -1;
}
void flash_area_close(const struct flash_area* fa)
{
    (void) fa;
}

static bool flash_area_range_is_valid(const struct flash_area* fa, uint32_t off, uint32_t len)
{
    return (fa != NULL) && (len != 0U) && (off <= fa->fa_size) && (len <= fa->fa_size - off);
}

int flash_area_read(const struct flash_area* fa, uint32_t off, void* dst, uint32_t len)
{
    if (dst == NULL || !flash_area_range_is_valid(fa, off, len))
    {
        return -1;
    }

    return tlk_storage_read(fa->fa_off + off, dst, len);
}
int flash_area_write(const struct flash_area* fa, uint32_t off, const void* src, uint32_t len)
{
    if (src == NULL || !flash_area_range_is_valid(fa, off, len))
    {
        return -1;
    }

    return tlk_storage_write(fa->fa_off + off, src, len);
}
int flash_area_erase(const struct flash_area* fa, uint32_t off, uint32_t len)
{
    if (!flash_area_range_is_valid(fa, off, len))
    {
        return -1;
    }

    return tlk_storage_erase(fa->fa_off + off, len);
}

uint32_t flash_area_align(const struct flash_area* fa)
{
    (void) fa;
    return FLASH_WRITE_ALIGN;
}
uint8_t flash_area_erased_val(const struct flash_area* fa)
{
    (void) fa;
    return (uint8_t) FLASH_ERASED_VAL;
}
int flash_area_get_sectors(int fa_id, uint32_t* count, struct flash_sector* sectors)
{
    const struct flash_area* fa;

    if (count == NULL || sectors == NULL || flash_area_open((uint8_t) fa_id, &fa) != 0)
    {
        return -1;
    }

    uint32_t sector_count = fa->fa_size / FLASH_SECTOR_SIZE;
    uint32_t max          = *count;

    for (uint32_t i = 0; i < sector_count && i < max; i++)
    {
        sectors[i].fs_off  = i * FLASH_SECTOR_SIZE;
        sectors[i].fs_size = FLASH_SECTOR_SIZE;
    }

    *count = sector_count;
    return 0;
}

int flash_area_id_from_image_slot(int slot)
{
    return slot;
}
int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    if (image_index != 0)
    {
        return -1;
    }

    return slot;
}
int flash_area_id_to_multi_image_slot(int image_index, int area_id)
{
    if (image_index != 0)
    {
        return -1;
    }

    return area_id;
}

int flash_area_id_from_direct_image(int image_id)
{
    switch (image_id)
    {
    case 0: /* No explicit selection -- same as 1. */
    case 1:
        return FLASH_AREA_IMAGE_PRIMARY(0);
    case 2:
        return FLASH_AREA_IMAGE_SECONDARY(0);
    default:
        return -1;
    }
}
