#ifndef OS_OS_MALLOC_H
#define OS_OS_MALLOC_H

#include <stdlib.h>

#define os_malloc(size)       malloc(size)
#define os_realloc(ptr, size) realloc(ptr, size)
#define os_free(ptr)          free(ptr)

#endif /* OS_OS_MALLOC_H */
