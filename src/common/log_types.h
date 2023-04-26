
//==============================================================================
//
//   log_types.h 
//
//   this file purpose is just to define coors constants separately than all the
//   code, macros and logic pre-definitions. for pretty files.
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#ifndef __LOG_TYPES__
#define __LOG_TYPES__

// using standard exceptions
#include <iostream>
#include <exception>
using namespace std;

const std::string ANSI_TEXT_COLOR_RESET             = "\u001b[0m";
const std::string ANSI_TEXT_COLOR_BLACK             = "\u001b[30m";
const std::string ANSI_TEXT_COLOR_RED               = "\u001b[31m";
const std::string ANSI_TEXT_COLOR_GREEN             = "\u001b[32m";
const std::string ANSI_TEXT_COLOR_YELLOW            = "\u001b[33m";
const std::string ANSI_TEXT_COLOR_BLUE              = "\u001b[34m";
const std::string ANSI_TEXT_COLOR_MAGENTA           = "\u001b[35m";
const std::string ANSI_TEXT_COLOR_CYAN              = "\u001b[36m";
const std::string ANSI_TEXT_COLOR_WHITE             = "\u001b[37m";
const std::string ANSI_TEXT_COLOR_BLACK_BRIGHT      = "\u001b[30;1m";
const std::string ANSI_TEXT_COLOR_RED_BRIGHT        = "\u001b[31;1m";
const std::string ANSI_TEXT_COLOR_GREEN_BRIGHT      = "\u001b[32;1m";
const std::string ANSI_TEXT_COLOR_YELLOW_BRIGHT     = "\u001b[33;1m";
const std::string ANSI_TEXT_COLOR_BLUE_BRIGHT       = "\u001b[34;1m";
const std::string ANSI_TEXT_COLOR_MAGENTA_BRIGHT    = "\u001b[35;1m";
const std::string ANSI_TEXT_COLOR_CYAN_BRIGHT       = "\u001b[36;1m";
const std::string ANSI_TEXT_COLOR_WHITE_BRIGHT      = "\u001b[37;1m";
const std::string ANSI_TEXT_COLOR_BLACK_BKGRND      = "\u001b[40m";
const std::string ANSI_TEXT_COLOR_RED_BKGRND        = "\u001b[41m";
const std::string ANSI_TEXT_COLOR_GREEN_BKGRND      = "\u001b[42m";
const std::string ANSI_TEXT_COLOR_YELLOW_BKGRND     = "\u001b[43m";
const std::string ANSI_TEXT_COLOR_BLUE_BKGRND       = "\u001b[44m";
const std::string ANSI_TEXT_COLOR_MAGENTA_BKGRND    = "\u001b[45m";
const std::string ANSI_TEXT_COLOR_CYAN_BKGRND       = "\u001b[46m";
const std::string ANSI_TEXT_COLOR_WHITE_BKGRND      = "\u001b[47m";
const std::string CONSOLE_COLOR_BKGRND_WHITE_BLUE   = "\u001b[37;44;1m > ";
const std::string CONSOLE_COLOR_BKGRND_WHITE_GREEN  = "\u001b[32;44;1m > ";
const std::string CONSOLE_COLOR_BKGRND_YELLOW       = "\u001b[33;41;1m > ";
const std::string CONSOLE_COLOR_RED_BRIGHT          = "\u001b[31;1m";
const std::string CONSOLE_COLOR_GREEN_BRIGHT        = "\u001b[32;1m";
const std::string CONSOLE_COLOR_YELLOW_BRIGHT       = "\u001b[33;1m";
const std::string CONSOLE_COLOR_MAGENTA_BRIGHT      = "\u001b[35;1m";
const std::string CONSOLE_COLOR_CYAN_BRIGHT         = "\u001b[36;1m";
const std::string CONSOLE_COLOR_BLUE                = "\u001b[34m";
const std::string CONSOLE_COLOR_MAGENTA             = "\u001b[35m";
const std::string CONSOLE_COLOR_CYAN                = "\u001b[36m";
const std::string CONSOLE_COLOR_RED                 = "\u001b[31m";
const std::string CONSOLE_COLOR_GREEN               = "\u001b[32m";
const std::string CONSOLE_COLOR_YELLOW              = "\u001b[33m";



#endif //__LOG_TYPES__

