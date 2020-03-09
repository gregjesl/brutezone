#include "test.h"
#include <timezone.h>

#define SEC_PER_HOUR 3600

int main(int argc, char **argv)
{
	TEST_EQUAL(TIMEZONE_NOT_FOUND, timezone_localtime_isdst("Foo/Bar", 0));

	const char *timezone = "Europe/Berlin";

	// Berlin 1970-01-01 00:00:00 is UTC 1969-01-01 23:00:00, so outside of our (default) range
	const time_t local_1970_01_01_00_00_00 = 0;
	TEST_EQUAL(TIMEZONE_OUT_OF_RANGE, timezone_localtime_isdst(timezone, local_1970_01_01_00_00_00));

	// Berlin 2038-01-19 01:00:00 is UTC 2038-01-19 00:00:00, so outside of our (default) range
	const time_t local_2038_01_19_01_00_00 = 2147475600;
	TEST_EQUAL(TIMEZONE_OUT_OF_RANGE, timezone_localtime_isdst(timezone, local_2038_01_19_01_00_00));

	// Berlin 1970-01-01 01:00:00 was standard time
	const time_t local_1970_01_01_01_00_00 = local_1970_01_01_00_00_00 + SEC_PER_HOUR;
	TEST_EQUAL(0, timezone_localtime_isdst(timezone, local_1970_01_01_01_00_00));

	// Berlin 2038-01-19 00:00:00 will be (baring any legal changes) standard time
	const time_t local_2038_01_19_00_00_00 = 2147475600 - SEC_PER_HOUR;
	TEST_EQUAL(0, timezone_localtime_isdst(timezone, local_2038_01_19_00_00_00));

	// 2020-03-29 02:00:00 clocks are turned forward 1 hour to 2020-03-29 03:00:00 daylight time.

	// thus 2020-03-29 02:00:00 to 2020-03-29 02:59:59 never exist
	const time_t local_2020_03_29_02_00_00 = 1585447200;
	for (time_t i = 0; i < SEC_PER_HOUR; ++i) {
		TEST_EQUAL(TIMEZONE_INVALID_TIME, timezone_localtime_isdst(timezone, local_2020_03_29_02_00_00 + i));
	}

	// and 2020-03-29 01:59:59 is standard time
	const time_t local_2020_03_29_01_59_59 = local_2020_03_29_02_00_00 - 1;
	TEST_EQUAL(0, timezone_localtime_isdst(timezone, local_2020_03_29_01_59_59));

	// and 2020-03-29 03:00:00 is daylight savings time
	const time_t local_2020_03_29_03_00_00 = local_2020_03_29_02_00_00 + SEC_PER_HOUR;
	TEST_EQUAL(1, timezone_localtime_isdst(timezone, local_2020_03_29_03_00_00));

	// 2020-10-25 03:00:00 clocks are turned backward 1 hour to 2020-10-25 02:00:00 local standard time

	// thus 2020-10-25 02:00:00 to 2020-10-25 02:59:59 could be either daylight saving time or not
	const time_t local_2020_10_25_02_00_00 = 1603591200;
	for (time_t i = 0; i < SEC_PER_HOUR; ++i) {
		TEST_EQUAL(TIMEZONE_AMBIGUATIVE_TIME, timezone_localtime_isdst(timezone, local_2020_10_25_02_00_00 + i));
	}

	// and 2020-10-25 01:59:59 is daylight saving time
	const time_t local_2020_10_25_01_59_59 = local_2020_10_25_02_00_00 - 1;
	TEST_EQUAL(1, timezone_localtime_isdst(timezone, local_2020_10_25_01_59_59));

	// and 2020-10-25 03:00:00 is standard time
	const time_t local_2020_10_25_03_00_00 = local_2020_10_25_02_00_00 + SEC_PER_HOUR;
	TEST_EQUAL(0, timezone_localtime_isdst(timezone, local_2020_10_25_03_00_00));

	// Indian/Mahe never changed times, and thus has no daylight time
	timezone = "Indian/Mahe";
	TEST_EQUAL(0, timezone_localtime_isdst(timezone, 10000000));

	timezone = "Australia/Melbourne";
	// Australia/Melbourne 1970-01-01 10:00:00 was standard time
	const time_t local_1970_01_01_10_00_00 = local_1970_01_01_00_00_00 + 10 * SEC_PER_HOUR;
	TEST_EQUAL(0, timezone_localtime_isdst(timezone, local_1970_01_01_10_00_00));

	// Australia/Melbourne 1970-01-01 10:00:00 will be (baring any legal changes) daylight saving time
	const time_t local_2038_01_19_10_00_00 = local_2038_01_19_00_00_00 + 10 * SEC_PER_HOUR;
	TEST_EQUAL(1, timezone_localtime_isdst(timezone, local_2038_01_19_10_00_00));

	return 0;
}
