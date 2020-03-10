#include "timezone.h"
#include <string.h>
#include "test.h"
#include "timezone_database.h"

int main(void)
{
	for (size_t i = 0; i < sizeof(timezone_array) / sizeof(*timezone_array); ++i) {
		// Find the timezone
		const tzdb_timezone *tz = find_timezone(timezone_array[i].name);

		if (!tz) {
			printf("%s not found\n", timezone_array[i].name);
		}
		// Ensure the timezone was found
		TEST_NOT_NULL(tz);

		// Compare the timezone names
		if (strcmp(tz->name, timezone_array[i].name) != 0) {
			printf("%s expected, %s found\n", timezone_array[i].name, tz->name);
		}
		TEST_STRING_EQUAL(tz->name, timezone_array[i].name);
	}

	// Attempt to find a non-valid timezone
	TEST_NULL(find_timezone("Not/Valid"));
}
