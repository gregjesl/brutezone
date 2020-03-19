#ifndef TIMEZONE_BENCHMARK_H
#define TIMEZONE_BENCHMARK_H

#include <time.h>

static inline void escape(const void *p)
{
    asm volatile("" : : "g"(p) : "memory");
}

static inline unsigned long int timespec_diff_ns(const struct timespec *start,
                                                 const struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1000000000 + end->tv_nsec -
           start->tv_nsec;
}

#endif
