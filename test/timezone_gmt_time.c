#include "timezone.h"
#include <assert.h>

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	time_t gmtime, latime, revert;

	// Get the gmt time
	time(&gmtime);

	// Move the time a bit
	gmtime += 86400 * 30;

	// Get the time in LA
	latime = timezone_local_time(timezonename, gmtime);

	// Revert back to gmt
	revert = timezone_gmt_time(timezonename, latime);

	// Verify the result
	assert(gmtime == revert);

	return 0;
}