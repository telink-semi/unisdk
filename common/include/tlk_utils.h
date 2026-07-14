#ifndef TLK_UTILS_H
#define TLK_UTILS_H

#include <stddef.h>

#define CONTAINER_OF(ptr, type, field) ((type*) (((char*) (ptr)) - offsetof(type, field)))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define TLK_SEC_TO_US(s) ((s) * 1000000U) // Seconds to microseconds
#define TLK_SEC_TO_MS(s) ((s) * 1000U)    // Seconds to milliseconds
#define TLK_MS_TO_US(ms) ((ms) * 1000U)   // Milliseconds to microseconds
#define TLK_US_TO_MS(us) ((us) / 1000U)   // Microseconds to milliseconds

#endif /* TLK_COMMON_UTILS_H */
