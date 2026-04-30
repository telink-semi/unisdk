#include "registers/tlk_gpio.h"
#include "registers/tlk_analog_reg.h"

/**
 * @brief      This function servers to shut down all the GPIO when power on or wakeup from deep sleep.
 * @return     none.
 * @note       This function called by .S file to shutdown all the GPIO input which can decrease the current early.
 *             If this C function is called in the S file, it needs to be called after setting sp.
 *             In order to be compatible with flash_boot_ramcode.link, added a new code segment named flash_code.
 *             If this function compiled to the text segment, then it cannot be found.
 *             If this function compiled to the vectors segment, then it will at the starting address of vectors segment which can not be accepted.
*/
_tlk_attribute_ram_code_sec_noinline_ void tlk_gpio_shutdown_for_asm(void)
{
    //disable input
    tlk_gpio_reg.port_config[0].input_en = 0x80; // SWS pa_ie
    tlk_gpio_reg.port_config[1].input_en = 0x00; // pb_ie
    
    tlk_gpio_reg.port_config[4].input_en = 0x00; // pe_ie
    tlk_gpio_reg.port_config[5].input_en = 0x00; // pf_ie

    reg_rst1 |= FLD_RST1_ALGM;
    reg_clk_en1 |= FLD_CLK1_ALGM_EN;

    reg_ana_len     = 1;
    reg_ana_addr    = 0xbd;
    reg_ana_data(0) = 0x00;
    while (!(reg_ana_buf_cnt & FLD_ANA_TX_BUFCNT))
        ;
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    while (reg_ana_ctrl & FLD_ANA_BUSY)
        ;
    reg_ana_ctrl = 0x00;

    reg_ana_len     = 1;
    reg_ana_addr    = 0xc2;
    reg_ana_data(0) = 0x00;
    while (!(reg_ana_buf_cnt & FLD_ANA_TX_BUFCNT))
        ;
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    while (reg_ana_ctrl & FLD_ANA_BUSY)
        ;
    reg_ana_ctrl = 0x00;

    reg_rst1 &= ~(FLD_RST1_ALGM);
    reg_clk_en1 &= ~(FLD_CLK1_ALGM_EN);
}