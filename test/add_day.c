#include "test.h"
#include "timezone.h"

int main(void)
{
    time_t gmtime;
    struct tm tm;

    // Get the time of 29 January 1970 at 3 AM
    gmtime = (28 * 86400) + (10 * 3600) + (3 * 60) + 17;

    // Convert to a tm
    secs_to_tm(gmtime, &tm);

    // Add a day
    add_day(&tm);

    // Verify the result
    TEST_EQUAL(tm.tm_year, 70);
    TEST_EQUAL(tm.tm_mon, 0);
    TEST_EQUAL(tm.tm_mday, 30);
    TEST_EQUAL(tm.tm_hour, 10);
    TEST_EQUAL(tm.tm_min, 3);
    TEST_EQUAL(tm.tm_sec, 17);
    TEST_EQUAL(tm.tm_wday, 5);

    return 0;
}