
#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_core.h"
#include "properties/tlk_chip.h"
#include "storage.h"
#include "storage_device.h"
#include "system/debug/log/tlk_log.h"
#ifdef __cplusplus
}
#endif

TLK_LOG_CREATE(nv_storage_demo, "NV_STORAGE_DEMO");

static void tlk_print_buf(const char* comment, void* buf, size_t len)
{
    (void) comment;
    (void) buf;
    TLK_LOG_INFO(nv_storage_demo, "%s[%zu]:", comment, len);
    for (size_t i = 0; i < len; ++i)
    {
        TLK_LOG_INFO(nv_storage_demo, " %02x ", ((uint8_t*) buf)[i]);
    }
    TLK_LOG_INFO(nv_storage_demo, "\n");
}

int main(void)
{
    tlk_core_interrupt_enable();

    TLK_LOG_INFO(nv_storage_demo, "available storage devices:\n");
    for (struct tlk_storage_device* dev = tlk_storage_device_get_next(NULL); dev != NULL;
         dev                            = tlk_storage_device_get_next(dev))
    {
        TLK_LOG_INFO(nv_storage_demo, "device: %s\n", dev->name);
        TLK_LOG_INFO(nv_storage_demo, "\tbase address: 0x%zx\n", dev->base);
        TLK_LOG_INFO(nv_storage_demo, "\tsize: %zu (0x%zx)\n", dev->size, dev->size);
        TLK_LOG_INFO(nv_storage_demo, "\tpage: %zu\n", dev->page);
        TLK_LOG_INFO(nv_storage_demo, "\tclean pattern: 0x%02x\n", dev->clean_byte);
        TLK_LOG_INFO(nv_storage_demo,
                     "\tcapable: %c%c%c\n",
                     dev->erase ? 'E' : '_',
                     dev->read ? 'R' : '_',
                     dev->write ? 'W' : '_');
    }
    TLK_LOG_INFO(nv_storage_demo, "--------------------------\n");

    do
    {
        int             result;
        static uint8_t  data[1024];
        const uintptr_t addr =
            UNISDK_CHIP_MEMORY_ROM_STARTADDR + UNISDK_CHIP_MEMORY_ROM_SIZE * 1024 - 5000;

        /* Now next line should be commented again */
        // TLK_LOG_INFO(
        //     nv_storage_demo, "test storage access at 0x%zx with %zu bytes\n", addr,
        //     sizeof(data));

        struct tlk_storage_device* s_dev = tlk_storage_device_find(addr, sizeof(data));

        if (!s_dev)
        {
            TLK_LOG_INFO(nv_storage_demo, "error no storage device\n");
            break;
        }

        result = storage_read(addr, data, sizeof(data));
        if (!result)
        {
            tlk_print_buf("storage_read", data, sizeof(data));
        }
        else
        {
            TLK_LOG_INFO(nv_storage_demo, "error storage_read: (%d)\n", result);
            break;
        }

        bool is_clean = true;

        for (size_t i = 0; i < sizeof(data); ++i)
        {
            if (data[i] != s_dev->clean_byte)
            {
                is_clean = false;
                break;
            }
        }
        for (size_t i = 0; i < sizeof(data); ++i)
        {
            if (is_clean)
            {
                data[i] = i;
            }
            else
            {
                data[i]++;
            }
        }

        result = storage_write(addr, data, sizeof(data));
        if (!result)
        {
            TLK_LOG_INFO(nv_storage_demo, "storage_write ok!\n");
        }
        else
        {
            TLK_LOG_INFO(nv_storage_demo, "error storage_write: (%d)\n", result);
            break;
        }

        result = storage_read(addr, data, sizeof(data));
        if (!result)
        {
            tlk_print_buf("storage_read", data, sizeof(data));
        }
        else
        {
            TLK_LOG_INFO(nv_storage_demo, "error storage_read: (%d)\n", result);
            break;
        }
    } while (0);

    return 0;
}
