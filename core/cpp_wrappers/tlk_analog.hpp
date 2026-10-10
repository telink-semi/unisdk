#ifndef ANALOG_DRIVER_HPP
#define ANALOG_DRIVER_HPP

#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

#include "core/include/tlk_analog.h"

#ifdef __cplusplus
}
#endif

class AnalogDriver
{
  public:
    explicit AnalogDriver(uint8_t addr) : addr(addr) {}

    _tlk_attribute_ram_code_sec_ void writeReg8(uint32_t data)
    {
        tlk_analog_write_reg8(addr, data);
    }
    _tlk_attribute_ram_code_sec_ uint16_t readReg16()
    {
        return tlk_analog_read_reg16(addr);
    }
    _tlk_attribute_ram_code_sec_ void writeReg16(uint16_t data)
    {
        tlk_analog_write_reg16(addr, data);
    }
    _tlk_attribute_ram_code_sec_ uint32_t readReg32(uint8_t addr)
    {
        return tlk_analog_read_reg32(addr);
    }
    _tlk_attribute_ram_code_sec_ void writeReg32(uint32_t data)
    {
        tlk_analog_write_reg32(addr, data);
    }

  private:
    uint8_t addr;
};

#endif // ANALOG_DRIVER_HPP
