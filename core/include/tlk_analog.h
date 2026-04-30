#ifndef TLK_INCLUDE_DRIVERS_ANALOG_H_
#define TLK_INCLUDE_DRIVERS_ANALOG_H_

/**
 * @brief      This function serves to analog register read by byte.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
_tlk_attribute_ram_code_sec_ uint8_t tlk_analog_read_reg8(uint8_t addr);

/**
 * @brief      This function serves to analog register write by byte.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
_tlk_attribute_ram_code_sec_ void tlk_analog_write_reg8(uint8_t addr, uint8_t data);

/**
 * @brief      This function serves to analog register read by halfword.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
_tlk_attribute_ram_code_sec_ uint16_t tlk_analog_read_reg16(uint8_t addr);

/**
 * @brief      This function serves to analog register write by halfword.
 * @param[in]  addr - address need to be write.
 * @param[in]  data - the value need to be write.
 * @return     none.
 */
_tlk_attribute_ram_code_sec_ void tlk_analog_write_reg16(uint8_t addr, uint16_t data);

/**
 * @brief      This function serves to analog register read by word.
 * @param[in]  addr - address need to be read.
 * @return     the result of read.
 */
_tlk_attribute_ram_code_sec_ uint32_t tlk_analog_read_reg32(uint8_t addr);

/**
  * @brief      This function serves to analog register write by word.
  * @param[in]  addr - address need to be write.
  * @param[in]  data - the value need to be write.
  * @return     none.
  */
_tlk_attribute_ram_code_sec_ void tlk_analog_write_reg32(uint8_t addr, uint32_t data);

#endif