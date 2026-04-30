#include "storage_device.h"

static DLIST_DEFINE(storage_device);

bool storage_device_exists(const struct storage_device *dev)
{
    bool result = false;

    for (struct storage_device *d = storage_device_get_next(NULL); d != NULL;
         d = storage_device_get_next(d)) {
        if (dev == d) {
            result = true;
            break;
        }
    }
    return result;
}

void storage_device_add(struct storage_device *dev)
{
    if (!storage_device_exists(dev)) {
        dlist_append(&storage_device, &dev->node);
    }
}

void storage_device_remove(struct storage_device *dev)
{
    if (storage_device_exists(dev)) {
        dlist_remove(&dev->node);
    }
}

struct storage_device *storage_device_get_next(const struct storage_device *dev)
{
    struct dnode *node_dev = dev ? dev->node.next : storage_device.next;

    return node_dev != &storage_device ?
           CONTAINER_OF(node_dev, struct storage_device, node) : NULL;
}

struct storage_device *storage_device_find(uintptr_t addr, size_t len)
{
    struct storage_device *dev = NULL;

    for (struct storage_device *d = storage_device_get_next(NULL); d != NULL;
         d = storage_device_get_next(d)) {
        if (addr >= d->base && addr + len <= d->base + d->size) {
            dev = d;
            break;
        }
    }
    return dev;
}
