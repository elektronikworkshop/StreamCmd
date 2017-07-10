# *StreamCmd* - A pluggable command line handler for Arduino Stream objects
*StreamCmd* enables you to set up command line interfaces (CLI) which then will work on different kinds of Stream-objects like Serial, WiFiClient and so on.

For instance you can run the same CLI-object on the Serial interface but re-use it for [Telnet](../TelnetServer) or other interfaces which provide a Stream interface (if not: just write one - it's pretty simple).

*StreamCmd* features a neat argument parsing interface which allows you to fetch different flavors of numerical command arguments and lets you parse string options in a convenient manner. For instance:

```c++
size_t idx(0);
if (getOpt(idx, "off", "on") == ArgOk) {
  digitalWrite(LED_BUILTIN, idx);
}
// or
float f = 0.;
switch (getFloat(f, 1.41, 3.14)) {
  case ArgOk:
    break;
  case ArgNone:      /* no argument provided  */
  case ArgTooSmall:  /* argument < 1.41       */
  case ArgTooBig:    /* argument > 3.14       */
  case ArgInvalid:   /* argument not a number */
    break;
}  
```

Furthermore *StreamCmd* supports multiple different command sets which can be switched instantly. This allows you to swap functionality in and out. This comes in handy for instance when a user first has to pass some sort of authentication (you remember me mentioning telnet beforehand? :) )

And finally *StreamCmd* allows you to customize your CLIs with a prompt. This way the user knows on which machine he operates (if you use the host name as prompt for instance).

Inherit from *StreamCmd* and implement your callbacks as members, as shown in the [examples](#examples).

## Design notes
### Static memory allocation
The original design dynamically allocated the memory for the command dictionary. The decision to move to a compile time template-based scheme was that this will reduce memory fragmentation (every command add will reallocate) and most of the time you already know which commands you are about to incorporate. The library could be modified easily to get dynamic allocation back.

### History
This is a more or less complete rewrite of the SerialCommand library originally written by Steven Cogswell (2011) with modifications from Stefan Rado (2012).

The original library had several little issues like missing const correctness, old-style C++, no proper naming of member variables, some corner cases (Stream::read() can return -1 for instance), just to name a few. But the most annoying thing was the lack of context in the command callbacks which caused the rewrite.

### Todos
* Test on SAM, SAMD architectures - volunteers?
* Lambda support
* More complex callback dictionary to integrate help for the individual commands with the command itself which would simplify stuff like "help <command name>" and allows one to keep help and command close together what eases maintenance. Or even some sort of per command argument list with type and range checking like the boost command line parser implements but that's a rather huge task

## Installation
### Arduino IDE
1. Download the ZIP file (below) to your machine.
2. In the Arduino IDE, choose Sketch/Include Library/Add Zip Library
3. Navigate to the ZIP file, and click Open

--- or ---

In the Arduino IDE, choose Sketch/Include Library/Manage Libraries.  Click the StreamCmd Library from the list, and click the Install button.

## Compatible Hardware
No hardware dependencies.

## Examples
The library includes several examples to help you get started. These are accessible in the Examples/StreamCmd menu off the File menu in the Arduino IDE.
* **[CommandLineWithArguments](examples/CommandLineWithArguments/CommandLineWithArguments.ino):** An example showing off the basic usage and command argument parsing.
* **[CommandSets](examples/CommandSets/CommandSets.ino):** Shows how to use different command sets and how to switch between them.
* **[CommandStreams](examples/CommandStreams/CommandStreams.ino):** Shows how to use write one CLI and use it on different streams (Serial, telnet) -- to be added...

---
Copyright (c) 2017 [Elektronik Workshop](http://elektronikworkshop.ch)
