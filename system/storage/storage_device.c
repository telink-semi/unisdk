#include "storage_device.h"
#include "common/include/tlk_utils.h"

static TLK_DLIST_DEFINE(storage_device);

bool tlk_storage_is_exist_device(const struct tlk_storage_device* dev)
{
    bool result = false;

    for (struct tlk_storage_device* d = tlk_storage_get_next_device(NULL); d != NULL;
         d                            = tlk_storage_get_next_device(d))
    {
        if (dev == d)
        {
            result = true;
            break;
        }
    }
    return result;
}

void tlk_storage_add_device(struct tlk_storage_device* dev)
{
    if (!tlk_storage_is_exist_device(dev))
    {
        tlk_dlist_append(&storage_device, &dev->node);
    }
}

void tlk_storage_remove_device(struct tlk_storage_device* dev)
{
    if (tlk_storage_is_exist_device(dev))
    {
        tlk_dlist_remove(&dev->node);
    }
}

struct tlk_storage_device* tlk_storage_get_next_device(const struct tlk_storage_device* dev)
{
    struct dnode* node_dev = dev ? dev->node.next : storage_device.next;

    return node_dev != &storage_device ? TLK_CONTAINER_OF(node_dev, struct tlk_storage_device, node)
                                       : NULL;
}

struct tlk_storage_device* tlk_storage_find_device(uintptr_t addr, size_t len)
{
    struct tlk_storage_device* dev = NULL;

    for (struct tlk_storage_device* d = tlk_storage_get_next_device(NULL); d != NULL;
         d                            = tlk_storage_get_next_device(d))
    {
        if (addr >= d->base && addr + len <= d->base + d->size)
        {
            dev = d;
            break;
        }
    }
    return dev;
}
