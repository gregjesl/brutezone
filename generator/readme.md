# How to use the generator

The entire generator code is contained in [program.cs](program.cs). Simply running the generator from the /bin/(debug or release) folder (or running straight from Visual Studio) will overwrite the [timezone database header file](../inc/timezone_database.h). 

## Reducing file size
To reduce the file size of the database, change `StartTime` and `StopTime` in `Program.cs`.  The smaller the window, the smaller the file size. 

Another option for reducing file size is filtering out un-needed timezones. For example, you may not need the Antartica timezones...

## Credits
As noted on the [main page](../), this generator uses [NodaTime](https://nodatime.org/). 