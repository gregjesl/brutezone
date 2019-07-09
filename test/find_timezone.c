#include "timezone.h"
#include <string.h>
#include "test.h"

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	const tzdb_timezone *tz;

	// Find the timezone
	tz = find_timezone(timezonename);

	// Ensure the timezone was found
	TEST_NOT_NULL(tz);

	// Compare the timezone names
	TEST_STRING_EQUAL(tz->name, timezonename);

	// Attempt to find a non-valid timezone
	TEST_NULL(find_timezone("Not/Valid"));
}