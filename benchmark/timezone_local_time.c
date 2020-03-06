#include "benchmark.h"
#include "timezone.h"
#include "timezone_database.h"

#include <stdio.h>
#include <time.h>

#define STEP_WIDTH 100

int main(int argc, char **argv)
{
	const char *timezone = "America/Los_Angeles";
	struct timespec bm_start;
	struct timespec bm_end;
	int rc;

	rc = clock_gettime(CLOCK_MONOTONIC, &bm_start);
	if (rc) {
		fprintf(stderr, "failed to get time");
	}

	for (time_t t = timezone_offset_min_time; t < timezone_offset_max_time; t += STEP_WIDTH) {
		const time_t localtime = timezone_local_time(timezone, t);
		escape(&gmtime);
	}

	rc = clock_gettime(CLOCK_MONOTONIC, &bm_end);
	if (rc) {
		fprintf(stderr, "failed to get time");
	}

	fprintf(stdout, "avg timezone_gmt_time duration in ns: %lu",
	        timespec_diff_ns(&bm_start, &bm_end) / ((timezone_offset_max_time - timezone_offset_min_time) / STEP_WIDTH));

	return 0;
}
