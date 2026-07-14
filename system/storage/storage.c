#include "storage.h"
#include "storage_device.h"
#include <errno.h>
#include <string.h>

static int storage_pages_modify(struct tlk_storage_device* dev, uintptr_t addr, const void* buf,
                                size_t len)
{
    int result = 0;

    while (len)
    {
        size_t page_offset = addr % dev->page;
        size_t page_len    = MIN(len, dev->page - page_offset);

        result = dev->read(dev, addr, &dev->buffer[page_offset], page_len);
        if (result)
        {
            break;
        }
        bool no_erase = true, same_data = true;

        for (size_t i = 0; (no_erase || same_data) && i < page_len; ++i)
        {
            uint8_t bt_old = dev->buffer[page_offset + i];
            uint8_t bt_new = buf ? ((const uint8_t*) buf)[i] : dev->clean_byte;

            if (same_data && (bt_old != bt_new))
            {
                same_data = false;
            }
            if (no_erase && (bt_new & (~bt_old)))
            {
                no_erase = false;
            }
        }
        if (!same_data)
        {
            if (!no_erase)
            {
                if (page_offset)
                {
                    result = dev->read(dev, addr - page_offset, dev->buffer, page_offset);
                    if (result)
                    {
                        break;
                    }
                }
                if (buf)
                {
                    memcpy(&dev->buffer[page_offset], buf, page_len);
                }
                else
                {
                    memset(&dev->buffer[page_offset], dev->clean_byte, page_len);
                }
                if (page_offset + page_len < dev->page)
                {
                    result = dev->read(dev,
                                       addr + page_len,
                                       &dev->buffer[page_offset + page_len],
                                       dev->page - (page_offset + page_len));
                    if (result)
                    {
                        break;
                    }
                }
                result = dev->erase(dev, addr - page_offset, dev->page);
                if (result)
                {
                    break;
                }
                /* erased, need to restore from buffer */
                /* remove tail and head clean bytes */
                size_t head_clean = 0, tail_clean = 0;
                bool   head_count = true, tail_count = true;

                for (size_t i = 0; (head_count || tail_count) && i < dev->page; ++i)
                {
                    if (dev->buffer[i] != dev->clean_byte)
                    {
                        head_count = false;
                    }
                    if (dev->buffer[dev->page - 1 - i] != dev->clean_byte)
                    {
                        tail_count = false;
                    }
                    if (head_count)
                    {
                        head_clean++;
                    }
                    if (tail_count)
                    {
                        tail_clean++;
                    }
                    if (head_clean + tail_clean >= dev->page)
                    {
                        /* empty array detected */
                        break;
                    }
                }
                if (head_clean + tail_clean < dev->page)
                {
                    result = dev->write(dev,
                                        addr - page_offset + head_clean,
                                        &dev->buffer[head_clean],
                                        dev->page - head_clean - tail_clean);
                    if (result)
                    {
                        break;
                    }
                }
            }
            else if (buf)
            {
                /* no erase, so just write required area */
                result = dev->write(dev, addr, buf, page_len);
                if (result)
                {
                    break;
                }
            }
        }
        len -= page_len;
        addr += page_len;
        if (buf)
        {
            buf = (const uint8_t*) buf + page_len;
        }
    }
    return result;
}

int storage_read(uintptr_t addr, void* buf, size_t len)
{
    int result;

    do
    {
        if (!buf || !len)
        {
            result = -EINVAL;
            break;
        }
        struct tlk_storage_device* dev = tlk_storage_device_find(addr, len);

        if (!dev)
        {
            result = -ENODEV;
            break;
        }
        if (!dev->read)
        {
            result = -ENOSYS;
            break;
        }
        result = dev->read(dev, addr, buf, len);
    } while (0);
    return result;
}

int storage_erase(uintptr_t addr, size_t len)
{
    int result;

    do
    {
        if (!len)
        {
            result = -EINVAL;
            break;
        }
        struct tlk_storage_device* dev = tlk_storage_device_find(addr, len);

        if (!dev)
        {
            result = -ENODEV;
            break;
        }
        if (!dev->page)
        {
            /* no page access, so just write area with clean bytes */
            if (!dev->write || !dev->buffer)
            {
                result = -ENOSYS;
                break;
            }

            memset(dev->buffer, dev->clean_byte, MIN(len, STORAGE_DEVICE_DEFAULT_BUFFER_SIZE));
            while (len)
            {
                size_t tmp_len = MIN(len, STORAGE_DEVICE_DEFAULT_BUFFER_SIZE);

                result = dev->write(dev, addr, dev->buffer, tmp_len);
                if (result)
                {
                    break;
                }
                len -= tmp_len;
                addr += tmp_len;
            }
        }
        else
        {
            /* page access erase required pages, keep head & tail if exists */
            if (!dev->read || !dev->buffer || !dev->erase || !dev->write)
            {
                result = -ENOSYS;
                break;
            }
            result = storage_pages_modify(dev, addr, NULL, len);
        }
    } while (0);
    return result;
}

int storage_write(uintptr_t addr, const void* buf, size_t len)
{
    int result;

    do
    {
        if (!buf || !len)
        {
            result = -EINVAL;
            break;
        }
        struct tlk_storage_device* dev = tlk_storage_device_find(addr, len);

        if (!dev)
        {
            result = -ENODEV;
            break;
        }
        if (!dev->page)
        {
            /* no page access, so just overwrite area with new bytes */
            if (!dev->write)
            {
                result = -ENOSYS;
                break;
            }
            result = dev->write(dev, addr, buf, len);
        }
        else
        {
            /* page access update required pages, keep head & tail if exists */
            if (!dev->read || !dev->buffer || !dev->erase || !dev->write)
            {
                result = -ENOSYS;
                break;
            }
            result = storage_pages_modify(dev, addr, buf, len);
        }
    } while (0);
    return result;
}
