#ifndef __STORAGE_DEVICE_H
#define __STORAGE_DEVICE_H

#include "common/include/tlk_dlist.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define STORAGE_DEVICE_DEFAULT_BUFFER_SIZE 256

/*************************************************************************************
 * Storage Data
 * If page access (page grater than 0)
 * - tlk_storage_device_read (no restrictions)
 * - tlk_storage_device_erase (only accept: addr - page aligned, len - equal to page size)
 * - tlk_storage_device_write (only accept: addr & addr + len on same page)
 * If page access (page equal to 0)
 * - tlk_storage_device_read (no restrictions)
 * - tlk_storage_device_erase (no restrictions)
 * - tlk_storage_device_write (no restrictions)
 *************************************************************************************/

struct tlk_storage_device;

typedef int (*tlk_storage_device_read)(struct tlk_storage_device* dev, uintptr_t addr, void* buf,
                                       size_t len);
typedef int (*tlk_storage_device_erase)(struct tlk_storage_device* dev, uintptr_t addr, size_t len);
typedef int (*tlk_storage_device_write)(struct tlk_storage_device* dev, uintptr_t addr,
                                        const void* buf, size_t len);

struct tlk_storage_device
{
    struct dnode                   node; /* internal use (do not touch it) */
    const char*                    name;
    const uintptr_t                base;
    const size_t                   size;
    const size_t                   page;
    const uint8_t                  clean_byte;
    const tlk_storage_device_read  read;
    const tlk_storage_device_erase erase;
    const tlk_storage_device_write write;
    uint8_t* const                 buffer;
    void* const                    user;
};

bool                       tlk_storage_is_exist_device(const struct tlk_storage_device* dev);
void                       tlk_storage_add_device(struct tlk_storage_device* dev);
void                       tlk_storage_remove_device(struct tlk_storage_device* dev);
struct tlk_storage_device* tlk_storage_get_next_device(const struct tlk_storage_device* dev);
struct tlk_storage_device* tlk_storage_find_device(uintptr_t addr, size_t len);

#define TLK_STORAGE_DEFINE_DEVICE(_name, _addr, _size, _page, _clean, _rd, _er, _wr, _user)        \
    static uint8_t _name##_buffer[_page ? _page : STORAGE_DEVICE_DEFAULT_BUFFER_SIZE];             \
    static struct tlk_storage_device _name = {                                                     \
        .name       = #_name,                                                                      \
        .base       = (_addr),                                                                     \
        .size       = (_size),                                                                     \
        .page       = (_page),                                                                     \
        .clean_byte = (_clean),                                                                    \
        .read       = (_rd),                                                                       \
        .erase      = (_page) ? (_er) : NULL,                                                      \
        .write      = (_wr),                                                                       \
        .buffer     = _name##_buffer,                                                              \
        .user       = (_user),                                                                     \
    }

#endif /* __STORAGE_DEVICE_H */
