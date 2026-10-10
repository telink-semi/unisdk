#include "core/include/tlk_core.h"
#include "properties/tlk_chip.h"
#include "storage.h"
#include "storage_device.h"
#include "system/debug/log/tlk_log.h"

TLK_LOG_CREATE(nv_storage_demo, "NV_STORAGE_DEMO");

int main(void)
{
    tlk_core_interrupt_enable();

    TLK_LOG_INFO(nv_storage_demo, "----------------------------");
    TLK_LOG_INFO(nv_storage_demo, "available storage devices:");
    for (struct tlk_storage_device* dev = tlk_storage_get_next_device(NULL); dev != NULL;
         dev                            = tlk_storage_get_next_device(dev))
    {
        TLK_LOG_INFO(nv_storage_demo, "device: %s", dev->name);
        TLK_LOG_INFO(nv_storage_demo, "\tbase address: 0x%zx", dev->base);
        TLK_LOG_INFO(nv_storage_demo, "\tsize: %zu (0x%zx)", dev->size, dev->size);
        TLK_LOG_INFO(nv_storage_demo, "\tpage: %zu", dev->page);
        TLK_LOG_INFO(nv_storage_demo, "\tclean pattern: 0x%02x", dev->clean_byte);
        TLK_LOG_INFO(nv_storage_demo,
                     "\tcapable: %c%c%c",
                     dev->erase ? 'E' : '_',
                     dev->read ? 'R' : '_',
                     dev->write ? 'W' : '_');
    }
    TLK_LOG_INFO(nv_storage_demo, "----------------------------");

    do
    {
        int             result;
        static uint8_t  data[1024];
        const uintptr_t addr =
            UNISDK_CHIP_MEMORY_ROM_STARTADDR + UNISDK_CHIP_MEMORY_ROM_SIZE * 1024 - 5000;

        TLK_LOG_INFO(
            nv_storage_demo, "test storage access at 0x%zx with %zu bytes", addr, sizeof(data));

        struct tlk_storage_device* s_dev = tlk_storage_find_device(addr, sizeof(data));

        if (!s_dev)
        {
            TLK_LOG_INFO(nv_storage_demo, "error no storage device");
            break;
        }

        result = tlk_storage_read(addr, data, sizeof(data));
        if (!result)
        {
            TLK_LOG_INFO(nv_storage_demo, "%s[%zu]:", "storage_read", sizeof(data));
            TLK_LOG_HEXDUMP_INFO(nv_storage_demo, data, sizeof(data));
        }
        else
        {
            TLK_LOG_INFO(nv_storage_demo, "error storage_read: (%d)", result);
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

        result = tlk_storage_write(addr, data, sizeof(data));
        if (!result)
        {
            TLK_LOG_INFO(nv_storage_demo, "storage_write ok!");
        }
        else
        {
            TLK_LOG_INFO(nv_storage_demo, "error storage_write: (%d)", result);
            break;
        }

        result = tlk_storage_read(addr, data, sizeof(data));
        if (!result)
        {
            TLK_LOG_INFO(nv_storage_demo, "%s[%zu]:", "storage_read", sizeof(data));
            TLK_LOG_HEXDUMP_INFO(nv_storage_demo, data, sizeof(data));
        }
        else
        {
            TLK_LOG_INFO(nv_storage_demo, "error storage_read: (%d)", result);
            break;
        }
    } while (0);

    return 0;
}
