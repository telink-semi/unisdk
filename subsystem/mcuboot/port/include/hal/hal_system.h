#ifndef HAL_HAL_SYSTEM_H
#define HAL_HAL_SYSTEM_H

#include "core/include/tlk_sys.h"

static inline void hal_system_reset(void)
{
    tlk_sys_reboot(TLK_SYS_SW_REBOOT_XTAL_UNSTABLE);
}

#endif /* HAL_HAL_SYSTEM_H */
