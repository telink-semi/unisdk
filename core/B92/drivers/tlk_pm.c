/********************************************************************************************************
 * @file    tlk_pm.h
 *
 * @brief   This is the header file for B92
 *
 * @author  Driver Group
 * @date    2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

#include "core/include/tlk_pm.h"
#include "common/include/tlk_init.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_stimer.h"
#include "properties/tlk_plic.h"
#include "properties/tlk_pm.h"
#include "registers/tlk_analog.h"

static volatile struct
{
    enum tlk_pm_wakeup_source wakeup_sources;
    bool                      was_in_retention;
} tlk_pm_sleep_info;

union tlk_areg_ret_ctrl tlk_pm_sleep_mode_to_reg_value[] = {
    TLK_IF_ENABLED(CONFIG_TLK_PM_SUSPEND, ({.raw = UNISDK_PM_BFLD_SUSPEND_MODE}, ))
        TLK_IF_ENABLED(CONFIG_TLK_PM_DEEP_SLEEP, ({.raw = UNISDK_PM_BFLD_DEEP_SLEEP_MODE}, ))
            TLK_IF_ENABLED(CONFIG_TLK_PM_RETENTION_MEMORY_SIZE_32K,
                           ({.raw = UNISDK_PM_BFLD_RETENTION_MODE_SIZE_32K}, ))
                TLK_IF_ENABLED(CONFIG_TLK_PM_RETENTION_MEMORY_SIZE_64K,
                               ({.raw = UNISDK_PM_BFLD_RETENTION_MODE_SIZE_64K}, ))
                    TLK_IF_ENABLED(CONFIG_TLK_PM_RETENTION_MEMORY_SIZE_96K,
                                   ({.raw = UNISDK_PM_BFLD_RETENTION_MODE_SIZE_96K}, ))};

/* Helper functions */
extern void tlk_context_save(void);
extern _tlk_attribute_ram_code_sec_noinline_ enum tlk_pm_sleep_status
tlk_pm_sleep_impl(enum tlk_pm_sleep_mode mode, uint32_t duration_ms);

_tlk_attribute_ram_code_sec_ enum tlk_pm_wakeup_source _tlk_pm_get_desired_wakeup_source(void)
{
    return tlk_pm_sleep_info.wakeup_sources;
}

/* PM driver API */
_tlk_attribute_ram_code_sec_ enum tlk_pm_wakeup_source tlk_pm_get_wakeup_reason(void)
{
    return tlk_analog_read_reg8(TLK_AREG_WAKEUP_STATUS);
}

void tlk_pm_clear_wakeup_sources(void)
{
    tlk_pm_sleep_info.wakeup_sources = TLK_PM_WAKEUP_SOURCE_NONE;

    /* Manually clear the GPIO wakeup sources */
#if TLK_IS_ENABLED(CONFIG_TLK_GPIO)
    tlk_analog_write_reg32(TLK_AREG_PX_WAKEUP_EN(0), 0);
    tlk_analog_write_reg16(TLK_AREG_PX_WAKEUP_EN(4), 0);
#endif
}

_tlk_attribute_ram_code_sec_noinline_ enum tlk_pm_sleep_status
tlk_pm_sleep(enum tlk_pm_sleep_mode mode, uint32_t duration_ms)
{
    enum tlk_pm_sleep_status status                  = TLK_PM_SLEEP_OK;
    bool                     did_fallback_to_suspend = false;

    /* Determine whether timer wakeup is used */
    if (duration_ms == UINT32_MAX)
    {
        if (tlk_pm_sleep_info.wakeup_sources == TLK_PM_WAKEUP_SOURCE_NONE)
        {
            return TLK_PM_SLEEP_NO_WAKEUP_SOURCES;
        }
        else
        {
            tlk_pm_sleep_info.wakeup_sources &= ~TLK_PM_WAKEUP_SOURCE_TIMER;
        }
    }
    else
    {
        tlk_pm_sleep_info.wakeup_sources |= TLK_PM_WAKEUP_SOURCE_TIMER;
    }

    /* Validate the sleep duration */
    if (tlk_pm_sleep_info.wakeup_sources & TLK_PM_WAKEUP_SOURCE_TIMER)
    {
        /* 7/8 cycle of 32bit, 178*7/8 = 156 S */
        if (duration_ms * TLK_SYSTEM_TIMER_TICK_1MS > UNISDK_PM_MAX_SLEEP_DURATION_TIMER_TICKS)
        {
            return TLK_PM_SLEEP_TOO_LONG;
        }

        switch (mode)
        {
#if TLK_IS_ENABLED(CONFIG_TLK_PM_SUSPEND)

#if (CONFIG_TLK_PM_SUSPEND_MIN_DURATION_MS < CONFIG_TLK_PM_SUSPEND_WAKEUP_COST_MS)
#error "Minimal suspend time can not be less than wakeup cost"
#endif

        case TLK_PM_SLEEP_MODE_SUSPEND:
            if (duration_ms <=
                CONFIG_TLK_PM_SUSPEND_WAKEUP_COST_MS + CONFIG_TLK_PM_SUSPEND_MIN_DURATION_MS)
                return TLK_PM_SLEEP_TOO_SHORT;
            else
                duration_ms -= CONFIG_TLK_PM_SUSPEND_WAKEUP_COST_MS;
            break;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEEP_SLEEP)
        case TLK_PM_SLEEP_MODE_DEEP_SLEEP:
            if (duration_ms < CONFIG_TLK_PM_DEEP_SLEEP_MIN_DURATION_MS)
                return TLK_PM_SLEEP_TOO_SHORT;
            break;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PM_RAM_RETENTION_ENABLE)
#if (CONFIG_TLK_PM_DEEP_RETENTION_MIN_DURATION_MS < CONFIG_TLK_PM_DEEP_RETENTION_WAKEUP_COST_MS)
#error "Minimal deep retention time can not be less than wakeup cost"
#endif
        case TLK_PM_SLEEP_MODE_DEEP_RETENTION:
            if (duration_ms <= CONFIG_TLK_PM_DEEP_RETENTION_WAKEUP_COST_MS +
                                   CONFIG_TLK_PM_DEEP_RETENTION_MIN_DURATION_MS)
                return TLK_PM_SLEEP_TOO_SHORT;
            else
                duration_ms -= CONFIG_TLK_PM_DEEP_RETENTION_WAKEUP_COST_MS;
            break;
#endif

        default:
            // means no real sleep
            return TLK_PM_SLEEP_TOO_SHORT;
            break;
        }
    }

/* Check if suspend/sleep is allowed */
#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEEP_SLEEP)
    if (mode == TLK_PM_SLEEP_MODE_DEEP_SLEEP
#if TLK_IS_ENABLED(CONFIG_TLK_PM_RAM_RETENTION_ENABLE)
        || mode == TLK_PM_SLEEP_MODE_DEEP_RETENTION
#endif
    )
    {
        if (tlk_prevent_sleep())
        {
#if TLK_IS_ENABLED(CONFIG_TLK_PM_SUSPEND) && TLK_IS_ENABLED(CONFIG_TLK_PM_ALLOW_FALLBACK_TO_SUSPEND)
            did_fallback_to_suspend = true;
            mode                    = TLK_PM_SLEEP_MODE_SUSPEND;
#else
            return TLK_PM_SLEEP_DENIED;
#endif
        }
    }
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_PM_SUSPEND)
    if (mode == TLK_PM_SLEEP_MODE_SUSPEND)
    {
        if (tlk_prevent_suspend())
        {
            return TLK_PM_SLEEP_DENIED;
        }
    }
#endif

    /* Go sleep */
    TLK_DISABLE_BTB;

    switch (mode)
    {
#if TLK_IS_ENABLED(CONFIG_TLK_PM_SUSPEND)
    case TLK_PM_SLEEP_MODE_SUSPEND:
        tlk_before_suspend();
        status = tlk_pm_sleep_impl(mode, duration_ms);
        tlk_after_suspend();
        break;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PM_DEEP_SLEEP)
    case TLK_PM_SLEEP_MODE_DEEP_SLEEP:
        status = tlk_pm_sleep_impl(mode, duration_ms);
        break;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_PM_RAM_RETENTION_ENABLE)
    case TLK_PM_SLEEP_MODE_DEEP_RETENTION:
        tlk_before_sleep();
        tlk_context_save();
        /* In case of retention wakeup we will continue execution from here */
        if (!tlk_pm_sleep_info.was_in_retention)
        {
            /* Set the flag to continue execution from the else branch after the wakeup */
            tlk_pm_sleep_info.was_in_retention = true;

            status = tlk_pm_sleep_impl(mode, duration_ms);

            /* If the reboot did not happen for some reason, we will continue from here */
        }
        else
        {
            /* Restore the context */
            tlk_after_sleep();

            status = TLK_PM_SLEEP_OK;
        }
        tlk_pm_sleep_info.was_in_retention = false;

        break;
#endif

    default:
        break;
    }

    TLK_ENABLE_BTB;

    if (did_fallback_to_suspend)
    {
        status = TLK_PM_SLEEP_SUSPEND_FALLBACK;
    }

    return status;
}

#if TLK_IS_ENABLED(CONFIG_TLK_GPIO)

enum tlk_pm_sleep_status tlk_pm_set_gpio_wakeup(enum tlk_gpio_port port, enum tlk_gpio_pin pin,
                                                enum tlk_pm_gpio_wakeup_level polarity)
{
    TLK_ANALOG_MODIFY_RAW8(
        TLK_AREG_PX_POLARITY(port),
        (if (polarity) { TLK_BM_CLR(value, pin); } else { TLK_BM_SET(value, pin); }));

    TLK_ANALOG_MODIFY_RAW8(TLK_AREG_PX_WAKEUP_EN(port), (TLK_BM_SET(value, pin);));

    tlk_pm_sleep_info.wakeup_sources |= TLK_PM_WAKEUP_SOURCE_PAD;

    return TLK_PM_SLEEP_OK;
}

#endif
