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
        /// The time the UTC offset ends in the timezone
        /// </summary>
        public DateTimeOffset EndTime;

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

                // Create the first entry
                Entry entry = new Entry()
                {
                    StartTime = currentTime,
                    Offset = timezone.GetUtcOffset(Instant.FromDateTimeUtc(currentTime)).Seconds
                };

                // Loop until the stop time
                do
                {
                    // Move the current time forward a minute
                    currentTime = currentTime.AddTicks(TimeSpan.TicksPerMinute);

                    // Check for a change in the UTC offset
                    if (timezone.GetUtcOffset(Instant.FromDateTimeUtc(currentTime)).Seconds != entry.Offset)
                    {
                        // Record the stop time of the UTC offset
                        entry.EndTime = currentTime;

                        // Record the UTC offset entry
                        entries.Add(entry);

                        // Create the new entry
                        entry = new Entry()
                        {
                            StartTime = currentTime,
                            Offset = timezone.GetUtcOffset(Instant.FromDateTimeUtc(currentTime)).Seconds
                        };
                    }
                }
                while (currentTime < StopTime);

                // Close the last remaining entry
                entry.EndTime = currentTime;

                // Store the last remaining entry
                entries.Add(entry);

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
                file.WriteLine("#ifndef TIMEZONE_DATABASE_H");
                file.WriteLine("#define TIMEZONE_DATABASE_H");
                file.WriteLine("");
                file.WriteLine("#include <string.h>");
                file.WriteLine("#include <time.h>");
                file.WriteLine("");
                file.WriteLine("typedef struct { const time_t start; const short offset; } timezone_offset;");
                file.WriteLine("typedef struct { const char *name; const timezone_offset *entries; size_t n_entries; } tzdb_timezone;");
                file.WriteLine("");
                file.WriteLine($"static const time_t timezone_offset_min_time = {(StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond};");
                file.WriteLine($"static const time_t timezone_offset_max_time = {(StopTime - Epoch).Ticks / TimeSpan.TicksPerSecond};");
                file.WriteLine("");

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
                    singleStrings.Add($"\t{{{(singleZone.First().Value.First().StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond},{singleZone.First().Value.First().Offset / 60}}}");
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
                    file.WriteLine($"static const timezone_offset timezone_database_{result.Key.Replace('/', '_').Replace('-', '_').ToLower()}[] =");
                    file.WriteLine("{");
                    var strList = new List<string>();
                    foreach(var entry in result.Value.OrderBy(e => e.StartTime))
                    {
                        strList.Add($"\t{{{(entry.StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond},{entry.Offset / 60}}}");
                    }
                    file.WriteLine(string.Join(",\n", strList.ToArray()));
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
                file.WriteLine("static const tzdb_timezone timezone_array[] =");
                file.WriteLine("{");
                var tzlist = new List<string>();
                foreach (var result in pointers.OrderBy(t => t.Key, StringComparer.Ordinal))
                {
                    tzlist.Add($"\t{{\"{result.Key}\", {result.Value.Name}, {result.Value.NumEntries}}}");
                }
                file.WriteLine(string.Join(",\n", tzlist.ToArray()));
                file.WriteLine("};");
                file.WriteLine("");

                // Write the helper function
                file.WriteLine("#ifdef __cplusplus");
                file.WriteLine("extern \"C\" {");
                file.WriteLine(
@"#endif
static inline const tzdb_timezone* find_timezone(const char *timezone_name)
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
        }
        else if (cmp < 0) {
            end = needle;
        }
        else {
            // Return the timezone if found
            return needle;
        }
    } while (begin < end);

    // If the timezone was not found, return null
    return NULL;
}
#ifdef __cplusplus
}
#endif"
                );

                file.WriteLine("");
                file.WriteLine("#endif");
            }
        }
    }
}
