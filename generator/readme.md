# How to use the generator

The entire generator code is contained in [Program.cs](Program.cs). CMake creates targets for building and running the generator. After [running CMake](../#setup), run
```
make generator
```
and
```
make database
```
to compile the generator and (re)build the [timezone database header file](../inc/timezone_database.h), respectively.  

## Requirements
Building the generator requires Visual Studio and/or MonoDevelop.  Visual Studio is available for [Windows](https://visualstudio.microsoft.com/) and [Mac](https://visualstudio.microsoft.com/vs/mac/); MonoDevelop is availabile for [Linux](https://www.monodevelop.com/download/). 

## Reducing file size
To reduce the file size of the database, change `StartTime` and `StopTime` in `Program.cs`.  The smaller the window, the smaller the file size. 

Another option for reducing file size is filtering out un-needed timezones. For example, you may not need the Antartica timezones...

## Credits
As noted on the main page, this generator uses [NodaTime](https://nodatime.org/). 