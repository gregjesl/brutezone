# brutezone
IANA timezone library for C

## TL;DR

Brutezone is a C library for computing the time in [IANA timezones](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones). It uses a lookup table to get the UTC offset of the timezone. The code for the lookup table generator (written in C#) is [included in the project](/generator); you can shrink the lookup table if desired. 

## Important Notes
The UTC offsets are stored in a [lookup table](/inc/timezone_database.h). **This library is only valid for UTC times between [1 January 1970](https://en.wikipedia.org/wiki/Unix_time) and [19 January 2038](https://en.wikipedia.org/wiki/Year_2038_problem).**

If your application is memory-constrained (like in some embedded systems), **you can customize the lookup table by running the [generator](/generator).**

If you are running this library on an embedded device, ensure you have [clock synchronization](https://en.wikipedia.org/wiki/Network_Time_Protocol) implemented. 
## Usage

### Getting the time
- `time_t timezone_local_time` - Translates the UTC time `gmt` into the time in the timezone named `timezone`
- `time_t timezone_current_local_time` - Returns the current local time in the timezone named `timezone`
- `time_t timezone_gmt_time` - Converts `local_time` from the timezone named `timezone` to UTC time

### Converting between `time_t` and `struct tm`
- `int secs_to_tm` - Converts a tick counter (such as `time_t`) to a time structure
- `long long tm_to_secs` - Converts a time structure to a tick counter

### Adding and subtracting days
- `void add_day` - Adds a day
- `void subtract_day` - Subtracts a day

### Computing time occurances
- `time_t timezone_secs_until` - Returns the number of seconds until the next occurance of `hour:minute:second` in the timezone named `timezone`
- `time_t timezone_secs_since` - Returns the number of seconds since the last occurance of `hour:minute:second` in the timezone named `timezone`

### Computing day of week & time occurances
- `time_t timezone_secs_until_dow` - Returns the number of seconds until the next occurance of `hour:minute:second` on `wday` (day of the week) in the timezone named `timezone`
- `time_t timezone_secs_since_dow` - Returns the number of seconds since the last occurance of `hour:minute:second` on `wday` (day of the week) in the timezone named `timezone`

### Examples
See [the test folder](/test) for examples of all of the functions. 

## Setup

The library can be compiled through the standard build process:
``` bash
mkdir build
cd build
cmake ../
make
```

Once the library is built, you can test it by running
```
make test
```

## Motivation
We needed a simple-to-use, compact library for an alarm clock on an embedded system. Brutezone was the result. 

## Credits
The functions used to translate between `time_t` and `struct tm` were borrowed from [musl](http://www.musl-libc.org/)