#ifndef TLK_QDEC_REGISTERS_H_
#define TLK_QDEC_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
} tlk_qdec_reg_t;

#define TLK_QDEC_BASE_ADDR (0x80140240U)
#define tlk_qdec_reg (*(volatile tlk_qdec_reg_t *) TLK_QDEC_BASE_ADDR)

#endif

