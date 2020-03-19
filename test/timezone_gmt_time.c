#include "test.h"
#include "timezone.h"
#include "timezone_database.h"

static void test_gmtime(const char *timezonename)
{
    time_t basetime, gmtime, localtime;

    // Get the gmt time
    basetime = 1577836800; // 2020-01-01 00:00:00
	TEST_TRUE(basetime > BRUTEZONE_MIN_TIME);
	TEST_TRUE(basetime < BRUTEZONE_MAX_TIME);

    // Repeat the test for an entire year in half hour steps
    for (time_t offset = 0; offset < 365 * 86400; offset += 30 * 60) {
        gmtime = basetime + offset;

        // Get the local time
        localtime = timezone_local_time(timezonename, gmtime);

        // Verify the time was converted
        TEST_TRUE(localtime > 0);

        const int isdst = timezone_localtime_isdst(timezonename, localtime);

        TEST_NOT_EQUAL(isdst, TIMEZONE_NOT_FOUND);
        TEST_NOT_EQUAL(isdst, TIMEZONE_OUT_OF_RANGE);
        TEST_NOT_EQUAL(isdst, TIMEZONE_INVALID_TIME);

        // Revert back to gmt
        if (isdst == TIMEZONE_AMBIGUOUS_TIME) {
            // could be either of these
            const time_t revert_a = timezone_gmt_time_explicit(
                timezonename, localtime, TIMEZONE_FIRST);
            const time_t revert_b = timezone_gmt_time_explicit(
                timezonename, localtime, TIMEZONE_LATTER);
            TEST_TRUE(gmtime == revert_a || gmtime == revert_b);
            TEST_TRUE(revert_b > revert_a);
            TEST_EQUAL(timezone_gmt_time_explicit(timezonename, localtime,
                                                  TIMEZONE_STRICT),
                       TIMEZONE_AMBIGUOUS_TIME);
        } else {
            const time_t revert = timezone_gmt_time(timezonename, localtime);
            TEST_EQUAL(gmtime, revert);
        }
    }
}

int main(void)
{
    int i;

    // Loop through all timezones
    for (i = 0; i < TIMEZONE_DATABASE_COUNT; i++) {
        test_gmtime(timezone_array[i].name);
    }

    return 0;
}
