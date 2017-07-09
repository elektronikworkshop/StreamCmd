/** CommandLineWithArguments, a StreamCmd example
 *
 *  Copyright (c) 2017 Elektronik Workshop
 *
 **
 *  This example demonstrates the use of the StreamCmd class and shows you
 *  how to build a simple but already pretty powerful command line interface
 *  which performs argument and option parsing with type and range checking.
 **
 *  This file is part of the StreamCmd library and covered by the GNU Lesser
 *  General Public License v3.0, see the LICENSE file for more details on the
 *  license.
 */

#include <StreamCmd.h>

class Cli
  /* Inherit from a the default template to keep it simple.
   *  The template allows you to configure the command line buffer size, the
   *  maximum command size, the maximum number of commands and the number of
   *  command sets (see the CommandSets example).
   */
  : public StreamCmdDefault
{
public:
  /* We use the default parameters. At this point you could customize your
   *  CLI even further (the stream to use, EOL character, command prompt)
   */
  Cli()
  {
    addCommand("help", &Cli::help);
    addCommand("add",  &Cli::add);
    addCommand("led",  &Cli::led);
    setDefaultHandler(&Cli::invalid);
  }
protected:
  void help()
  {
    Serial.print(
      "CommandLineWithArguments, a StreamCmd example\n"
      "help         -- print this help\n"
      "add <a> <b>  -- add two numbers\n"
      "led <on|off> -- turn the built in LED on or off\n"
      );
  }
  void invalid(const char* command)
  {
    if (command and strlen(command)) {
      Serial.print("no such command \"");
      Serial.print(command);
      Serial.println("\" -- try \"help\"");
    }
  }
  /* Demonstrates the use of the numerical argument parsing interface */
  void add()
  {
    int a = 0, b = 0;

    /* You can be nice and perform complex argument checking */
    switch (getInt(a, -5, 5)) {
      case ArgTooSmall:
        Serial.println("first number too small, it should be in the range -5 .. 5");
        return;
      case ArgTooBig:
        Serial.println("first number too big, it should be in the range -5 .. 5");
        return;
      case ArgInvalid:
        Serial.println("first number invalid, allowed are integers in the range -5 .. 5");
        return;
      default:
      case ArgNone:
        Serial.println("first number missing");
        return;
      case ArgOk:
        break;
    }
    /* Or keep it simple for you but confusing to the user :) */
    if (getInt(b, 23, 44) != ArgOk) {
      Serial.println("second number missing or invalid, allowed: 23 .. 44)");
      return;
    }
    Serial.print(a);
    Serial.print(" + ");
    Serial.print(b);
    Serial.print(" = ");
    Serial.println(a + b);
  }
  /* Demonstrates the use of the option interface
   *
   * Note: as soon as std::forward is available to the AVR platform this
   * gets even more elegant:
   *
   *   size_t idx(0);
   *   if (getOpt(idx, "off", "on") != ArgOk) {
   *     ...
   *
   * The above already works on the ESP platforms though!
   */
  void led()
  {
    size_t idx(0);
    enum {OFF = 0, ON, NOPT};
    const char* options[NOPT] = {"off", "on"};
    if (getOption(idx, options, NOPT) != ArgOk) {
      Serial.print("invalid argument \"");
      Serial.print(current());
      Serial.println("\", should be either \"on\" or \"off\"");
      return;
    }
    digitalWrite(LED_BUILTIN, idx);
  }
} cli;

void setup()
{
  Serial.begin(9600);
  while (not Serial)
  {}
  Serial.println("type \"help\"");

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  cli.run();
}
