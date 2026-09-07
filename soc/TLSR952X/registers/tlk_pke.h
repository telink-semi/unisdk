#ifndef TLK_TLK_PKE_REGISTERS_H_
#define TLK_TLK_PKE_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef struct __attribute__((packed)) {
} tlk_pke_reg_t;

#define TLK_PKE_BASE_ADDR (0x80110000U)
#define tlk_pke_reg (*(volatile tlk_pke_reg_t *) TLK_PKE_BASE_ADDR)

#endif