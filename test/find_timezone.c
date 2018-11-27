#include "timezone.h"
#include <string.h>
#include <cassert>

int main(void)
{
	const char timezonename[] = "America/Los_Angeles";
	timezone *tz;

	// Find the timezone
	tz = find_timezone(timezonename);

	// Ensure the timezone was found
	assert(tz != NULL);

	// Compare the timezone names
	assert(strcmp(tz->name, timezonename) == 0);

	// Attempt to find a non-valid timezone
	assert(find_timezone("Not/Valid") == NULL);
}