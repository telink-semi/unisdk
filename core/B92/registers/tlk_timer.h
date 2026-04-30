#ifndef TLK_TIMER_REGISTERS_H_
#define TLK_TIMER_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
} tlk_timer_reg_t;

#define TLK_TIMER_BASE_ADDR (0x80140140U)
#define tlk_timer_reg (*(volatile tlk_timer_reg_t *) TLK_TIMER_BASE_ADDR)

#endif

