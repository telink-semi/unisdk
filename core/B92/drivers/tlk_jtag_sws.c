#include "common/include/tlk_init.h"
#include "core/include/tlk_gpio.h"

// TODO: switch to PINMUX defines

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_JTAG)

/**
 * @brief Enable a JTAG debug pins.
 *
 * @param None.
 *
 * @return None.
 */
void tlk_jtag_enable(void)
{
    tlk_gpio_configure(UNISDK_JTAG_TDI_PORT, UNISDK_JTAG_TDI_PIN, TLK_GPIO_INPUT_PULL_DOWN_100K);
    tlk_gpio_set_mux(UNISDK_JTAG_TDI_PORT, UNISDK_JTAG_TDI_PIN, 0);
    tlk_gpio_disable(UNISDK_JTAG_TDI_PORT, UNISDK_JTAG_TDI_PIN);

    tlk_gpio_configure(UNISDK_JTAG_TDO_PORT, UNISDK_JTAG_TDO_PIN, TLK_GPIO_INPUT_NO_PULL);
    tlk_gpio_set_mux(UNISDK_JTAG_TDO_PORT, UNISDK_JTAG_TDO_PIN, 0);
    tlk_gpio_disable(UNISDK_JTAG_TDO_PORT, UNISDK_JTAG_TDO_PIN);

    tlk_gpio_configure(UNISDK_JTAG_TMS_PORT, UNISDK_JTAG_TMS_PIN, TLK_GPIO_INPUT_PULL_UP_10K);
    tlk_gpio_set_mux(UNISDK_JTAG_TMS_PORT, UNISDK_JTAG_TMS_PIN, 0);
    tlk_gpio_disable(UNISDK_JTAG_TMS_PORT, UNISDK_JTAG_TMS_PIN);

    tlk_gpio_configure(UNISDK_JTAG_TCK_PORT, UNISDK_JTAG_TCK_PIN, TLK_GPIO_INPUT_PULL_UP_10K);
    tlk_gpio_set_mux(UNISDK_JTAG_TCK_PORT, UNISDK_JTAG_TCK_PIN, 0);
    tlk_gpio_disable(UNISDK_JTAG_TCK_PORT, UNISDK_JTAG_TCK_PIN);
}
/**
 * @brief Enable an SDP debug pins.
 *
 * @param None.
 *
 * @return None.
 */
void tlk_sdp_enable(void)
{
    tlk_gpio_configure(UNISDK_JTAG_TMS_PORT, UNISDK_JTAG_TMS_PIN, TLK_GPIO_INPUT_PULL_UP_10K);
    tlk_gpio_set_mux(UNISDK_JTAG_TMS_PORT, UNISDK_JTAG_TMS_PIN, 0);
    tlk_gpio_disable(UNISDK_JTAG_TMS_PORT, UNISDK_JTAG_TMS_PIN);

    tlk_gpio_configure(UNISDK_JTAG_TCK_PORT, UNISDK_JTAG_TCK_PIN, TLK_GPIO_INPUT_PULL_UP_10K);
    tlk_gpio_set_mux(UNISDK_JTAG_TCK_PORT, UNISDK_JTAG_TCK_PIN, 0);
    tlk_gpio_disable(UNISDK_JTAG_TCK_PORT, UNISDK_JTAG_TCK_PIN);
}

void tlk_enable_debug_interface(void)
{
    tlk_gpio_configure(TLK_GPIO_PORT_B, TLK_GPIO_PIN_0, TLK_GPIO_INPUT_NO_PULL);
    if (tlk_gpio_pin_read(TLK_GPIO_PORT_B, TLK_GPIO_PIN_0))
    {
        // true, 2 wire
        tlk_sdp_enable();
    }
    else
    {
        // false, 4 wire
        tlk_jtag_enable();
    }
    tlk_gpio_disable(TLK_GPIO_PORT_B, TLK_GPIO_PIN_0);
}

TLK_REGISTER_PRE_INIT(tlk_enable_debug_interface, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_SWS)
/**
 * @brief Enable an SWS debug pin.
 *
 * @param None.
 *
 * @return None.
 */
void tlk_sws_enable(void)
{
    tlk_gpio_configure(UNISDK_SWS_PORT, UNISDK_SWS_PIN, TLK_GPIO_INPUT_NO_PULL);
}

TLK_REGISTER_PRE_INIT(tlk_sws_enable)
#endif
