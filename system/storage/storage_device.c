#include "storage_device.h"

static DLIST_DEFINE(storage_device);

bool tlk_storage_device_exists(const struct tlk_storage_device* dev)
{
    bool result = false;

    for (struct tlk_storage_device* d = tlk_storage_device_get_next(NULL); d != NULL;
         d                            = tlk_storage_device_get_next(d))
    {
        if (dev == d)
        {
            result = true;
            break;
        }
    }
    return result;
}

void tlk_storage_device_add(struct tlk_storage_device* dev)
{
    if (!tlk_storage_device_exists(dev))
    {
        dlist_append(&storage_device, &dev->node);
    }
}

void tlk_storage_device_remove(struct tlk_storage_device* dev)
{
    if (tlk_storage_device_exists(dev))
    {
        dlist_remove(&dev->node);
    }
}

struct tlk_storage_device* tlk_storage_device_get_next(const struct tlk_storage_device* dev)
{
    struct dnode* node_dev = dev ? dev->node.next : storage_device.next;

    return node_dev != &storage_device ? CONTAINER_OF(node_dev, struct tlk_storage_device, node)
                                       : NULL;
}

struct tlk_storage_device* tlk_storage_device_find(uintptr_t addr, size_t len)
{
    struct tlk_storage_device* dev = NULL;

    for (struct tlk_storage_device* d = tlk_storage_device_get_next(NULL); d != NULL;
         d                            = tlk_storage_device_get_next(d))
    {
        if (addr >= d->base && addr + len <= d->base + d->size)
        {
            dev = d;
            break;
        }
    }
    return dev;
}
