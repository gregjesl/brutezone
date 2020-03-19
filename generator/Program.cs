using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NodaTime;
using NodaTime.TimeZones;
using System.Threading;

namespace brutezone
{
    /// <summary>
    /// The data structure for one UTC offset
    /// </summary>
    class Entry
    {
        /// <summary>
        /// The time the UTC offset starts in the timezone
        /// </summary>
        public DateTimeOffset StartTime;

        /// <summary>
        /// The UTC offset (in minutes)
        /// </summary>
        public int Offset;
    }

    class PointerEntry
    {
        public string Name;
        public int NumEntries;
    }

    class Program
    {
        static void Main(string[] args)
        {
            // Create a semaphore for controlling access to the results
            var semaphore = new SemaphoreSlim(1, 1);

            // The UTC start time of the lookup table
            DateTime StartTime = DateTime.SpecifyKind(new DateTime(1970, 1, 1, 0, 0, 0), DateTimeKind.Utc);

            // The UTC stop time of the lookup table
            DateTime StopTime = DateTime.SpecifyKind(new DateTime(2038, 1, 19, 0, 0, 0), DateTimeKind.Utc);

            // The epoch for the table
            // 1 January 1970 is the Unix epoch
            DateTime Epoch = DateTime.SpecifyKind(new DateTime(1970, 1, 1, 0, 0, 0), DateTimeKind.Utc);

            // A dictionary used to store the results
            // The key is the name of the timezone
            // The value is a list of UTC offsets
            var results = new Dictionary<string, List<Entry>>();

            // Loop through all canonical Timezones
            // Run in prallel to make things faster
            Parallel.ForEach(TzdbDateTimeZoneSource.Default.GetIds()
                             .Where(ZoneId => TzdbDateTimeZoneSource.Default.CanonicalIdMap[ZoneId] == ZoneId),
                             (ZoneId) =>
            {
                var timezone = TzdbDateTimeZoneSource.Default.ForId(ZoneId);

                // Initialize the list of entries
                var entries = new List<Entry>();

                // Initialize the current time
                DateTime currentTime = StartTime;

                do
                {
                    var interval = timezone.GetZoneInterval(Instant.FromDateTimeUtc(currentTime));

                    if (entries.Count == 0 || interval.WallOffset.Seconds != entries.Last().Offset) {
                        Entry entry = new Entry()
                        {
                            StartTime = currentTime,
                            Offset = interval.WallOffset.Seconds
                        };
                        entries.Add(entry);
                    }

                    if (!interval.HasEnd) {
                        break;
                    }
                    currentTime = interval.End.ToDateTimeUtc();
                }
                while (currentTime < StopTime);

                // Wait for access to the results structure
                semaphore.Wait();

                // Record the result for the timezone
                results.Add(ZoneId, entries);

                // Report the results
                Console.WriteLine($"{ZoneId}: {entries.Count} entries");

                // Release the lock
                semaphore.Release();
            });

            // Build the path to the header file
            string path = "../"; // from (debug/release) to bin
            path += "../"; // from bin to generator
            path += "../inc/"; // from generator to inc

            // Open the file
            using (System.IO.StreamWriter file =
                    new System.IO.StreamWriter(path + "timezone_database.h"))
            {
                file.WriteLine(
@"#ifndef BRUTEZONE_TIMEZONE_DATABASE_H
#define BRUTEZONE_TIMEZONE_DATABASE_H

#include <string.h>
#include <time.h>

#ifdef _MSC_VER

#pragma pack(push,1)
typedef struct {
    const time_t start;
    const short offset;
} timezone_offset;

typedef struct {
    const char *name;
    const timezone_offset *entries;
    const unsigned char n_entries;
} tzdb_timezone;

#pragma pack(pop)

#else

typedef struct {
    const time_t start;
    const short offset;
} __attribute__((packed)) timezone_offset;

typedef struct {
    const char *name;
    const timezone_offset *entries;
    const unsigned char n_entries;
} __attribute__((packed)) tzdb_timezone;

#endif
");

                file.WriteLine($"#define BRUTEZONE_DATABASE_MIN_TIME {(StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond}");
                file.WriteLine($"#define BRUTEZONE_DATABASE_MAX_TIME {(StopTime - Epoch).Ticks / TimeSpan.TicksPerSecond}");
                file.WriteLine(@"
#ifndef BRUTEZONE_MIN_TIME
#define BRUTEZONE_MIN_TIME BRUTEZONE_DATABASE_MIN_TIME
#endif

#ifndef BRUTEZONE_MAX_TIME
#define BRUTEZONE_MAX_TIME BRUTEZONE_DATABASE_MAX_TIME
#endif

#if BRUTEZONE_MIN_TIME >= BRUTEZONE_MAX_TIME
#error BRUTEZONE_MIN_TIME cant be larger than BRUTEZONE_MAX_TIME
#endif

#if BRUTEZONE_MIN_TIME < BRUTEZONE_DATABASE_MIN_TIME || BRUTEZONE_MIN_TIME >= BRUTEZONE_DATABASE_MAX_TIME
#error BRUTEZONE_MIN_TIME is outside of the database range
#endif

#if BRUTEZONE_MAX_TIME <= BRUTEZONE_DATABASE_MIN_TIME || BRUTEZONE_MAX_TIME > BRUTEZONE_DATABASE_MAX_TIME
#error BRUTEZONE_MAX_TIME is outside of the database range
#endif
");

                // The pointers dictionary associates a timezone string with a memory location for the array of UTC offset entries
                var pointers = new Dictionary<string, PointerEntry>();

                // Group all of the single timezones
                var singleTimezones = results.Where(t => t.Value.Count == 1)
                    .GroupBy(g => g.Value.First().Offset)
                    .ToList();

                // Record all of the single UTC offsets
                file.WriteLine($"static const timezone_offset timezone_database_no_change[{singleTimezones.Count}] =");
                file.WriteLine("{");
                int i = 0;
                var singleStrings = new List<string>();
                foreach(var singleZone in singleTimezones.OrderBy(g => g.First().Value.First().Offset))
                {
                    singleStrings.Add($"\t{{{(singleZone.First().Value.First().StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond},{singleZone.First().Value.First().Offset / 10}}}");
                    foreach(var zone in singleZone)
                    {
                        pointers.Add(zone.Key, new PointerEntry(){Name = $"&timezone_database_no_change[{i}]", NumEntries = 1});
                    }
                    i++;
                }
                file.WriteLine(string.Join(",\n", singleStrings.ToArray()));
                file.WriteLine("};");
                file.WriteLine("");

                // Record all of the timezones with more than one UTC offset
                foreach (var result in results.Where(t => t.Value.Count > 1).OrderBy(t => t.Key))
                {
                    // Example format: 
                    /*
                    const timezone_offset timezone_database_america_los_angeles[] =
                    {
                        {1,2,3},
                        {4,5,6}
                    };
                    */
                    if(result.Value.Count() > 255)
                    {
                        throw new IndexOutOfRangeException("Current implementation uses an unsigned char");
                    }

                    file.WriteLine($"static const timezone_offset timezone_database_{result.Key.Replace('/', '_').Replace('-', '_').ToLower()}[] =");
                    file.WriteLine("{");
                    var offsets = result.Value.OrderBy(e => e.StartTime);
                    for(int j = 0; j < offsets.Count(); ++j)
                    {
                        var entry = offsets.ElementAt(j);
                        if (j < offsets.Count() - 1) {
                            file.WriteLine($"#if BRUTEZONE_MIN_TIME < {(offsets.ElementAt(j + 1).StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond}");
                        }
                        if (j > 0) {
                            file.WriteLine($"#if BRUTEZONE_MAX_TIME > {(entry.StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond}");
                        }
                        file.WriteLine($"\t{{{(entry.StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond},{entry.Offset / 10}}},");
                        if (j > 0) {
                            file.WriteLine($"#endif");
                        }
                        if (j < offsets.Count() - 1) {
                            file.WriteLine($"#endif");
                        }
                    }
                    file.WriteLine("};");
                    file.WriteLine("");

                    pointers.Add(result.Key, new PointerEntry(){
                        Name = $"timezone_database_{result.Key.Replace('/', '_').Replace('-','_').ToLower()}",
                        NumEntries = result.Value.Count});
                }

                foreach (var Alias in TzdbDateTimeZoneSource.Default.Aliases)
                {
                    foreach(var AliasedZoneId in Alias) {
                        pointers.Add(AliasedZoneId, pointers[Alias.Key]);
                    }
                }

                // Write out the list of timezones and the associated memory locations
                file.WriteLine($"#define TIMEZONE_DATABASE_COUNT {pointers.Count}");
                file.WriteLine("#define TIMEZONE_ARRAY_COUNT(a) sizeof(a)/sizeof(*a)");
                file.WriteLine("static const tzdb_timezone timezone_array[] =");
                file.WriteLine("{");
                var tzlist = new List<string>();
                foreach (var result in pointers.OrderBy(t => t.Key, StringComparer.Ordinal))
                {
                    if (result.Value.NumEntries == 1) {
                        tzlist.Add($"\t{{\"{result.Key}\", {result.Value.Name}, {result.Value.NumEntries}}}");
                    }
                    else {
                        tzlist.Add($"\t{{\"{result.Key}\", {result.Value.Name}, TIMEZONE_ARRAY_COUNT({result.Value.Name})}}");
                    }
                }
                file.WriteLine(string.Join(",\n", tzlist.ToArray()));
                file.WriteLine("};");
                file.WriteLine("");
                file.WriteLine("#endif");
            }
        }
    }
}
