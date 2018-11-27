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
    class Entry
    {
        public DateTimeOffset StartTime;
        public DateTimeOffset EndTime;
        public int Offset;
    }

    class Program
    {
        static void Main(string[] args)
        {
            var semaphore = new SemaphoreSlim(1, 1);
            DateTime StartTime = DateTime.UtcNow;
            // DateTime StartTime = DateTime.SpecifyKind(new DateTime(2018, 1, 1, 0, 0, 0), DateTimeKind.Utc);
            DateTime StopTime = DateTime.SpecifyKind(new DateTime(2030, 1, 1, 0, 0, 0), DateTimeKind.Utc);
            DateTime Epoch = DateTime.SpecifyKind(new DateTime(1970, 1, 1, 0, 0, 0), DateTimeKind.Utc);

            var results = new Dictionary<string, List<Entry>>();

            Parallel.ForEach(TzdbDateTimeZoneSource.Default.ZoneLocations, (timezoneLocation) => {
                var entries = new List<Entry>();
                var timezone = DateTimeZoneProviders.Tzdb[timezoneLocation.ZoneId];
                DateTime currentTime = StartTime;
                Entry entry = new Entry()
                {
                    StartTime = currentTime,
                    Offset = timezone.GetUtcOffset(Instant.FromDateTimeUtc(currentTime)).Seconds
                };
                do
                {
                    currentTime = currentTime.AddTicks(TimeSpan.TicksPerMinute);
                    if (timezone.GetUtcOffset(Instant.FromDateTimeUtc(currentTime)).Seconds != entry.Offset)
                    {
                        entry.EndTime = currentTime;
                        entries.Add(entry);
                        entry = new Entry()
                        {
                            StartTime = currentTime,
                            Offset = timezone.GetUtcOffset(Instant.FromDateTimeUtc(currentTime)).Seconds
                        };
                    }
                }
                while (currentTime < StopTime);
                entry.EndTime = currentTime;
                entries.Add(entry);

                semaphore.Wait();
                Console.WriteLine($"{timezoneLocation.ZoneId}: {results.Count} entries");
                results.Add(timezone.Id, entries);
                semaphore.Release();
            });

            string path = "../"; // from (debug/release) to bin
            path += "../"; // from bin to generator
            path += "../inc/"; // from generator to inc
            using (System.IO.StreamWriter file =
                    new System.IO.StreamWriter(path + "timezone_database.h"))
            {
                file.WriteLine("#ifndef TIMEZONE_DATABASE_H");
                file.WriteLine("#define TIMEZONE_DATABASE_H");
                file.WriteLine("");
                file.WriteLine("#include <time.h>");
                file.WriteLine("");
                file.WriteLine($"#define TIMEZONE_DATABASE_COUNT {results.Count}");
                file.WriteLine("");
                file.WriteLine("typedef struct { const time_t start; const time_t end; const short offset; } timezone_offset;");
                file.WriteLine("typedef struct { const char *name; const timezone_offset *entries; } timezone;");
                file.WriteLine("");
                file.WriteLine($"static const time_t timezone_offset_min_time = {(StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond};");
                file.WriteLine($"static const time_t timezone_offset_max_time = {(StopTime - Epoch).Ticks / TimeSpan.TicksPerSecond};");
                file.WriteLine("");

                var pointers = new Dictionary<string, string>();

                // Group all of the single timezones
                var singleTimezones = results.Where(t => t.Value.Count == 1)
                    .GroupBy(g => g.Value.First().Offset)
                    .ToList();

                file.WriteLine($"static const timezone_offset timezone_database_no_change[{singleTimezones.Count}] = ");
                file.WriteLine("{");
                int i = 0;
                var singleStrings = new List<string>();
                foreach(var singleZone in singleTimezones.OrderBy(g => g.First().Value.First().Offset))
                {
                    singleStrings.Add($"\t{{{(singleZone.First().Value.First().StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond},{(singleZone.First().Value.First().EndTime - Epoch).Ticks / TimeSpan.TicksPerSecond},{singleZone.First().Value.First().Offset / 60}}}");
                    foreach(var zone in singleZone)
                    {
                        pointers.Add(zone.Key, $"&timezone_database_no_change[{i}]");
                    }
                    i++;
                }
                file.WriteLine(string.Join(",\r\n", singleStrings.ToArray()));
                file.WriteLine("};");
                file.WriteLine("");

                foreach (var result in results.Where(t => t.Value.Count > 1).OrderBy(t => t.Key))
                {
                    /*
                    const timezone_offset timezone_database_america_los_angeles[] =
                    {
                        {1,2,3},
                        {4,5,6}
                    };
                    */
                    file.WriteLine($"static const timezone_offset timezone_database_{result.Key.Replace('/', '_').Replace('-', '_').ToLower()}[] = ");
                    file.WriteLine("{");
                    var strList = new List<string>();
                    foreach(var entry in result.Value.OrderBy(e => e.StartTime))
                    {
                        strList.Add($"\t{{{(entry.StartTime - Epoch).Ticks / TimeSpan.TicksPerSecond},{(entry.EndTime - Epoch).Ticks / TimeSpan.TicksPerSecond},{entry.Offset / 60}}}");
                    }
                    file.WriteLine(string.Join(",\r\n", strList.ToArray()));
                    file.WriteLine("};");
                    file.WriteLine("");

                    pointers.Add(result.Key, $"timezone_database_{result.Key.Replace('/', '_').Replace('-','_').ToLower()}");
                }


                file.WriteLine("static timezone timezone_array[TIMEZONE_DATABASE_COUNT] = ");
                file.WriteLine("{");
                var tzlist = new List<string>();
                foreach (var result in pointers.OrderBy(t => t.Key))
                {
                    tzlist.Add($"\t{{\"{result.Key}\", {result.Value}}}");
                }
                file.WriteLine(string.Join(",\r\n", tzlist.ToArray()));
                file.WriteLine("};");
                file.WriteLine("");

                // Write the helper function
                file.WriteLine(
@"inline const timezone* find_timezone(const char *timezone_name)
{
    unsigned int index;

    // Iterate through all timezones
    for(index = 0; index < TIMEZONE_DATABASE_COUNT; index++)
    {
        // Return the timezone if found
        if(strcmp(timezone_array[index].name, timezone_name) == 0) return &timezone_array[index];
    }

    // If the timezone was not found, return null
    return NULL;
}"
                );

                file.WriteLine("");
                file.WriteLine("#endif");
            }
        }
    }
}
