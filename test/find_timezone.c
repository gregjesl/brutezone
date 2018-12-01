#include "timezone.h"
#include <string.h>
#include <assert.h>

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	const tzdb_timezone *tz;

	// Find the timezone
	tz = find_timezone(timezonename);

	// Ensure the timezone was found
	assert(tz != NULL);

	// Compare the timezone names
	assert(strcmp(tz->name, timezonename) == 0);

	// Attempt to find a non-valid timezone
	assert(find_timezone("Not/Valid") == NULL);
}