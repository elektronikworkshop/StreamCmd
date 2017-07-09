/** CommandSets, a StreamCmd command set example
 *
 *  Copyright (c) 2017 Elektronik Workshop
 **
 *  This example demonstrates the use of StreamCmd's command sets.
 **
 *  This file is part of the StreamCmd library and covered by the GNU Lesser
 *  General Public License v3.0, see the LICENSE file for more details on the
 *  license.
 */

#include <StreamCmd.h>

class Cli
  /* This time we inherit from the "real" thing as we'd like to use more than
   *  one command set:
   */
  : public StreamCmd</* _NumCommandSets = */ 2>
{
public:
  typedef enum
  {
    Operation = 0,
    Authentication,
  } CommandSetId;
  /* We use the default parameters in the base class constructor */
  Cli()
  {
    /* Not necessary -- after initialization we're on set 0 per default.
     * This is for the sake of clarity.
     */
    switchCommandSet(Operation);
    /* Setting up the authenticated command set */
    addCommand("help", &Cli::help);
    addCommand("quit", &Cli::quit);

    /* Switching to the login command set */
    switchCommandSet(Authentication);
    /* Setting up the login function */
    setDefaultHandler(&Cli::auth);
  }
protected:
  void auth(const char* command)
  {
    if (strcmp(command, "password") == 0) {
      switchCommandSet(Operation);
      Serial.println("successfully authenticated. try \"help\"");
    } else {
      Serial.println("invalid password, try \"password\"");
    }
  }
  void quit()
  {
    switchCommandSet(Authentication);
    Serial.println("you've been logged out successfully");
  }
  void help()
  {
    Serial.print(
      "CommandSets, a StreamCmd command sets example\n"
      "help -- print this help\n"
      "quit -- log out\n"
      );
  }
} cli;

void setup()
{
  Serial.begin(9600);
  while (not Serial)
  {}
  Serial.println("please authenticate:");
}

void loop()
{
  cli.run();
}

