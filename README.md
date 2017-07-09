# *StreamCmd* - A pluggable command line handler for Arduino Stream objects
StreamCmd enables you to set up command line interfaces (CLI) which then will work on different kinds of Stream-objects like Serial, WiFiClient and so on.

For instance you can run the same CLI-object on the Serial interface but re-use it for telnet or other interfaces which provide a Stream interface (if not: just write one - it's pretty simple).

Furthermore StreamCmd supports multiple different command sets which can be switched instantly. This allows you to swap functionality in and out. This comes in handy for instance when a user first has to pass some sort of authentication (you remember me mentioning telnet beforehand? :) )

StreamCmd features a neat argument parsing interface which allows you to fetch different flavors of numerical command arguments and lets you parse string options in a convenient manner.

And finally StreamCmd allows you to customize your CLIs with a prompt. This way the user knows on which machine he operates (if you use the host name as prompt for instance).

Inherit from StreamCmd and implement your callbacks as members, as shown in the examples.

## Todo
* Lambda support
* More complex callback dictionary to integrate help for the individual commands with the command itself which would simplify stuff like "help <command name>" and allows one to keep help and command close together what eases maintenance. Or even some sort of per command argument list with type and range checking like the boost command line parser implements but that's a rather huge task

## Installation
#### Arduino IDE
In the Arduino IDE, choose Sketch/Include Library/Manage Libraries.  Click the StreamCmd Library from the list, and click the Install button.

--- or ---

1. Download the ZIP file (below) to your machine.
2. In the Arduino IDE, choose Sketch/Include Library/Add Zip Library
3. Navigate to the ZIP file, and click Open

## Compatible Hardware:
No hardware dependencies.


## Examples:
The library includes several examples to help you get started. These are accessible in the Examples/StreamCmd menu off the File menu in the Arduino IDE.
