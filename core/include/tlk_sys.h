#ifndef TLK_INCLUDE_DRIVERS_SYS_H_
#define TLK_INCLUDE_DRIVERS_SYS_H_

#define _TLK_ASM_NOP_       __asm__ __volatile__("nop");
#define TLK_CLOCK_DLY_8_CYC FOR(8, _TLK_ASM_NOP_)
#define TLK_CLOCK_DLY_64_CYC FOR(8, TLK_CLOCK_DLY_8_CYC)

#include "core/include/tlk_power.h"

enum tlk_sys_reboot_reason {
    TLK_SYS_REBOOT_REASON_UNKNOWN = 0,
    TLK_SYS_REBOOT_REASON_POR = TLK_BIT(0),
    TLK_SYS_REBOOT_REASON_SW = TLK_BIT(1),
    TLK_SYS_REBOOT_REASON_WDT = TLK_BIT(2),
    TLK_SYS_REBOOT_REASON_DEEP_SLEEP = TLK_BIT(3),
    TLK_SYS_REBOOT_REASON_DEEP_RETENTION = TLK_BIT(4),
};

enum tlk_sys_sw_reboot_reason {
    TLK_SYS_SW_REBOOT_XTAL_UNSTABLE = 0x00,
    TLK_SYS_SW_REBOOT_PLL_UNSTABLE = 0x01,
};

enum tlk_sys_sw_reboot_reason tlk_sys_get_sw_reboot_reason(void);
enum tlk_sys_reboot_reason tlk_sys_get_reboot_reason(void);

void tlk_sys_reboot(enum tlk_sys_sw_reboot_reason reason);

// TODO: Check, if we need this API, because the usec delay is already provided in API module
void tlk_sys_delay(unsigned int duration_us);

_tlk_attribute_ram_code_sec_noinline_ void tlk_sys_init(enum tlk_power_mode power_mode, enum tlk_power_vbat vbat_v, enum tlk_power_gpio_voltage gpio_v, enum tlk_power_cap cap);

_tlk_attribute_ram_code_sec_noinline_ void tlk_sys_update_reboot_reason(unsigned char clr_en);

#endif