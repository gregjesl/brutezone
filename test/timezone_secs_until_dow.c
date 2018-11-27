#include "timezone.h"
#include <cassert>
#include <math.h>

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	time_t latime;
	int result;
	struct tm tm;
	unsigned char hour, minute, second, dow;

	// Get the time in LA
	latime = timezone_current_local_time(timezonename);

	// Convert to a tm struct
	secs_to_tm(latime, &tm);

	// Record the hour and minute
	hour = tm.tm_hour;
	minute = tm.tm_min;
	second = tm.tm_sec;
	dow = tm.tm_wday;

	// Get the result
	result = timezone_secs_until_dow(timezonename, hour, minute, second, dow + 3 % 7);

	// Result should be within 5 seconds
	assert(abs(result - 86400 * 3) < 5);

	return 0;
}