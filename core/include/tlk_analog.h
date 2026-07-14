#ifndef TLK_INCLUDE_DRIVERS_ANALOG_H_
#define TLK_INCLUDE_DRIVERS_ANALOG_H_
#include <stdint.h>

#define _TLK_AREG_TYPE(reg) tlk_areg_##reg
#define TLK_AREG_TYPE(reg)  _TLK_AREG_TYPE(reg)
#define _TLK_AREG_SIZE(reg) TLK_AREG_##reg##_SIZE
#define TLK_AREG_SIZE(reg)  _TLK_AREG_SIZE(reg)

#define ___TLK_ANALOG_MODIFY(reg, size, statement)                                                 \
    do                                                                                             \
    {                                                                                              \
        (void) reg;                                                                                \
        TLK_AREG_TYPE(reg) value = (TLK_AREG_TYPE(reg)) tlk_analog_read_reg##size(reg);            \
        TLK_DEBRACKET statement tlk_analog_write_reg##size(reg, value.raw);                        \
    } while (0)

#define __TLK_ANALOG_MODIFY(reg, size, statement) ___TLK_ANALOG_MODIFY(reg, size, statement)
#define _TLK_ANALOG_MODIFY(reg, statement) __TLK_ANALOG_MODIFY(reg, TLK_AREG_SIZE(reg), statement)
#define TLK_ANALOG_MODIFY(reg, statement)  _TLK_ANALOG_MODIFY(reg, statement)

#define _TLK_ANALOG_MODIFY_RAW(reg, size, statement)                                               \
    do                                                                                             \
    {                                                                                              \
        uint##size##_t          value = tlk_analog_read_reg##size(reg);                            \
        TLK_DEBRACKET statement tlk_analog_write_reg##size(reg, value);                            \
    } while (0)

#define TLK_ANALOG_MODIFY_RAW(reg, size, statement) _TLK_ANALOG_MODIFY_RAW(reg, size, statement)
#define TLK_ANALOG_MODIFY_RAW8(reg, statement)      TLK_ANALOG_MODIFY_RAW(reg, 8, statement)
#define TLK_ANALOG_MODIFY_RAW16(reg, statement)     TLK_ANALOG_MODIFY_RAW(reg, 16, statement)
#define TLK_ANALOG_MODIFY_RAW32(reg, statement)     TLK_ANALOG_MODIFY_RAW(reg, 32, statement)

#define ___TLK_ANALOG_READ(reg, size)                                                              \
    ((void) reg, (TLK_AREG_TYPE(reg)) tlk_analog_read_reg##size(reg))
#define __TLK_ANALOG_READ(reg, size) ___TLK_ANALOG_READ(reg, size)
#define _TLK_ANALOG_READ(reg)        __TLK_ANALOG_READ(reg, TLK_AREG_SIZE(reg))
#define TLK_ANALOG_READ(reg)         _TLK_ANALOG_READ(reg)

#define ___TLK_ANALOG_WRITE(reg, var, size)                                                        \
    tlk_analog_write_reg##size(reg, ((TLK_AREG_TYPE(reg)) var).raw)
#define __TLK_ANALOG_WRITE(reg, var, size) ___TLK_ANALOG_WRITE(reg, var, size)
#define _TLK_ANALOG_WRITE(reg, var)        __TLK_ANALOG_WRITE(reg, var, TLK_AREG_SIZE(reg))
#define TLK_ANALOG_WRITE(reg, var)         _TLK_ANALOG_WRITE(reg, var)

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
