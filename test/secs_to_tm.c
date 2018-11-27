#include "timezone.h"
#include <cassert>

int main(void)
{
	time_t gmtime;
	struct tm tm;

	// Get the time of 29 January 1970 at 3 AM
	gmtime = (28 * 86400) + (10 * 3600) + (3 * 60) + 17;

	// Convert to a tm
	secs_to_tm(gmtime, &tm);

	// Verify the result
	assert(tm.tm_year == 70);
	assert(tm.tm_mon == 0);
	assert(tm.tm_mday == 29);
	assert(tm.tm_hour == 10);
	assert(tm.tm_min == 3);
	assert(tm.tm_sec == 17);
	assert(tm.tm_wday == 4);

	return 0;
}