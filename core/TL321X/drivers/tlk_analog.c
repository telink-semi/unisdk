#include "core/include/tlk_analog.h"
#include "core/include/tlk_core.h"
#include "registers/tlk_analog_reg.h"
/**
 * @brief      This function serves to judge whether analog write/read is busy .
 * @return     none.
 */
_tlk_attribute_ram_code_sec_ static void tlk_analog_wait(void)
{
    while (reg_ana_ctrl & FLD_ANA_BUSY)
    {
    }
}

/**
 * @brief      This function serves to judge whether analog Tx buffer is empty.
 * @return     none.
 */
_tlk_attribute_ram_code_sec_ static void tlk_analog_wait_txbuf_no_empty(void)
{
    /**
        Because the logic of the current chip hardware to write the first data is:
        regardless of whether there is data in the FIFO, the write operation will be performed
       immediately. In order to prevent incorrect data from being emitted due to the slow filling of
       the FIFO, we need to determine whether the FIFO is non-empty before triggering the write
       action.
    */
    while (!(reg_ana_buf_cnt & FLD_ANA_TX_BUFCNT))
    {
    }
}

/* Analog driver API */

_tlk_attribute_ram_code_sec_noinline_ uint8_t tlk_analog_read_reg8(uint8_t addr)
{
    uint32_t r   = tlk_core_interrupt_disable();
    reg_ana_addr = addr;
    reg_ana_len  = 1;
    reg_ana_ctrl = FLD_ANA_CYC;
    tlk_analog_wait();
    uint8_t data = reg_ana_data(0);
    tlk_core_interrupt_restore(r);
    return data;
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_analog_write_reg8(uint8_t addr, uint8_t data)
{
    uint32_t r      = tlk_core_interrupt_disable();
    reg_ana_len     = 1;
    reg_ana_addr    = addr;
    reg_ana_data(0) = data;
    tlk_analog_wait_txbuf_no_empty();
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    tlk_analog_wait();
    reg_ana_ctrl = 0x00;
    tlk_core_interrupt_restore(r);
}

_tlk_attribute_ram_code_sec_noinline_ uint16_t tlk_analog_read_reg16(uint8_t addr)
{
    uint32_t r   = tlk_core_interrupt_disable();
    reg_ana_len  = 2;
    reg_ana_addr = addr;
    reg_ana_ctrl = FLD_ANA_CYC;
    tlk_analog_wait();
    uint16_t data = reg_ana_addr_data16;
    tlk_core_interrupt_restore(r);
    return data;
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_analog_write_reg16(uint8_t addr, uint16_t data)
{
    uint32_t r          = tlk_core_interrupt_disable();
    reg_ana_len         = 2;
    reg_ana_addr        = addr;
    reg_ana_addr_data16 = data;
    tlk_analog_wait_txbuf_no_empty();
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    tlk_analog_wait();
    reg_ana_ctrl = 0x00;
    tlk_core_interrupt_restore(r);
}

_tlk_attribute_ram_code_sec_noinline_ uint32_t tlk_analog_read_reg32(uint8_t addr)
{
    uint32_t r   = tlk_core_interrupt_disable();
    reg_ana_len  = 4;
    reg_ana_addr = addr;
    reg_ana_ctrl = FLD_ANA_CYC;
    tlk_analog_wait();
    uint32_t data = reg_ana_addr_data32;
    tlk_core_interrupt_restore(r);
    return data;
}

_tlk_attribute_ram_code_sec_noinline_ void tlk_analog_write_reg32(uint8_t addr, uint32_t data)
{
    uint32_t r          = tlk_core_interrupt_disable();
    reg_ana_len         = 4;
    reg_ana_addr        = addr;
    reg_ana_addr_data32 = data;
    tlk_analog_wait_txbuf_no_empty();
    reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);
    tlk_analog_wait();
    reg_ana_ctrl = 0x00;
    tlk_core_interrupt_restore(r);
}
