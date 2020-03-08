#include "timezone.h"
#include "test.h"

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	time_t basetime, gmtime, latime, revert, day_offset;

	// Get the gmt time
	time(&basetime);

	// Repeat the test for an entire year
	for(day_offset = 0; day_offset < 365 * 86400; day_offset += 86400)
	{

		// Move the time a bit
		gmtime = basetime + day_offset;

		// Get the time in LA
		latime = timezone_local_time(timezonename, gmtime);

		// Revert back to gmt
		revert = timezone_gmt_time(timezonename, latime);

		// Verify the result
		TEST_EQUAL(gmtime, revert);
	}

	return 0;
}