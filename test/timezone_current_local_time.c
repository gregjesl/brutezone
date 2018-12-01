#include "timezone.h"
#include <assert.h>
#include <stdlib.h>

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	time_t gmtime, latime, diff;

	// Get the gmt time
	time(&gmtime);

	// Get the time in LA
	latime = timezone_current_local_time(timezonename);

	// Record the difference
	assert(latime < gmtime);
	diff = gmtime - latime;

	// LA is between 7 or 8 hours difference depending on DST
	// 5 seconds tolerance
	assert(abs((int)diff - 7 * 60 * 60) < 5 || abs((int)diff - 8 * 60 * 60) < 5);

	return 0;
}