#ifndef WAKE_TIMEZONE_H
#define WAKE_TIMEZONE_H

#include "timezone_database.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

enum timezone_err {
    TIMEZONE_NOT_FOUND = -1,
    TIMEZONE_OUT_OF_RANGE = -2,
    TIMEZONE_AMBIGUOUS_TIME = -3,
    TIMEZONE_INVALID_TIME = -4,
};

enum timezone_gmt_time_behaviour {
    TIMEZONE_STRICT = 0,
    TIMEZONE_ANY = 1,
    TIMEZONE_FIRST = 2,
    TIMEZONE_LATTER = 3,
};

/*!
 * \brief lookup a timezone by IANA-Name
 *
 * \param[in] timezone_name the IANA-Name of the timezone to look for
 *                          (e.g. Europe/Berlin)
 *
 * \return pointer to the tzdb_timezone struct holding the offset-information,
 *         or NULL if the timezone was not found in the database
 */
const tzdb_timezone *find_timezone(const char *timezone_name);

/*!
 * \brief determines if the given local time is DST
 *
 * \param[in] timezone_name
 * \param[in] localtime
 *
 * \return 1 if the given timestamp is DST, 0 if it isn't,
 *         TIMEZONE_AMBIGUOUS_TIME if it is not defined due to daylight-time ->
 * standard-time transition TIMEZONE_INVALID_TIME if the timestamp is not a
 * valid time due to standard-time -> daylight-time transition
 *         TIMEZONE_NOT_FOUND if the timezone was not found in the database
 *         TIMEZONE_OUT_OF_RANGE if the given timestamp is outside of the
 * database range.
 *
 * \note this function considers any time-period that has a bigger offset
 *       than the one immediately before ("clock was turned forward") to be DST,
 *       even if the "historic" reason for this was something different.
 */
int timezone_localtime_isdst(const char *timezone_name, time_t localtime);

/*!
 * \brief converts a unix-timestamp in GMT to the corresponding local
 *        time in the give IANA-timezone
 *
 * \param[in] timezone_name IANA timezone name
 * \param[in] gmt unix timestamp (seconds since 1970-01-01 00:00:00)
 *                representing a gmt time
 *
 * \return unix timestamp representing the corresponding local time to the given
 * gmt time, or TIMEZONE_NOT_FOUND if the timezone_name was not found in the
 * database, or TIMEZONE_OUT_OF_RANGE if the given timespamp was outside of the
 * database range
 */
time_t timezone_local_time(const char *timezone, time_t gmt);

time_t timezone_current_local_time(const char *timezone);

/*!
 * \brief converts a unix-timestamp representing a local time in the
 *        given IANA-timezone to gmt time
 *
 * For some input times, there might not be an unambiguative
 * interpretation. Namely, right after the clocks are turned back
 * (for DST), there is a window of usually one hour where the same
 * local time represents two gmt-times. This function might return
 * either one of them.
 *
 * Similarly, when clocks are turned forward there is a period of time
 * (by how much the clock was turned forward), which is never a valid
 * localtime.
 *
 * \param[in] timezone_name IANA timezone name
 * \param[in] local_time unix timestamp (seconds since 1970-01-01 00:00:00)
 *            representing a time in the given timezone
 * \param[in] behaviour how to deal with ambiguative or invalid local times,
 *            see return
 *
 * \return - if there is no ambiguity, returns the unix timestamp representing
 *           the corresponding gmt time to the given localtime,
 *         - if local_time is ambiguitve:
 *             - returns the earlier one if behaviour is TIMEZONE_FIRST,
 *             - returns the later one if behaviour is TIMEZONE_LATTER,
 *             - returns either one if behaviour is TIMEZONE_ANY (fastest),
 *             - returns TIMEZONE_AMBIGUOUS_TIME if behaviour is
 *               TIMEZONE_STRICT
 *         - if local_time is an invalid time:
 *             - returns gmt time as if the earlier offset was still valid if
 *               behaviour is TIMEZONE_FIRST or TIMEZONE_ANY
 *             - returns gmt time as if the later offset was valid if
 *               behaviour is TIMEZONE_LATTER
 *             - returns TIMEZONE_INVALID_TIME if behaviour is TIMEZONE_STRICT
 *         - TIMEZONE_NOT_FOUND if the timezone was not found in the database
 *         - TIMEZONE_OUT_OF_RANGE if the given timestamp is outside of the
 * database range.
 */
time_t timezone_gmt_time_explicit(const char *timezone, time_t local_time,
                                  enum timezone_gmt_time_behaviour behaviour);

/*!
 * \brief same as timezone_gmt_time(timezone, localtime, TIMEZONE_ANY)
 */
time_t timezone_gmt_time(const char *timezone, time_t local_time);

int secs_to_tm(long long t, struct tm *tm);
long long tm_to_secs(const struct tm *tm);
void add_day(struct tm *tm);
void subtract_day(struct tm *tm);
time_t timezone_secs_until(const char *timezone, const unsigned char hour,
                           const unsigned char minute,
                           const unsigned char second);
time_t timezone_secs_since(const char *timezone, const unsigned char hour,
                           const unsigned char minute,
                           const unsigned char second);
time_t timezone_secs_until_dow(const char *timezone, const unsigned char hour,
                               const unsigned char minute,
                               const unsigned char second,
                               const unsigned char wday);
time_t timezone_secs_since_dow(const char *timezone, const unsigned char hour,
                               const unsigned char minute,
                               const unsigned char second,
                               const unsigned char wday);
void iso_time(char *str, const struct tm datetime);
time_t from_iso_time(const char *str);
#ifdef __cplusplus
}
#endif

#endif
