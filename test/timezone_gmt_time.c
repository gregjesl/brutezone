#include "timezone.h"
#include "test.h"
#include "timezone_database.h"

static void test_gmtime(const char *timezonename)
{
	time_t basetime, gmtime, localtime, revert;

	// Get the gmt time
	basetime = 1577836800; // 2020-01-01 00:00:00

	// Repeat the test for an entire year in half hour steps
	for(time_t offset = 0; offset < 365 * 86400; offset += 30 * 60)
	{
		gmtime = basetime + offset;

		// Get the local time
		localtime = timezone_local_time(timezonename, gmtime);

		const int isdst = timezone_localtime_isdst(timezonename, localtime);

		TEST_NOT_EQUAL(isdst, TIMEZONE_NOT_FOUND);
		TEST_NOT_EQUAL(isdst, TIMEZONE_OUT_OF_RANGE);
		TEST_NOT_EQUAL(isdst, TIMEZONE_INVALID_TIME);

		if (isdst == TIMEZONE_AMBIGUATIVE_TIME) {
			// No sense in testing ambiguative timestamps right now
			continue;
		}

		// Verify the time was converted
		TEST_TRUE(localtime > 0);

		// Revert back to gmt
		revert = timezone_gmt_time(timezonename, localtime);

		TEST_EQUAL(gmtime, revert);
	}
}

int main(void)
{
	int i;

	// Loop through all timezones
	for(i = 0; i < TIMEZONE_DATABASE_COUNT; i++) 
	{
		test_gmtime(timezone_array[i].name);
	}

    return 0;
}
