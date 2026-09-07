#include "common/include/tlk_init.h"
#include "core/include/tlk_analog.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_analog_reg.h"
#include "registers/tlk_gpio.h"
#include "registers/tlk_soc.h"

/**
 * @brief      This function servers to shut down all the GPIO when power on or wakeup from deep
 * sleep.
 * @return     none.
 * @note       This function called by .S file to shutdown all the GPIO input which can decrease the
 * current early. If this C function is called in the S file, it needs to be called after setting
 * sp. In order to be compatible with flash_boot_ramcode.link, added a new code segment named
 * flash_code. If this function compiled to the text segment, then it cannot be found. If this
 * function compiled to the vectors segment, then it will at the starting address of vectors segment
 * which can not be accepted.
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_gpio_shutdown(void)
{
    // disable input
    tlk_gpio_reg.port_config[0].input_en = 0x80;  // SWS reg_gpio_pa_ie
    tlk_gpio_reg.port_config[1].input_en &= 0xf0; // reg_gpio_pb_ie

    tlk_gpio_reg.port_config[4].input_en = 0x00; // reg_gpio_pd_ie
    tlk_gpio_reg.port_config[5].input_en = 0x00; // reg_gpio_pe_ie

    tlk_gpio_reg.port_config[0].output_en = 0xFF;
    tlk_gpio_reg.port_config[1].output_en = 0xFF;
    tlk_gpio_reg.port_config[2].output_en = 0xFF;
    tlk_gpio_reg.port_config[3].output_en = 0xFF;
    tlk_gpio_reg.port_config[4].output_en = 0xFF;
    tlk_gpio_reg.port_config[5].output_en = 0xFF;

    tlk_gpio_reg.port_config[0].function = 0x7F;
    tlk_gpio_reg.port_config[1].function = 0xFF;
    tlk_gpio_reg.port_config[2].function = 0xFF;
    tlk_gpio_reg.port_config[3].function = 0xFF;
    tlk_gpio_reg.port_config[4].function = 0xFF;
    tlk_gpio_reg.port_config[5].function = 0xFF;

    tlk_analog_write_reg8(0xbd, 0);
    tlk_analog_write_reg8(0xc0, 0);
}

// This function is a must for PM mode, because there is a current leakage via SWS pin. Need to be
// strictly pulled.
_tlk_attribute_ram_code_sec_noinline_ void tlk_gpio_sws_pullup(void)
{
    tlk_analog_write_reg8(TLK_AREG_PX_PUPD0 + 1,
                          (tlk_analog_read_reg8(TLK_AREG_PX_PUPD0 + 1) & 0x3F) | 0x40);
}
TLK_REGISTER_PRE_INIT(tlk_gpio_shutdown, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_HIGH)
TLK_REGISTER_PRE_INIT(tlk_gpio_sws_pullup, TLK_INIT_LEVEL_SYSTEM, TLK_INIT_PRIORITY_LOW)
