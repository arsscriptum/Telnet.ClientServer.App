
//==============================================================================
//
//   log.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#ifndef __RECON_LOG_H__
#define __RECON_LOG_H__

#include "stdafx.h"
#include "compilation_settings.h"
#include "log_types.h"
#include <sstream>
#include <string>
#include <stdio.h>
#include <iostream>
#include <ctime>




typedef std::pair<std::string, std::string> EndOfLineEscapeTag;


struct EndOfLineEscapeStreamScope
{
protected:
    EndOfLineEscapeTag tag;
    std::ostream& os;

    EndOfLineEscapeStreamScope(const EndOfLineEscapeTag& ttag, std::ostream& tout) : tag(ttag),
        os(tout)
    {
        os << tag.first; //you can overload this for custom ostream types with a different color interface
        //this might also have a stack interface for if you need multiple resets
    }
    friend EndOfLineEscapeStreamScope operator<<(std::ostream& out, const EndOfLineEscapeTag& tg);

public:
    template <class T>
    EndOfLineEscapeStreamScope& operator<<(T&& t)
    {
        os << std::forward<T>(t);
        return *this;
    }

    EndOfLineEscapeStreamScope& operator<<(std::ostream& (&M)(std::ostream&))
    {
        M(os);
        return *this;
    }

    ~EndOfLineEscapeStreamScope()
    {
        os << tag.second;
    }
};

inline EndOfLineEscapeStreamScope operator<<(std::ostream& os, const EndOfLineEscapeTag& tg)
{
    return EndOfLineEscapeStreamScope(tg, os);
}

#ifndef DEJA_DISABLED
	#include "DejaLib.h"
#endif


void __cdecl ConsoleOut( std::string color, const char *format, ...);
void __cdecl ConsoleDebugLog(const char *format, ...);
void __cdecl ConsoleErrorLog(const char *format, ...);

# define cprint_r( ... )                    { ConsoleOut(CONSOLE_COLOR_RED,  __VA_ARGS__ );  }	
# define cprint_rb( ... )                   { ConsoleOut(CONSOLE_COLOR_RED_BRIGHT,  __VA_ARGS__ );  }	
# define cprint_y( ... )                    { ConsoleOut(CONSOLE_COLOR_YELLOW,  __VA_ARGS__ );  }	
# define cprint_yb( ... )                   { ConsoleOut(CONSOLE_COLOR_YELLOW_BRIGHT,  __VA_ARGS__ );  }	
# define cprint_g( ... )                    { ConsoleOut(CONSOLE_COLOR_GREEN,  __VA_ARGS__ );  }	
# define cprint_c( ... )                    { ConsoleOut(CONSOLE_COLOR_CYAN,  __VA_ARGS__ );  }
# define cprint_cb( ... )                   { ConsoleOut(CONSOLE_COLOR_CYAN,  __VA_ARGS__ );  }
# define cprint_m( ... )                    { ConsoleOut(CONSOLE_COLOR_MAGENTA,  __VA_ARGS__ );  }	
# define cprint_b( ... )                    { ConsoleOut(CONSOLE_COLOR_BLUE,  __VA_ARGS__ );  }	
# define cprint_bb( ... )                   { ConsoleOut(ANSI_TEXT_COLOR_BLUE_BRIGHT,  __VA_ARGS__ );  }	
# define cprint_info( ... )                 { ConsoleOut( CONSOLE_COLOR_BKGRND_WHITE, __VA_ARGS__ );  }	

#define debug_printf( ... )

int WriteToLog(char* str);
#ifndef DEJA_DISABLED
	# define LOG_TRACE                     DEJA_TRACE	
	# define LOG_INFO                      DEJA_TRACE
	# define LOG_WARNING                   DEJA_WARNING
	# define LOG_ERROR                     DEJA_ERROR
#else
	# define LOG_PROFILE( channel, ... ) 
	# define LOG_TRACE( channel, ... )
	# define LOG_INFO( channel, ... )
	# define LOG_WARNING( channel, ... )
	# define LOG_ERROR( channel, ... ) 
#endif

#ifdef UNICODE
#  define			_XPLATFORM_COPY		wcsncpy
#else
#  define			_XPLATFORM_COPY		strncpy
#endif // UNICODE

class LogContext
{
public:
    LogContext(LPCSTR lbl) {
        SetLabel(lbl);
        DEJA_TRACE("LogContext::Enter", "Context \"%s\"", lbl);
    }
    ~LogContext() {
        DEJA_TRACE("LogContext::Exit", "Context \"%s\"", GetLabel());
    }
protected:
    void SetLabel(LPCSTR lbl) { _XPLATFORM_COPY(_label, lbl, MAX_PATH - 1); }
    LPCSTR GetLabel() { return _label; }

private:
    TCHAR _label[MAX_PATH];
};


#if ENABLE_LOG_CONTEXT
#   define CONTEXT_BLOCK( label )            { DEJA_CONTEXT( label ); LogContext contextBlock( label ) ; }
#else
#   define CONTEXT_BLOCK( label )
#endif // ENABLE_LOG_CONTEXT



#endif //__CORELIB_LOG_H__

