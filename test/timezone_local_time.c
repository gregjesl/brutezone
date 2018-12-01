#include "timezone.h"
#include <assert.h>

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	time_t gmtime, latime, diff;
	
	// Get the gmt time
	time(&gmtime);

	// Move the time a bit
	gmtime += 86400 * 30;

	// Get the time in LA
	latime = timezone_local_time(timezonename, gmtime);

	// Record the difference
	assert(latime < gmtime);
	diff = gmtime - latime;

	// LA is between 7 or 8 hours difference depending on DST
	assert(diff == 7 * 60 * 60 || diff == 8 * 60 * 60);

	return 0;
}