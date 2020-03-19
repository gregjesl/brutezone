#include "test.h"
#include "timezone.h"

int main(void)
{
    const char timezonename[] = "America/Los_Angeles";
    time_t basetime, gmtime, latime, diff, day_offset;

    // Start on 1 January 2020 at 11:00:00 AM
    {
        struct tm tm;
        tm.tm_year = 120;
        tm.tm_mon = 0;
        tm.tm_mday = 0;
        tm.tm_hour = 11;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        basetime = tm_to_secs(&tm);
    }

    // Repeat the test for an entire year
    for (day_offset = 0; day_offset < 366 * 86400; day_offset += 86400) {

        // Set the time
        gmtime = basetime + day_offset;

        // Get the time in LA
        latime = timezone_local_time(timezonename, gmtime);

        // Record the difference
        diff = gmtime - latime;

        // DST occurs between the 68th day and the 305th day of the year in 2020
        if (day_offset / 86400 < 68 || day_offset / 86400 > 305) {
            TEST_EQUAL(diff, 8 * 60 * 60);
        } else {
            TEST_EQUAL(diff, 7 * 60 * 60);
        }
    }

    return 0;
}