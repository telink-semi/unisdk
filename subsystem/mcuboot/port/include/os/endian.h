#ifndef OS_ENDIAN_H
#define OS_ENDIAN_H

#include <stdint.h>

static inline uint16_t htons(uint16_t x)
{
    return (uint16_t) ((x << 8) | (x >> 8));
}

#define ntohs(x) htons(x)

#endif /* OS_ENDIAN_H */
