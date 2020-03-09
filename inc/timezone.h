#ifndef WAKE_TIMEZONE_H
#define WAKE_TIMEZONE_H

#include <time.h>
#include "timezone_database.h"

#ifdef __cplusplus
extern "C" {
#endif

enum timezone_err {
	TIMEZONE_NOT_FOUND = -1,
	TIMEZONE_OUT_OF_RANGE = -2,
	TIMEZONE_AMBIGUATIVE_TIME = -3,
	TIMEZONE_INVALID_TIME = -4,
};

/*!
 * \brief determines if the given local time is DST
 *
 * \param[in] timezone_name
 * \param[in] localtime
 *
 * \return 1 if the given timestamp is DST, 0 if it isn't,
 *         TIMEZONE_AMBIGUATIVE_TIME if it is not defined due to daylight-time -> standard-time transition
 *         TIMEZONE_INVALID_TIME if the timestamp is not a valid time due to standard-time -> daylight-time transition
 *         TIMEZONE_NOT_FOUND if the timezone was not found in the database
 *         TIMEZONE_OUT_OF_RANGE if the given timestamp is outside of the database range.
 *
 * \note this function considers any time-period that has a bigger offset
 *       than the one immediately before ("clock was turned forward") to be DST,
 *       even if the "historic" reason for this was something different.
 */
int timezone_localtime_isdst(const char *timezone_name, time_t localtime);

time_t timezone_local_time(const char *timezone, const time_t gmt);
time_t timezone_current_local_time(const char *timezone);
time_t timezone_gmt_time(const char *timezone, const time_t local_time);
int secs_to_tm(long long t, struct tm *tm);
long long tm_to_secs(const struct tm *tm);
void add_day(struct tm *tm);
void subtract_day(struct tm *tm);
time_t timezone_secs_until(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second);
time_t timezone_secs_since(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second);
time_t timezone_secs_until_dow(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second, const unsigned char wday);
time_t timezone_secs_since_dow(const char *timezone, const unsigned char hour, const unsigned char minute, const unsigned char second, const unsigned char wday);
void iso_time(char* str, const struct tm datetime);
time_t from_iso_time(const char* str);
#ifdef __cplusplus
}
#endif


#endif
