#ifndef __STORAGE_DEVICE_H
#define __STORAGE_DEVICE_H

#include "common/include/tlk_dlist.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define STORAGE_DEVICE_DEFAULT_BUFFER_SIZE 256

/*************************************************************************************
 * Storage Data
 * If page access (page grater than 0)
 * - storage_device_read (no restrictions)
 * - storage_device_erase (only accept: addr - page aligned, len - equal to page size)
 * - storage_device_write (only accept: addr & addr + len on same page)
 * If page access (page equal to 0)
 * - storage_device_read (no restrictions)
 * - storage_device_erase (no restrictions)
 * - storage_device_write (no restrictions)
 *************************************************************************************/

struct storage_device;

typedef int (*storage_device_read)(struct storage_device *dev,
                                   uintptr_t addr, void *buf, size_t len);
typedef int (*storage_device_erase)(struct storage_device *dev,
                                    uintptr_t addr, size_t len);
typedef int (*storage_device_write)(struct storage_device *dev,
                                    uintptr_t addr, const void *buf, size_t len);

struct storage_device {
    struct dnode node; /* internal use (do not touch it) */
    const char *name;
    const uintptr_t base;
    const size_t size;
    const size_t page;
    const uint8_t clean_byte;
    const storage_device_read read;
    const storage_device_erase erase;
    const storage_device_write write;
    uint8_t *const buffer;
    void *const user;
};

bool storage_device_exists(const struct storage_device *dev);
void storage_device_add(struct storage_device *dev);
void storage_device_remove(struct storage_device *dev);
struct storage_device *storage_device_get_next(const struct storage_device *dev);
struct storage_device *storage_device_find(uintptr_t addr, size_t len);

#define STORAGE_DEVICE_DEFINE(_name, _addr, _size, _page, _clean, _rd, _er, _wr, _user) \
    static uint8_t _name##_buffer[_page ? _page : STORAGE_DEVICE_DEFAULT_BUFFER_SIZE]; \
    static struct storage_device _name = { \
        .name = #_name, \
        .base = (_addr), \
        .size = (_size), \
        .page = (_page), \
        .clean_byte = (_clean), \
        .read = (_rd), \
        .erase = (_page) ? (_er) : NULL, \
        .write = (_wr), \
        .buffer = _name##_buffer, \
        .user = (_user), \
    }

#endif /* __STORAGE_DEVICE_H */
