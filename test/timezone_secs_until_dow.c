#include "timezone.h"
#include <assert.h>
#include <stdlib.h>

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	time_t latime;
	int result;
	struct tm tm;
	unsigned char hour, minute, second, dow;
	int i;

	// Get the time in LA
	latime = timezone_current_local_time(timezonename) + 60;

	// Convert to a tm struct
	secs_to_tm(latime, &tm);

	// Record the hour and minute
	hour = tm.tm_hour;
	minute = tm.tm_min;
	second = tm.tm_sec;

	for (i = 1; i < 7; i++)
	{
		// Get the test day of week
		dow = tm.tm_wday + i % 7;
		
		// Get the result
		result = timezone_secs_until_dow(timezonename, hour, minute, second, dow);

		// Result should be within 5 seconds
		assert(abs(result - (86400 * i) - 60) < 5);
	}

	return 0;
}