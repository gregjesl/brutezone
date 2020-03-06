#include "timezone.h"
#include "test.h"
#include "timezone_database.h"

static void test_gmtime(time_t gmtime, const char *timezonename)
{
	time_t latime, revert;

	// Get the time in LA
	latime = timezone_local_time(timezonename, gmtime);

	// Revert back to gmt
	revert = timezone_gmt_time(timezonename, latime);

	//printf("gmt: %ld", gmtime);
	// Verify the result
	if (gmtime != revert) {
		printf("gmtime: %ld, latime: %ld, revert: %ld\n", gmtime, latime, revert);
	}
	TEST_EQUAL(gmtime, revert);

}

int main(void)
{
    const char timezonename[] = "America/Los_Angeles";

    test_gmtime(25696800, timezonename);

    return 0;
}
