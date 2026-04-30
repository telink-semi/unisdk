#ifndef TLK_MTIMER_REGISTERS_H_
#define TLK_MTIMER_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t time_l; // address: 0xe6000000, offset: 0x0
    uint32_t time_h; // address: 0xe6000004, offset: 0x4
    uint32_t compare_l; // address: 0xe6000008, offset: 0x8
    uint32_t compare_h; // address: 0xe600000c, offset: 0xc
} tlk_mtimer_reg_t;

#define TLK_MTIMER_BASE_ADDR (0xe6000000U)
#define tlk_mtimer_reg (*(volatile tlk_mtimer_reg_t *) TLK_MTIMER_BASE_ADDR)

#endif

