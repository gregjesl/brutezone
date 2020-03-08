#include "timezone.h"
#include "test.h"
#include "timezone_database.h"

static void test_gmtime(const char *timezonename)
{
	time_t basetime, gmtime, localtime, revert, day_offset;

	// Get the gmt time
	time(&basetime);

	// Repeat the test for an entire year
	for(day_offset = 0; day_offset < 365 * 86400; day_offset += 86400)
	{

		// Move the time a bit
		gmtime = basetime + day_offset;

		// Get the time in LA
		localtime = timezone_local_time(timezonename, gmtime);

		// Verify the time was converted
		TEST_TRUE(localtime > 0);

		// Revert back to gmt
		revert = timezone_gmt_time(timezonename, localtime);

		// Verify the result
		// TEST_EQUAL(gmtime, revert);
		if(gmtime != revert) {
			printf("Timezone %s failed\n", timezonename);
		}
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
