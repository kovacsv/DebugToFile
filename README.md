# DebugToFile

DebugToFile can write the debug output of any application to a text file.

## Example usages

The first parameter is the path of the log file, the second is the path of the application to run. All of the remaining parameters are passed as an argument to the given application.

    DebugToFile.exe LogFile.txt MyApplication.exe
    DebugToFile.exe LogFile.txt MyApplication.exe -MyApplicationParameter

## Build

Build steps:
- Open the given Visual Studio 2015 solution.
- Build it.
