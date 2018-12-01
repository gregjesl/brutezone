#include "timezone.h"
#include <assert.h>
#include <stdlib.h>

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	time_t latime;
	int result;
	struct tm tm;
	unsigned char hour, minute, second;

	// Get the time in LA
	latime = timezone_current_local_time(timezonename);

	// Convert to a tm struct
	secs_to_tm(latime, &tm);

	// Record the hour and minute
	hour = tm.tm_hour;
	minute = tm.tm_min;
	second = tm.tm_sec;

	// Remove a day
	subtract_day(&tm);

	// Get the result
	result = timezone_secs_since(timezonename, hour, minute, second);

	// Result should be within 5 seconds
	assert(abs(result - 86400) < 5);

	return 0;
}