#ifndef TLK_TICK_H
#define TLK_TICK_H

#include "core/include/tlk_clock.h"
#include "properties/tlk_clock.h"

#define TLK_DIV_ROUND_UP(value, divisor) (((value) + (divisor) - 1ULL) / (divisor))

#define TLK_TICK_HF_RC_TICKS_FROM_US(us)                                                           \
    TLK_DIV_ROUND_UP((uint64_t) (us) * (uint64_t) UNISDK_STIMER_CLOCK, 1000000ULL)

#define TLK_TICK_HF_RC_TICKS_FROM_MS(ms)                                                           \
    TLK_DIV_ROUND_UP((uint64_t) (ms) * (uint64_t) UNISDK_STIMER_CLOCK, 1000ULL)

#define TLK_TICK_HF_RC_TICKS_FROM_S(s) (uint64_t) (s) * (uint64_t) UNISDK_STIMER_CLOCK

#define TLK_TICK_32K_TICKS_FROM_US(us)                                                             \
    TLK_DIV_ROUND_UP((uint64_t) (us) * ((tlk_clock_32k_get_source() == TLK_CLOCK_32K_XTAL)         \
                                            ? UNISDK_CLOCK_32K_XTAL_FREQ                           \
                                            : UNISDK_CLOCK_32K_RC_FREQ),                           \
                     1000000ULL)

#define TLK_TICK_32K_TICKS_FROM_MS(ms)                                                             \
    TLK_DIV_ROUND_UP((uint64_t) (ms) * ((tlk_clock_32k_get_source() == TLK_CLOCK_32K_XTAL)         \
                                            ? UNISDK_CLOCK_32K_XTAL_FREQ                           \
                                            : UNISDK_CLOCK_32K_RC_FREQ),                           \
                     1000ULL)

#define TLK_TICK_32K_TICKS_FROM_S(s)                                                               \
    ((uint64_t) (s) * ((tlk_clock_32k_get_source() == TLK_CLOCK_32K_XTAL)                          \
                           ? UNISDK_CLOCK_32K_XTAL_FREQ                                            \
                           : UNISDK_CLOCK_32K_RC_FREQ))

#endif
