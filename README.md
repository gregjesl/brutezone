[![Build status](https://ci.appveyor.com/api/projects/status/gbhod5iaf6swa9fv?svg=true)](https://ci.appveyor.com/project/gregjesl/brutezone)
# Brutezone
Brutezone is a C library for computing the time in [IANA timezones](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones). It uses a lookup table to get the UTC offset of the timezone. The code for the lookup table generator (written in C#) is [included in the project](/generator); you can shrink the lookup table if desired. 

## Important Notes
The UTC offsets are stored in a [lookup table](/inc/timezone_database.h). **This library is only valid for UTC times between [1 January 1970](https://en.wikipedia.org/wiki/Unix_time) and [19 January 2038](https://en.wikipedia.org/wiki/Year_2038_problem).**

If your application is memory-constrained (like in some embedded systems), **you can limit the time-range of the database by defining the minimum and/or the maximum times you care about as UTC time_t for BRUTEZONE_MIN_TIME or BRUTEZONE_MAX_TIME during compilation.**

If you are running this library on an embedded device, **ensure you have [clock synchronization](https://en.wikipedia.org/wiki/Network_Time_Protocol) implemented.** 
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

### Handling Timestamps
- `iso_time` - Populates the `str` input with the [ISO 8601](https://en.wikipedia.org/wiki/ISO_8601) timestamp of the `datetime` argument. Example output: `1970-01-29T10:03:17` for 29 January 1970 at 10:03:17 AM. 
- `from_iso_time` - Returns the time structure from the given timestamp. 

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

If you have Visual Studio and/or MonoDevelop installed, you can build the generator by running
```
make generator
```

Once you have build the generator, you can rebuild the timezone table by running
```
make database
```

## Motivation
We needed a simple-to-use, compact library for an alarm clock on an embedded system. Brutezone was the result. It's called Brutezone because the UTC offsets are calculated for each timezone using brute force (and [NodaTime](https://nodatime.org/))

## Credits
The functions used to translate between `time_t` and `struct tm` were borrowed from [musl](http://www.musl-libc.org/)

The UTC offset lookup table was derived using the [NodaTime](https://nodatime.org/) library. 
