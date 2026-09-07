#ifndef TLK_INCLUDE_DRIVERS_SYS_H_
#define TLK_INCLUDE_DRIVERS_SYS_H_

#define _TLK_ASM_NOP_        __asm__ __volatile__("nop");
#define TLK_CLOCK_DLY_8_CYC  TLK_FOR(8, _TLK_ASM_NOP_)
#define TLK_CLOCK_DLY_64_CYC TLK_FOR(8, TLK_CLOCK_DLY_8_CYC)

#include "core/include/tlk_power.h"

enum tlk_sys_reboot_reason
{
    TLK_SYS_REBOOT_REASON_UNKNOWN        = 0,
    TLK_SYS_REBOOT_REASON_POR            = TLK_BIT(0),
    TLK_SYS_REBOOT_REASON_SW             = TLK_BIT(1),
    TLK_SYS_REBOOT_REASON_WDT            = TLK_BIT(2),
    TLK_SYS_REBOOT_REASON_DEEP_SLEEP     = TLK_BIT(3),
    TLK_SYS_REBOOT_REASON_DEEP_RETENTION = TLK_BIT(4),
};

enum tlk_sys_sw_reboot_reason
{
    TLK_SYS_SW_REBOOT_XTAL_UNSTABLE = 0x00,
    TLK_SYS_SW_REBOOT_PLL_UNSTABLE  = 0x01,
};

/**
 * @brief Get the reason for the last software-initiated reboot.
 *
 * @return The software reboot reason code.
 */
enum tlk_sys_sw_reboot_reason tlk_sys_get_sw_reboot_reason(void);
/**
 * @brief Get the reason for the last system reboot.
 *
 * @return The reboot reason (POR, software, watchdog, deep sleep, etc.).
 */
enum tlk_sys_reboot_reason tlk_sys_get_reboot_reason(void);

/**
 * @brief Reboot the system.
 *
 * @param reason The software reboot reason to report.
 *
 * @return None.
 */
void tlk_sys_reboot(enum tlk_sys_sw_reboot_reason reason);

// TODO: Check, if we need this API, because the usec delay is already provided in API module
/**
 * @brief Delay execution for a specified duration.
 *
 * @param duration_us Delay duration in microseconds.
 *
 * @return None.
 */
void tlk_sys_delay(uint32_t duration_us);

/**
 * @brief Initialize the system with the given power configuration.
 *
 * @param power_mode Power mode (LDO-only, DCDC+LDO, or DCDC-only).
 * @param vbat_v     VBAT voltage range configuration.
 * @param gpio_v     GPIO voltage level (3.3V or 1.8V).
 * @param cap        Crystal capacitor configuration (internal or external).
 *
 * @return None.
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_sys_init(enum tlk_power_mode         power_mode,
                                                        enum tlk_power_vbat         vbat_v,
                                                        enum tlk_power_gpio_voltage gpio_v,
                                                        enum tlk_power_cap          cap);

#endif
