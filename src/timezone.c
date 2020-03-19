// Use sscanf_s when appropriate
// Macro must be defined before stdio.h is included
#if defined(__STDC_LIB_EXT1__) && defined(__STDC_WANT_LIB_EXT1__)
#define READ(a, b, c) sscanf_s(a, b, c)
#else
#define READ(a, b, c) sscanf(a, b, c)
#endif

#include "timezone.h"
#include "timezone_database.h"
#include "timezone_impl.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define UNPACK(a) ((time_t)(a.offset)) * 10
#define UNPACK_PTR(a) ((time_t)(a->offset)) * 10

static int localtime_in_tz_range(const tzdb_timezone *tz, time_t local_time)
{
    return (local_time - UNPACK(tz->entries[0]) >= tz->entries[0].start &&
            local_time - UNPACK(tz->entries[tz->n_entries - 1]) <
                timezone_offset_max_time);
}

// only valid if localtime_in_tz_range(tz, localtime) == true or behaviour ==
// TIMEZONE_STRICT
static const timezone_offset *
find_gmt_offset(const tzdb_timezone *tz, time_t local_time,
                enum timezone_gmt_time_behaviour behaviour)
{
    const timezone_offset *begin = tz->entries;
    const timezone_offset *end = tz->entries + tz->n_entries;
    const timezone_offset *last = end;

    do {
        const timezone_offset *needle = begin + (end - begin) / 2;
        const time_t result = local_time - UNPACK_PTR(needle);
        if (result < needle->start) {
            end = needle;
        } else {
            const timezone_offset *next = needle + 1;
            if ((next == last && result < timezone_offset_max_time) ||
                result < next->start) {
                return needle;
            } else {
                begin = next;
            }
        }
    } while (begin < end);

    switch (behaviour) {
        case TIMEZONE_FIRST:
            return begin;
        case TIMEZONE_ANY:
        case TIMEZONE_LATTER:
            return end;
        case TIMEZONE_STRICT:
        default:
            return NULL;
    }
}

static const timezone_offset *find_localtime_offset(const tzdb_timezone *tz,
                                                    time_t gmt)
{
    const timezone_offset *begin = tz->entries;
    const timezone_offset *end = tz->entries + tz->n_entries;
    const timezone_offset *last = end;

    do {
        const timezone_offset *needle = begin + (end - begin) / 2;
        if (gmt < needle->start) {
            end = needle;
        } else {
            const timezone_offset *next = needle + 1;
            if ((next == last && gmt < timezone_offset_max_time) ||
                gmt < next->start) {
                return needle;
            } else {
                begin = next;
            }
        }
    } while (begin < end);

    return NULL;
}

const tzdb_timezone *find_timezone(const char *timezone_name)
{
    const tzdb_timezone *begin = timezone_array;
    const tzdb_timezone *end = timezone_array + TIMEZONE_DATABASE_COUNT;

    // Since the list of timezones above is always generated in sorted order,
    // we use a binary search to find the timezone
    do {
        const tzdb_timezone *needle = begin + (end - begin) / 2;
        const int cmp = strcmp(timezone_name, needle->name);
        if (cmp > 0) {
            begin = needle + 1;
        } else if (cmp < 0) {
            end = needle;
        } else {
            // Return the timezone if found
            return needle;
        }
    } while (begin < end);

    // If the timezone was not found, return null
    return NULL;
}

int timezone_localtime_isdst(const char *timezone_name, time_t local_time)
{
    const tzdb_timezone *tz = find_timezone(timezone_name);
    if (!tz)
        return TIMEZONE_NOT_FOUND;

    if (!localtime_in_tz_range(tz, local_time))
        return TIMEZONE_OUT_OF_RANGE;

    const timezone_offset *offset =
        find_gmt_offset(tz, local_time, TIMEZONE_STRICT);
    if (!offset)
        return TIMEZONE_INVALID_TIME;

    const timezone_offset *end = tz->entries + tz->n_entries;

    const timezone_offset *next = offset + 1;
    if (next < end && local_time - UNPACK_PTR(next) >= next->start)
        return TIMEZONE_AMBIGUOUS_TIME;

    const timezone_offset *prev = offset - 1;
    if (prev >= tz->entries && local_time - UNPACK_PTR(next) < offset->start)
        return TIMEZONE_AMBIGUOUS_TIME;

    if ((prev >= tz->entries && prev->offset < offset->offset) ||
        (next < end && next->offset < offset->offset)) {
        return 1;
    }

    return 0;
}

time_t timezone_local_time(const char *timezone_name, time_t gmt)
{
    const tzdb_timezone *tz = find_timezone(timezone_name);
    if (!tz)
        return TIMEZONE_NOT_FOUND;

    const timezone_offset *offset = find_localtime_offset(tz, gmt);
    if (!offset)
        return TIMEZONE_OUT_OF_RANGE;

    return gmt + UNPACK_PTR(offset);
}

time_t timezone_gmt_time_explicit(const char *timezone_name, time_t local_time,
                                  enum timezone_gmt_time_behaviour behaviour)
{
    const tzdb_timezone *tz = find_timezone(timezone_name);
    if (!tz)
        return TIMEZONE_NOT_FOUND;

    if (!localtime_in_tz_range(tz, local_time))
        return TIMEZONE_OUT_OF_RANGE;

    const timezone_offset *offset = find_gmt_offset(tz, local_time, behaviour);
    if (!offset)
        return TIMEZONE_OUT_OF_RANGE;

    switch (behaviour) {
        case TIMEZONE_ANY:
            return local_time - UNPACK_PTR(offset);

        case TIMEZONE_FIRST:
            if (offset > tz->entries &&
                local_time - UNPACK(offset[-1]) < offset->start)
                return local_time - UNPACK(offset[-1]);
            return local_time - UNPACK_PTR(offset);

        case TIMEZONE_LATTER:
            if (offset < tz->entries + tz->n_entries - 1 &&
                local_time - UNPACK(offset[1]) >= offset[1].start) {
                return local_time - UNPACK(offset[1]);
            }
            return local_time - UNPACK_PTR(offset);

        case TIMEZONE_STRICT:
        default:
            if ((offset > tz->entries &&
                 local_time - UNPACK(offset[-1]) < offset->start) ||
                (offset < tz->entries + tz->n_entries - 1 &&
                 local_time - UNPACK(offset[1]) >= offset[1].start)) {
                return TIMEZONE_AMBIGUOUS_TIME;
            }
            return local_time - UNPACK_PTR(offset);
    }
}

time_t timezone_gmt_time(const char *timezone_name, time_t local_time)
{
    return timezone_gmt_time_explicit(timezone_name, local_time, TIMEZONE_ANY);
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
 * From
 * http://git.musl-libc.org/cgit/musl/tree/src/time/__secs_to_tm.c?h=v0.9.15 MIT
 * License
 */
int secs_to_tm(long long t, struct tm *tm)
{
    long long days, secs;
    int remdays, remsecs, remyears;
    int qc_cycles, c_cycles, q_cycles;
    int years, months;
    int wday, yday, leap;
    static const char days_in_month[] = {31, 30, 31, 30, 31, 31,
                                         30, 31, 30, 31, 31, 29};

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
    if (wday < 0)
        wday += 7;

    qc_cycles = (int)(days / DAYS_PER_400Y);
    remdays = days % DAYS_PER_400Y;
    if (remdays < 0) {
        remdays += DAYS_PER_400Y;
        qc_cycles--;
    }

    c_cycles = remdays / DAYS_PER_100Y;
    if (c_cycles == 4)
        c_cycles--;
    remdays -= c_cycles * DAYS_PER_100Y;

    q_cycles = remdays / DAYS_PER_4Y;
    if (q_cycles == 25)
        q_cycles--;
    remdays -= q_cycles * DAYS_PER_4Y;

    remyears = remdays / 365;
    if (remyears == 4)
        remyears--;
    remdays -= remyears * 365;

    leap = !remyears && (q_cycles || !c_cycles);
    yday = remdays + 31 + 28 + leap;
    if (yday >= 365 + leap)
        yday -= 365 + leap;

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
 * From
 * http://git.musl-libc.org/cgit/musl/tree/src/time/__tm_to_secs.c?h=v0.9.15 MIT
 * License
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

time_t timezone_secs_until(const char *timezone, const unsigned char hour,
                           const unsigned char minute,
                           const unsigned char second)
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
    if (tm_to_secs(&tm) > timer)
        return tm_to_secs(&tm) - timer;

    // Add a day to make the time in the future
    add_day(&tm);

    // Return the difference
    return tm_to_secs(&tm) - timer;
}

time_t timezone_secs_since(const char *timezone, const unsigned char hour,
                           const unsigned char minute,
                           const unsigned char second)
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
    if (tm_to_secs(&tm) < timer)
        return timer - tm_to_secs(&tm);

    // Remove a day to make the time in the past
    subtract_day(&tm);

    // Return the difference
    return timer - tm_to_secs(&tm);
}

time_t timezone_secs_until_dow(const char *timezone, const unsigned char hour,
                               const unsigned char minute,
                               const unsigned char second,
                               const unsigned char wday)
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
    if (tm.tm_wday == wday)
        return timer2 - timer;

    // Return the day
    if (wday > tm.tm_wday)
        return timer2 + (86400 * (wday - tm.tm_wday)) - timer;
    return timer2 + (86400 * ((wday + 7) - tm.tm_wday)) - timer;
}

time_t timezone_secs_since_dow(const char *timezone, const unsigned char hour,
                               const unsigned char minute,
                               const unsigned char second,
                               const unsigned char wday)
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
    if (tm.tm_wday == wday)
        return timer - timer2;

    // Return the day
    // Check if the desired day of the week was earlier this week
    if (tm.tm_wday > wday) {
        // It was
        return timer + (86400 * (tm.tm_wday - wday)) - timer2;
    }
    return timer + (86400 * ((tm.tm_wday + 7) - wday)) - timer2;
}

void iso_time(char *str, const struct tm datetime)
{
    sprintf(str, "%i-%02i-%02iT%02i:%02i:%02i", datetime.tm_year + 1900,
            datetime.tm_mon + 1, datetime.tm_mday, datetime.tm_hour,
            datetime.tm_min, datetime.tm_sec);
}

time_t from_iso_time(const char *str)
{
    struct tm result;
    char *reader;
    char copy[20];

    memcpy(copy, str, 20);
    reader = copy;

    int year;
    READ(reader, "%i", &year);
    result.tm_year = (year - 1900);
    reader += 5;

    READ(reader, "%i", &result.tm_mon);
    result.tm_mon -= 1;
    reader += 3;

    READ(reader, "%i", &result.tm_mday);
    reader += 3;

    READ(reader, "%i", &result.tm_hour);
    reader += 3;

    READ(reader, "%i", &result.tm_min);
    reader += 3;

    READ(reader, "%i", &result.tm_sec);

    return tm_to_secs(&result);
}
