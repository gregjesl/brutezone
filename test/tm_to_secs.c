#include "timezone.h"
#include <assert.h>

int main(void)
{
	time_t gmtime;
	struct tm tm;

	// Fill the structure
	tm.tm_year = 70;
	tm.tm_mon = 0;
	tm.tm_mday = 29;
	tm.tm_hour = 10;
	tm.tm_min = 3;
	tm.tm_sec = 17;
	tm.tm_wday = 4;

	// Verify the result
	assert(tm_to_secs(&tm) == (28 * 86400) + (10 * 3600) + (3 * 60) + 17);

	return 0;
}