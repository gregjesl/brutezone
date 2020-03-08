#ifndef WAKE_TIMEZONE_H
#define WAKE_TIMEZONE_H

#include <time.h>
#include "timezone_database.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief converts a unix-timestamp in gmt to the corresponding local
 *        time in the give IANA-timezone
 *
 * \param[in] timezone_name IANA timezone name
 * \param[in] gmt unix timestamp (seconds since 1970-01-01 00:00:00)
 *                representing a gmt time
 *
 * \return unix timestamp representing the corresponding local time to the given
 *         gmt time, or 0 if the timezone_name was not found in the database or
 *         if the given timespamp was outside of the database range
 */
time_t timezone_local_time(const char *timezone, const time_t gmt);

/*!
 * \brief converts a unix-timestamp representing a local time in the
 *        given IANA-timezone to gmt time
 *
 * \param[in] timezone_name IANA timezone name
 * \param[in] local_time unix timestamp (seconds since 1970-01-01 00:00:00)
 *            representing a time in the given timezone
 *
 * \return unix timestamp representing the corresponding gmt time to the given
 *         local time, or 0 if the timezone_name was not found in the database
 *         or if the given timespamp was outside of the database range
 *
 * \note For some input times, there might not be an unambiguative
 *       interpretation. Namely, right after the clocks are turned back
 *       (for DST), there is a window of usually one hour where the same
 *       local time represents two gmt-times. This function might return
 *       either one of them.
 */
time_t timezone_current_local_time(const char *timezone);

/*!
 * \brief returns the current local time in the timezone named timezone
 *
 * \param[in] timezone_name IANA timezone name
 *
 * \return unix timestamp representing the current time in the given timezone,
 *         or 0 if the timezone_name was not found in the database
 *         or if the current time is outside of the database range
 */
time_t timezone_gmt_time(const char *timezone, const time_t local_time);

/*!
 * \brief converts a tick counter (such as time_t) to a time structure
 *
 * \param[in] t seconds since 1970-01-01 00:00:00
 * \param[out] tm struct to populate
 *
 * \return 0 on success, != 0 on failure
 */
int secs_to_tm(long long t, struct tm *tm);

/*!
 * \brief converts a time structure to a tick counter
 *
 * \param[in] tm time to convert
 *
 * \return seconds since 1970-01-01 00:00:00
 */
long long tm_to_secs(const struct tm *tm);

/*!
 * \brief adds a day to the given time struct
 *
 * \param[in,out] tm time struct
 */
void add_day(struct tm *tm);

/*!
 * \brief substracts a day from the given time struct
 *
 * \param[in,out] tm time struct
 */
void subtract_day(struct tm *tm);

/*!
 * \brief returns the number of seconds until the next occurance of
 *        hour:minute:second in the timezone named timezone
 */
time_t timezone_secs_until(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second);

/*!
 * \brief returns the number of seconds since the last occurance of
 *        hour:minute:second in the timezone named timezone
 */
time_t timezone_secs_since(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second);

/*!
 * \brief returns the number of seconds until the next occurance of
 *        hour:minute:second on wday (day of the week) in the timezone
 *        named timezone
 */
time_t timezone_secs_until_dow(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second, const unsigned char wday);

/*!
 * \brief returns the number of seconds since the last occurance of
 *        hour:minute:second on wday (day of the week) in the timezone
 *        named timezone
 */
time_t timezone_secs_since_dow(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second, const unsigned char wday);

/*!
 * \brief populates the str input with the ISO 8601 timestamp of the
 *        datetime argument.
 *
 * \param[out] str
 * \param[in] datetime
 *
 * \example output: "1970-01-29T10:03:17" for 29 January 1970 at 10:03:17 AM.
 */
void iso_time(char* str, const struct tm datetime);

/*!
 * \brief returns the unix timestamp for the given textual iso timestamp
 *
 * \param[in] str
 */
time_t from_iso_time(const char* str);

#ifdef __cplusplus
}
#endif

#endif
