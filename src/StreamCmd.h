/** StreamCmd - A pluggable command line handler for Arduino Stream objects like
 * Serial, WiFiClient, ... .
 *
 * Copyright (C) 2017 Elektronik Workshop <hoi@elektronikworkshop.ch>
 * http://elektronikworkshop.ch
 *
 * A more or less complete rewrite of the SerialCommand library of Stefan Rado
 * and Steven Cogswell. The original library had several little issues like
 * missing const correctness. Several nice features were added, see the README
 * for more information or take a look at the code.
 *
 ***
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EW_STREAM_CMD_H_
#define _EW_STREAM_CMD_H_

#if defined(WIRING) && WIRING >= 100
  #include <Wiring.h>
#elif defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

#include <cstring>
#include <climits>  /* INT_MIN, ... */
#include <cfloat>   /* FLT_MIN, ... */
#include <errno.h>  /* errno        */
#include <utility>  /* std::forward */

/** A Pluggable command line handler for Arduino Stream objects (Serial, ...).
 *
 *
 */
template<size_t _CommandBufferSize = 32,
         size_t _MaxCommandSize    =  8,
         size_t _MaxCommands       = 32,
         size_t _NumCommandSets    =  1>
class StreamCmd
{
public:
  /** Command buffer size */
  static const size_t CommandBufferSize = _CommandBufferSize;
  /** Maximum length of a command excluding the zero termination */
  static const size_t MaxCommandSize = _MaxCommandSize;
  /** Number of commands per set */
  static const size_t MaxCommands = _MaxCommands;
  /** Number of available command sets */
  static const size_t NumCommandSets = _NumCommandSets;

  /** Constructor.
   * @param stream
   * The stream object which will drive StreamCmd.
   * @param eol
   * The end of line (EOL) character to split the input stream into single
   * command lines.
   * @param prompt
   * A user configurable prompt, will result in "yourprompt> ", whereas the "> "
   * is appended automatically. Passing nullptr will disable the prompt.
   * Note that the pointer passed during construction must be valid for the
   * lifetime of this object.
   */
  StreamCmd(Stream& stream = Serial,
            char eol = '\n',
            const char* prompt = nullptr)
    : m_stream(stream)
//    , m_commandSets{0}
      // {nullptr, 0, DefaultCallback()},
      // {nullptr, 0, DefaultCallback()}}
    , m_currentCommandSet(0)
    , m_delimiter{' ', 0}
    , m_eol(eol)
    , m_prompt(prompt)
    , m_commandLine{0}
    , m_pos(0)
    , m_last(nullptr)
    , m_current(nullptr)
  { }
  /** Read the stream and run the CLI engine.
   * Usually called repeatedly in the main loop.
   */
  virtual void run()
  {
    while (m_stream.available() > 0) {

      int raw = m_stream.read();
      if (raw == -1) {
        break;
      }
      char ch = raw;

      if (ch == m_eol) {

        bool executed = false;

        /* tokenize command line */

        char *command = strtok_r(m_commandLine, m_delimiter, &m_last);

        /* this way current() will return the command when called before next() */
        m_current = command;

        if (command) {

          for (unsigned int i = 0; i < set().m_commandCount; i++) {

            auto& entry = set().m_commandList[i];

            if (strncmp(command, entry.command, MaxCommandSize) == 0) {
              (this->*entry.commandCallback)();
              executed = true;
              break;
            }
          }
        }

        if (not executed and set().m_defaultCallback) {
          (this->*set().m_defaultCallback)(command ? command : "");
        }

        if (m_prompt) {
          m_stream.print(m_prompt);
          m_stream.print("> ");
        }

        clearBuffer();

      } else if (isprint(ch)) {
        if (m_pos < CommandBufferSize) {
          m_commandLine[m_pos++] = ch;
          m_commandLine[m_pos  ] = '\0';
        } else {
          m_stream.println(
            "StreamCmd line buffer overflow -- increase StreamCmd's "
            "CommandBufferSize template argument");
        }
      }
    }
  }

  /** Clear input buffer. */
  void clearBuffer()
  {
    m_commandLine[0] = '\0';
    m_pos = 0;
  }

  /** Switch the command set.
   */
  bool switchCommandSet(uint8_t set)
  {
    if (set >= NumCommandSets) {
      return false;
    }
    m_currentCommandSet = set;
    return true;
  }

  /** Get the current active command set.
   */
  uint8_t getCommandSet() const
  {
    return m_currentCommandSet;
  }

  /** Get the associated stream object. */
  Stream& stream()
  {
    return m_stream;
  }

  /** Get the number of registered commands.
   * Provided for debugging purposes, i.e. to check if you run out of
   * command entry slots.
   */
  size_t getNumCommandsRegistered(uint8_t set) const
  {
    if (set >= NumCommandSets) {
      return 0;
    }
    return m_commandSets[set].m_commandCount;
  }
protected:

  /** Get next command line token (aka argument).
   * Returns the next command line argument or nullptr if there's none such.
   */
  const char *next()
  {
    m_current = strtok_r(nullptr, m_delimiter, &m_last);
    return m_current;
  }

  /** Only valid after next() has been called. As with next() this can
   * return nullptr, so don't shoot yourself in the foot, you've been warned!
   */
  const char *current()
  {
    return m_current;
  }

  /* Argument getters */

  typedef enum
  {
    ArgOk = 0,
    /** No argument, i.e. next() returned nullptr */
    ArgNone,
    /** Invalid argument, e.g. conversion to requesed number type failed */
    ArgInvalid,
    /** Argument didn't conform to the requested lower limit. */
    ArgTooSmall,
    /** Argument didn't conform to the requested upper limit. */
    ArgTooLarge,
    /** No match in options found. */
    ArgNoMatch,
  } GetResult;

  template<typename T, typename TI, typename F>
  GetResult getNum(T& num, T min, T max, int base, F strtoX)
  {
    const char* arg = next();
    if (not arg) {
      return ArgNone;
    }

    char *endptr = nullptr;
    errno = 0;

    TI _num = strtoX(arg, &endptr, base);

    if (errno or arg == endptr) {
      return ArgInvalid;
    }

    if (_num < min) {
      return ArgTooSmall;
    } else if (_num > max) {
      return ArgTooLarge;
    }
    num = _num;
    return ArgOk;
  }

  #define STREAM_CMD_MAKE_GETTER(NAME, T, TI, MIN, MAX, CONV)     \
    GetResult NAME(T& n, T min = MIN, T max = MAX)                \
    {                                                             \
      return StreamCmd::getNum<T, TI>(n, min, max, 10, CONV);     \
    }
  #define STREAM_CMD_MAKE_GETTER_B(NAME, T, TI, MIN, MAX, CONV)   \
    GetResult NAME(T& n, T min = MIN, T max = MAX, int base = 10) \
    {                                                             \
      return StreamCmd::getNum<T, TI>(n, min, max, base, CONV);   \
    }

  /** Wrapper function to make the template uniform.
   * Note: as of 2017-07-08 the ESP8266 port doesn't support strtof - linker
   * fails. So we use strtod even for float. But the strtoX functions are
   * much better than the atoX pendents since their error checking is much
   * superior.
   */
  static double _strtod(const char* str, char** str_end, int)
  { return strtod(str, str_end); }
  static long double _strtold(const char* str, char** str_end, int)
  { return strtold(str, str_end); }
  STREAM_CMD_MAKE_GETTER(getFloat, float, double, FLT_MIN, FLT_MAX, &StreamCmd::_strtod)
  STREAM_CMD_MAKE_GETTER(getDouble, double, double, DBL_MIN, DBL_MAX, &StreamCmd::_strtod)
  STREAM_CMD_MAKE_GETTER(getLDouble, long double, long double, LDBL_MIN, LDBL_MAX, &StreamCmd::_strtold)

  STREAM_CMD_MAKE_GETTER_B(getInt, int, long, INT_MIN, INT_MAX, strtol)
  STREAM_CMD_MAKE_GETTER_B(getUInt, unsigned int, unsigned long, 0, UINT_MAX, strtoul)

  STREAM_CMD_MAKE_GETTER_B(getLong, long, long, LONG_MIN, LONG_MAX, strtol)
  STREAM_CMD_MAKE_GETTER_B(getULong, unsigned long, unsigned long, 0, ULONG_MAX, strtoul)

  STREAM_CMD_MAKE_GETTER_B(getLLong, long long, long long, LLONG_MIN, LLONG_MAX, strtoll)
  STREAM_CMD_MAKE_GETTER_B(getULLong, unsigned long long, unsigned long long, 0, ULLONG_MAX, strtoull)

  /** Parse option argument and compare against a list of valid options.
   *
   * This variadic template function will allow you to write neat stuff like
   * this:
   *
   * <pre>
   * <code>
   * size_t idx(0);
   * enum {ON = 0, OFF};
   * switch (getOption(idx, "on", "off")) {
   *   case ArgOk:
   *     switch (idx) {
   *       case ON:
   *         // do whatever is necessary to switch it on!
   *         break;
   *       case OFF:
   *         // ...
   *         break;
   *     }
   *   case ArgNoMatch:
   *     Serial << "invalid argument \"" << current() << "\". use \"on\" or \"off\"\n";
   *     break;
   *  }
   * </code>
   * </pre>
   */
  template<typename... Vals>
  GetResult
  getOption(size_t& idx, Vals&&... vals)
  {
    const size_t N = sizeof... (Vals);
    const char* options[N] = { std::forward<Vals>(vals)... };
    return getOption(idx, options, N);
  }

  GetResult
  getOption(size_t& idx, const char** options, size_t size)
  {
    const char* arg = next();
    if (not arg) {
      return ArgNone;
    }

    for (size_t i = 0; i < size; i++) {
      if (strcmp(options[i], arg) == 0) {
        idx = i;
        return ArgOk;
      }
    }
    return ArgNoMatch;
  }

  typedef void(StreamCmd::*CommandCallback)(void);
  typedef void(StreamCmd::*DefaultCallback)(const char*);

  /** Add a command to the current command set.
   */
  void addCommand(const char *command, CommandCallback commandCallback)
  {
    auto& count = set().m_commandCount;

    if (count == MaxCommands) {
      /* WARNING
       *
       * This will go unnoticed when commands are registered in
       * constructor in global scope before main is called.
       * This is the drawback when not allocating dynamically.
       *
       * To avoid such problems, register commands in a begin() function
       * which again is called after stream initialization (e.g. Serial)
       */
      m_stream.print("StreamCmd command list overflow, dropping command \"");
      m_stream.print(command);
      m_stream.println("\", increase StreamCmd's MaxCommands template argument");
    }

    auto& e = set().m_commandList[count];
    strncpy(e.command, command, MaxCommandSize);
    e.commandCallback = commandCallback;

    count++;
  }

  template <typename T>
  void addCommand(const char *command, void(T::*m)(void))
  {
    addCommand(command, static_cast<CommandCallback>(m));
  }

  /** Set the default handler of the current command set.
   */
  void setDefaultHandler(DefaultCallback defaultCallback)
  {
    set().m_defaultCallback = defaultCallback;
  }

  template <typename T>
  void setDefaultHandler(void(T::*m)(const char*))
  {
    setDefaultHandler(static_cast<DefaultCallback>(m));
  }

private:
  /** The stream object on which StreamCmd should operate on. */
  Stream& m_stream;

  /** Command dictionary entry. */
  struct CommandEntry
  {
    char command[MaxCommandSize + 1];
    CommandCallback commandCallback;
  };

  /** Struct representing a command set. Each set features a
   *  regular command dictionary plus a default command handler.
   */
  struct CommandSet
  {
    CommandEntry m_commandList[MaxCommands];
    uint8_t m_commandCount;
    DefaultCallback m_defaultCallback;
  };

  CommandSet& set()
  {
    return m_commandSets[m_currentCommandSet];
  }

  CommandSet m_commandSets[NumCommandSets];
  uint8_t m_currentCommandSet;


  /** Delimiter for tokenizing the command line. Defaults to a single space. */
  char m_delimiter[2];
  /** The end of line (EOL) character. */
  char m_eol;
  /** A user configurable command promt name (host name for instance).
   * Must remain valid for the lifetime of the StreamCmd object.
   */
  const char* m_prompt;

  /** The command line buffer. */
  char m_commandLine[CommandBufferSize + 1];
  /** The current write position in the command line buffer. */
  uint8_t m_pos;
  /** strtok_r state variable */
  char *m_last;
  /** Current argument - only valid after calling next() */
  const char* m_current;
};

typedef StreamCmd<> StreamCmdDefault;

#endif /* _EW_STREAM_CMD_H_ */
