#ifndef WAKE_TIMEZONE_H
#define WAKE_TIMEZONE_H

#include <time.h>
#include "timezone_database.h"

#ifdef __cplusplus
extern "C" {
#endif
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