#include "properties/tlk_chip.h"
#include "system/debug/log/tlk_log.h"

extern char  _end;
static char* heap_end = 0;

TLK_LOG_CREATE(mem_log, "MEM POOL");

#ifdef CONFIG_TLK_PM_RETENTION_MEMORY_SIZE
#define MEM_AVAILABLE_AREA CONFIG_TLK_PM_RETENTION_MEMORY_SIZE
#else
#if TLK_IS_ENABLED(CONFIG_TLK_PM_RAM_RETENTION_ENABLE)
#define MEM_AVAILABLE_AREA UNISDK_CHIP_MEMORY_IRAM_SIZE
#else
#define MEM_AVAILABLE_AREA UNISDK_CHIP_MEMORY_DRAM_SIZE
#endif
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PM_RAM_RETENTION_ENABLE)
#define HEAP_BANK_ADDR UNISDK_CHIP_MEMORY_IRAM_STARTADDR
#else
#define HEAP_BANK_ADDR UNISDK_CHIP_MEMORY_DRAM_STARTADDR
#endif

#define CHIP_HEAP_BANK_END_ADDR (MEM_AVAILABLE_AREA * 1024) + HEAP_BANK_ADDR

void* _sbrk(ptrdiff_t incr)
{
    if (heap_end == 0)
        heap_end = &_end;

    char* prev_heap_end = heap_end;

    if ((uint32_t) (heap_end + incr) > CHIP_HEAP_BANK_END_ADDR)
    {
        TLK_LOG_ERROR(mem_log,
                      "Dynamic memory out of range: requested: %d, available: %d",
                      incr,
                      CHIP_HEAP_BANK_END_ADDR - (uint32_t) heap_end);
        return (void*) -1;
    }

    heap_end += incr;
    return prev_heap_end;
}
