#ifndef __STORAGE_H
#define __STORAGE_H

#include <stddef.h>
#include <stdint.h>

int storage_read(uintptr_t addr, void* buf, size_t len);
int storage_erase(uintptr_t addr, size_t len);
int storage_write(uintptr_t addr, const void* buf, size_t len);

#endif /* __STORAGE_H */
