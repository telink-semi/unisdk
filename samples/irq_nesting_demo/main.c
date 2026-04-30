// TODO: test demo configs
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_plic.h"
#include "core/include/tlk_plic_sw.h" 
#include "core/include/tlk_clic.h"
#include "core/include/tlk_mtimer.h"
#include "tlk_core.h"
#include "common/include/tlk_utils.h"
#include "api/include/tlk_time.h"
#include "system/debug/log/tlk_log.h"
#include "tlk_board_pinout.h"
#include <string.h>

#define MTIME_VALUE   100

TLK_LOG_CREATE(irq_demo, "IRQ Nesting DEMO");

/* Redefinition of the interrupt handler for software IRQ */
void tlk_mswi_irq_handler(void)
{
    TLK_LOG_INFO(irq_demo, "Enter software IRQ handler");

/* Check if nesting into software interrups is enabled */
#if IS_ENABLED(CONFIG_TLK_PLIC_SW_ENABLE_NESTING) & IS_ENABLED(CONFIG_TLK_NEST_INTO_SW)
#if IS_ENABLED(CONFIG_TLK_NEST_EXT)
    /* Toggle GPIO pin to generate GPIO interrupt for PLIC */
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
    tlk_api_delay(TLK_USEC(1));
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
#endif /* CONFIG_TLK_NEST_EXT & CONFIG_TLK_NEST_INTO_SW */

#if IS_ENABLED(CONFIG_TLK_NEST_MTIMER)
    tlk_mtimer_set_mtime_compare(MTIME_VALUE);
#endif /* CONFIG_TLK_NEST_MTIMER */
#endif /* CONFIG_TLK_PLIC_SW_ENABLE_NESTING */

    TLK_LOG_INFO(irq_demo, "Leave software IRQ handler");
}

void gpio_cb(enum tlk_gpio_port port __unused, enum tlk_gpio_pin pin __unused)
{
    TLK_LOG_INFO(irq_demo, "Enter GPIO IRQ handler");

/* Check if nesting into software interrups is enabled */
#if IS_ENABLED(CONFIG_TLK_PLIC_ENABLE_NESTING) & IS_ENABLED(CONFIG_TLK_NEST_INTO_EXT)
#if IS_ENABLED(CONFIG_TLK_NEST_SW)
    tlk_plic_sw_set_pending();
#endif /* CONFIG_TLK_NEST_SW */

#if IS_ENABLED(CONFIG_TLK_NEST_MTIMER)
    tlk_mtimer_set_mtime_compare(MTIME_VALUE);
#endif /* CONFIG_TLK_NEST_MTIMER */
#endif /* CONFIG_TLK_PLIC_ENABLE_NESTING & CONFIG_TLK_NEST_INTO_EXT */

    TLK_LOG_INFO(irq_demo, "Leave GPIO IRQ handler");
}

/* Redefine mtimer IRQ handler */
void mtime_irq_handler(void)
{
    TLK_LOG_INFO(irq_demo, "Enter mtimer IRQ handler");
    
    tlk_mtimer_set_mtime_compare(0xffffffffffffffff);
#if IS_ENABLED(CONFIG_TLK_MTIMER_ENABLE_NESTING) & IS_ENABLED(CONFIG_TLK_NEST_INTO_MTIMER)
#if IS_ENABLED(CONFIG_TLK_NEST_EXT)
    /* Toggle GPIO pin to generate GPIO interrupt for PLIC */
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
    tlk_api_delay(TLK_USEC(1));
    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
#endif /* CONFIG_TLK_NEST_EXT */

#if IS_ENABLED(CONFIG_TLK_NEST_SW)
    tlk_plic_sw_set_pending();
#endif /* CONFIG_TLK_NEST_SW */
#endif /* CONFIG_TLK_MTIMER_ENABLE_NESTING & CONFIG_TLK_NEST_INTO_MTIMER */

    TLK_LOG_INFO(irq_demo, "Leave mtimer IRQ handler");
}

int main(void)
{
    tlk_gpio_configure(UNISDK_BOARD_KEY_0_PORT, UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INPUT_PULL_DOWN);
    tlk_gpio_configure(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_pin_write(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, 1);

    tlk_plic_sw_interrupt_enable();

    struct tlk_gpio_irq_callback callback = {
        .handler = gpio_cb,
        .pin = UNISDK_BOARD_KEY_0_PIN
    };

    tlk_gpio_irq_add_callback(UNISDK_BOARD_KEY_1_PORT, &callback);
    tlk_gpio_irq_configure(UNISDK_BOARD_KEY_0_PORT, UNISDK_BOARD_KEY_0_PIN, TLK_GPIO_INTR_RISING_EDGE);
    tlk_mtimer_register_callback(mtime_irq_handler);

    tlk_clic_enable_interrupt_source(TLK_FLD_MIE_MTIE);
    tlk_clic_enable_interrupt_source(TLK_FLD_MIE_MSIE);
    tlk_clic_enable_interrupt_source(TLK_FLD_MIE_MEIE);

    tlk_core_interrupt_enable();
    TLK_LOG_INFO(irq_demo, "Configuration successful!");

    while(1) {
#if IS_ENABLED(CONFIG_TLK_NEST_INTO_SW)
        tlk_plic_sw_set_pending();
#elif IS_ENABLED(CONFIG_TLK_NEST_INTO_EXT)
        tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
        tlk_api_delay(TLK_USEC(1));
        tlk_gpio_pin_toggle(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN);
#elif IS_ENABLED(CONFIG_TLK_NEST_INTO_MTIMER)
        tlk_mtimer_set_mtime_compare(MTIME_VALUE);
#endif
        TLK_LOG_INFO(irq_demo,"\n");
        tlk_api_delay(TLK_USEC(3));
    }
}
