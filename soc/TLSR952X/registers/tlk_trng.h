#ifndef TLK_TRNG_REGISTERS_H_
#define TLK_TRNG_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
} tlk_trng_reg_t;

#define TLK_TRNG_BASE_ADDR (0x80101800U)
#define tlk_trng_reg (*(volatile tlk_trng_reg_t *) TLK_TRNG_BASE_ADDR)

#endif

