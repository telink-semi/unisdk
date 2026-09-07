#ifndef TLK_BENCHMARK_H_
#define TLK_BENCHMARK_H_

#include "api/include/tlk_time.h"

#define TLK_BENCHMARK(code, postaction)                                                            \
    do                                                                                             \
    {                                                                                              \
        struct tlk_benchmark benchmark;                                                            \
        tlk_benchmark_begin(&benchmark);                                                           \
        TLK_DEBRACKET code tlk_benchmark_end(&benchmark);                                          \
        TLK_DEBRACKET      postaction                                                              \
    } while (0);

typedef uint32_t tlk_benchmark_time_t;

struct tlk_benchmark_span
{
    tlk_benchmark_time_t begin;
    tlk_benchmark_time_t end;
};

static inline void tlk_benchmark_begin(struct tlk_benchmark_span* span)
{
    span->begin = tlk_api_time_get_micros();
}

static inline void tlk_benchmark_end(struct tlk_benchmark_span* span)
{
    span->end = tlk_api_time_get_micros();
}

static inline tlk_benchmark_time_t tlk_benchmark_elapsed(struct tlk_benchmark_span* span)
{
    return span->end - span->begin;
}

#endif
