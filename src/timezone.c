#include "timezone.h"
#include "timezone_impl.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

time_t timezone_local_time(const char *timezone_name, const time_t gmt)
{
    unsigned char index;
    const tzdb_timezone *tz;

    // Validate that the GMT time is within the table window
    if(gmt < timezone_offset_min_time || gmt >= timezone_offset_max_time) return 0;

    // Find the timezone
    tz = find_timezone(timezone_name);

    // If the timezone was not found, return 0
    if(tz == NULL) return 0;

    // Check for the first entry
    if(tz->entries[0].end > gmt) return gmt + (60 * tz->entries[0].offset);

    // Find the offset
    index = 0;
    while(tz->entries[index].end < timezone_offset_max_time)
    {
        index++;
        if(tz->entries[index].end > gmt) return gmt + (60 * tz->entries[index].offset);
    }

    // Out of bounds
    return 0;
}

time_t timezone_gmt_time(const char *timezone_name, const time_t local_time)
{
    time_t result;
    unsigned int index;
    const tzdb_timezone *tz;

    // Find the timezone
    tz = find_timezone(timezone_name);

    // If the timezone was not found, return 0
    if(tz == NULL) return 0;
    
    // Check for the first entry
    result = local_time - (60 * tz->entries[0].offset);
    if(result < tz->entries[0].end)
    {
        // Validate the result
        if(result < timezone_offset_max_time || result >= timezone_offset_min_time) return result;
        else return 0;
    }

    // Find the offset
    index = 0;
    while(tz->entries[index].end < timezone_offset_max_time)
    {
        index++;
        result = local_time - (60 * tz->entries[index].offset);
        if(result >= tz->entries[index].start)
        {
            // Validate the result
            if(result < timezone_offset_max_time || result >= timezone_offset_min_time) return result;
            else return 0;
        }
    }

    // Out of bounds
    return 0;
}

time_t timezone_current_local_time(const char *timezone)
{
    time_t timer;

    // Get the current time
    time(&timer);

    // Return the result
    return timezone_local_time(timezone, timer);
}

/*!
 * \note From http://git.musl-libc.org/cgit/musl/tree/src/time/__secs_to_tm.c?h=v0.9.15
 *       MIT License
 */
int secs_to_tm(long long t, struct tm *tm)
{
	long long days, secs;
	int remdays, remsecs, remyears;
	int qc_cycles, c_cycles, q_cycles;
	int years, months;
	int wday, yday, leap;
	static const char days_in_month[] = { 31,30,31,30,31,31,30,31,30,31,31,29 };

	/* Reject time_t values whose year would overflow int */
	if (t < INT_MIN * 31622400LL || t > INT_MAX * 31622400LL)
		return -1;

	secs = t - LEAPOCH;
	days = secs / 86400;
	remsecs = secs % 86400;
	if (remsecs < 0) {
		remsecs += 86400;
		days--;
	}

	wday = (3 + days) % 7;
	if (wday < 0) wday += 7;

	qc_cycles = days / DAYS_PER_400Y;
	remdays = days % DAYS_PER_400Y;
	if (remdays < 0) {
		remdays += DAYS_PER_400Y;
		qc_cycles--;
	}

	c_cycles = remdays / DAYS_PER_100Y;
	if (c_cycles == 4) c_cycles--;
	remdays -= c_cycles * DAYS_PER_100Y;

	q_cycles = remdays / DAYS_PER_4Y;
	if (q_cycles == 25) q_cycles--;
	remdays -= q_cycles * DAYS_PER_4Y;

	remyears = remdays / 365;
	if (remyears == 4) remyears--;
	remdays -= remyears * 365;

	leap = !remyears && (q_cycles || !c_cycles);
	yday = remdays + 31 + 28 + leap;
	if (yday >= 365 + leap) yday -= 365 + leap;

	years = remyears + 4 * q_cycles + 100 * c_cycles + 400 * qc_cycles;

	for (months = 0; days_in_month[months] <= remdays; months++)
		remdays -= days_in_month[months];

	if (years + 100 > INT_MAX || years + 100 < INT_MIN)
		return -1;

	tm->tm_year = years + 100;
	tm->tm_mon = months + 2;
	if (tm->tm_mon >= 12) {
		tm->tm_mon -= 12;
		tm->tm_year++;
	}
	tm->tm_mday = remdays + 1;
	tm->tm_wday = wday;
	tm->tm_yday = yday;

	tm->tm_hour = remsecs / 3600;
	tm->tm_min = remsecs / 60 % 60;
	tm->tm_sec = remsecs % 60;

	return 0;
}

/*!
 * \note From http://git.musl-libc.org/cgit/musl/tree/src/time/__tm_to_secs.c?h=v0.9.15
 *       MIT License
 */
long long tm_to_secs(const struct tm *tm)
{
	int is_leap;
	long long year = tm->tm_year;
	int month = tm->tm_mon;
	if (month >= 12 || month < 0) {
		int adj = month / 12;
		month %= 12;
		if (month < 0) {
			adj--;
			month += 12;
		}
		year += adj;
	}
	long long t = year_to_secs(year, &is_leap);
	t += month_to_secs(month, is_leap);
	t += 86400LL * (tm->tm_mday - 1);
	t += 3600LL * tm->tm_hour;
	t += 60LL * tm->tm_min;
	t += tm->tm_sec;
	return t;
}

void add_day(struct tm *tm)
{
    long long timer;

    // Convert the struct
    timer = tm_to_secs(tm);

    // Add the time
    timer += 86400;

    // Convert back
    secs_to_tm(timer, tm);
}

void subtract_day(struct tm *tm)
{
    long long timer;

    // Convert the struct
    timer = tm_to_secs(tm);

    // Add the time
    timer -= 86400;

    // Convert back
    secs_to_tm(timer, tm);
}

time_t timezone_secs_until(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second)
{
    time_t timer;
    struct tm tm;

    // Get the current local time
    timer = timezone_current_local_time(timezone);

    // Convert to time struct
    secs_to_tm(timer, &tm);

    // Set the hours and minutes
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;

    // If the updated time is in the future, return the time until the event
    if(tm_to_secs(&tm) > timer) return tm_to_secs(&tm) - timer;
    
    // Add a day to make the time in the future
    add_day(&tm);

    // Return the difference
    return tm_to_secs(&tm) - timer;
}

time_t timezone_secs_since(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second)
{
    time_t timer;
    struct tm tm;

    // Get the current local time
    timer = timezone_current_local_time(timezone);

    // Convert to time struct
    secs_to_tm(timer, &tm);

    // Set the hours and minutes
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;

    // If the updated time is in the past, return the time since the event
    if(tm_to_secs(&tm) < timer) return timer - tm_to_secs(&tm);
    
    // Remove a day to make the time in the past
    subtract_day(&tm);

    // Return the difference
    return timer - tm_to_secs(&tm);
}

time_t timezone_secs_until_dow(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second, const unsigned char wday)
{
    time_t timer, timer2;
    struct tm tm;

    // Get the current time in the timezone
    timer = timezone_current_local_time(timezone);

    // Get the next occurance in the timezone
    timer2 = timer + timezone_secs_until(timezone, hour, minute, second);

    // Convert to time struct
    secs_to_tm(timer2, &tm);

    // Check for same day
    if(tm.tm_wday == wday) return timer2 - timer;

    // Return the day
    if(wday > tm.tm_wday) return timer2 + (86400 * (wday - tm.tm_wday)) - timer;
    return timer2 + (86400 * ((wday + 7) - tm.tm_wday)) - timer;
}

time_t timezone_secs_since_dow(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second, const unsigned char wday)
{
    time_t timer, timer2;
    struct tm tm;

    // Get the current time in the timezone
    timer = timezone_current_local_time(timezone);

    // Get the last occurance in the timezone
	timer2 = timer - timezone_secs_since(timezone, hour, minute, second);

    // Convert to time struct
    secs_to_tm(timer2, &tm);

    // Check for same day
    if(tm.tm_wday == wday) return timer - timer2;

    // Return the day
	// Check if the desired day of the week was earlier this week
	if (tm.tm_wday > wday)
	{
		// It was
		return timer + (86400 * (tm.tm_wday - wday)) - timer2;
	}
    return timer + (86400 * ((tm.tm_wday + 7) - wday)) - timer2;
}

void iso_time(char* str, const struct tm datetime)
{
    sprintf(str, 
        "%i-%02i-%02iT%02i:%02i:%02i", 
        datetime.tm_year + 1900, 
        datetime.tm_mon + 1, 
        datetime.tm_mday, 
        datetime.tm_hour, 
        datetime.tm_min, 
        datetime.tm_sec
    );
}

time_t from_iso_time(const char* str)
{
    struct tm result;
    char *reader;
    char copy[20];

    memcpy(copy, str, 20);
    reader = copy;

    int year;
    sscanf(reader, "%i", &year);
    result.tm_year = (year - 1900);
    reader += 5;

    sscanf(reader, "%i", &result.tm_mon);
    result.tm_mon -= 1;
    reader += 3;

    sscanf(reader, "%i", &result.tm_mday);
    reader += 3;

    sscanf(reader, "%i", &result.tm_hour);
    reader += 3;

    sscanf(reader, "%i", &result.tm_min);
    reader += 3;

    sscanf(reader, "%i", &result.tm_sec);

    return tm_to_secs(&result);
}
