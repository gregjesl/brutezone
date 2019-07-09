#include "timezone.h"
#include <string.h>
#include "test.h"

int main(void)
{
    time_t gmtime;
	struct tm tm;
    char timestamp[100];

	// Get the time of 29 January 1970 at 10:03:17 AM
	gmtime = (28 * 86400) + (10 * 3600) + (3 * 60) + 17;

    // Get the timestamp
    secs_to_tm(gmtime, &tm);
    iso_time(timestamp, tm);
    TEST_STRING_EQUAL(timestamp, "1970-01-29T10:03:17");

    // Return back
    TEST_EQUAL(from_iso_time(timestamp), gmtime);
}