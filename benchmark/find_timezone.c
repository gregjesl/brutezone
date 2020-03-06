#include "benchmark.h"
#include "timezone_database.h"
#include <stdio.h>
#include <time.h>

#define ROUNDS 1000
#define N_TIMEZONES sizeof(timezone_array) / sizeof(*timezone_array)

int main(int argc, char **argv)
{
    struct timespec bm_start;
    struct timespec bm_end;
    int rc;

    rc = clock_gettime(CLOCK_MONOTONIC, &bm_start);
    if (rc) {
        fprintf(stderr, "failed to get time");
        return 1;
    }

    for (unsigned int i = 0; i < ROUNDS; ++i) {
        for (unsigned int j = 0; j < N_TIMEZONES; ++j) {
            escape(find_timezone(timezone_array[j].name));
        }
    }

    rc = clock_gettime(CLOCK_MONOTONIC, &bm_end);
    if (rc) {
        fprintf(stderr, "failed to get time");
        return 1;
    }

    fprintf(stdout, "avg find_timezone duration in ns: %lu",
            timespec_diff_ns(&bm_start, &bm_end) / (ROUNDS * N_TIMEZONES));

    return 0;
}
