#ifndef BRUTEZONE_TIMEZONE_IMPL_H
#define BRUTEZONE_TIMEZONE_IMPL_H

/* 2000-03-01 (mod 400 year, immediately after feb29 */
#define LEAPOCH (946684800LL + 86400 * (31 + 29))

#define DAYS_PER_400Y (365 * 400 + 97)
#define DAYS_PER_100Y (365 * 100 + 24)
#define DAYS_PER_4Y (365 * 4 + 1)

#ifdef __cplusplus
extern "C" {
#endif
long long year_to_secs(long long year, int *is_leap);
int month_to_secs(int month, int is_leap);
#ifdef __cplusplus
}
#endif

#endif