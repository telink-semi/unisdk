#ifndef OS_OS_CPUTIME_H
#define OS_OS_CPUTIME_H

#include "core/include/tlk_sys.h"

static inline void os_cputime_delay_usecs(uint32_t usecs)
{
    tlk_sys_delay(usecs);
}

#endif /* OS_OS_CPUTIME_H */
