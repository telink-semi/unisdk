#ifndef TLK_UTILS_H
#define TLK_UTILS_H

#include <stddef.h>

#define CONTAINER_OF(ptr, type, field) \
    ((type *)(((char *)(ptr)) - offsetof(type, field)))

#define MIN(a, b)                   ((a) < (b) ? (a) : (b))
#define MAX(a, b)                   ((a) > (b) ? (a) : (b))

#define TLK_M2USEC(milliseconds) ((milliseconds) * 1000)
#define TLK_SEC(seconds) (seconds)
#define TLK_MSEC(seconds) ((seconds) * 1000)
#define TLK_USEC(seconds) ((TLK_M2USEC(seconds)) * 1000)

#endif /* TLK_COMMON_UTILS_H */
