#ifndef SOC_ADDR_H
#define SOC_ADDR_H

#include "properties/tlk_chip.h"

#define REG_ADDR8(a)          (*(volatile unsigned char *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (a)))
#define REG_ADDR16(a)         (*(volatile unsigned short *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (a)))
#define REG_ADDR32(a)         (*(volatile unsigned long *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (a)))

#define write_reg8(addr, v)   (*(volatile unsigned char *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (addr)) = (unsigned char)(v))
#define write_reg16(addr, v)  (*(volatile unsigned short *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (addr)) = (unsigned short)(v))
#define write_reg32(addr, v)  (*(volatile unsigned long *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (addr)) = (unsigned long)(v))

#define read_reg8(addr)       (*(volatile unsigned char *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (addr)))
#define read_reg16(addr)      (*(volatile unsigned short *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (addr)))
#define read_reg32(addr)      (*(volatile unsigned long *)(UNISDK_CHIP_MEMORY_REG_STARTADDR | (addr)))

#define write_sram8(addr, v)  (*(volatile unsigned char *)((addr)) = (unsigned char)(v))
#define write_sram16(addr, v) (*(volatile unsigned short *)((addr)) = (unsigned short)(v))
#define write_sram32(addr, v) (*(volatile unsigned long *)((addr)) = (unsigned long)(v))

#define read_sram8(addr)      (*(volatile unsigned char *)((addr)))
#define read_sram16(addr)     (*(volatile unsigned short *)((addr)))
#define read_sram32(addr)     (*(volatile unsigned long *)((addr)))

#endif
