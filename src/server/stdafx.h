
//==============================================================================
//
//   stdafx.h 
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#ifndef __STDAFX_F__
#define __STDAFX_F__

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif

#include <afx.h>

#include <windows.h>

#include "log.h"


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#pragma message("BYTE ORDER ==> LITTLE ENDIAN")
#else
#pragma message("BYTE ORDER ==> BIG ENDIAN")
#endif
#ifdef UNICODE
#  define           _PRINTF         wprintf
#  define			_STRCMP		    wstrcmp
#  define			_STRRCHR		wcsrchr
#  define			_STRLEN			wcslen
#  define			__SNPRINTF		_snwprintf
#  define			__SPRINTF		_swprintf
#define				_STRNCPY		wcsncpy
#  define			_STRNLEN		wcsnlen
#else
#  define           _PRINTF         printf
#  define			_STRCMP		    strcmp
#  define			_STRRCHR		strrchr
#  define			_STRLEN			strlen
#  define			__SNPRINTF		_snprintf
#  define			__SPRINTF		_sprintf
#  define			_STRNCPY		strncpy
#  define			_STRNLEN		strnlen
#endif
#if defined _WIN32
#  define			__PATH_SEPARATOR  _T('\\')
#else
#  define			__PATH_SEPARATOR  _T('/')
#endif




#define bzero(a) memset(a,0,sizeof(a)) //easier -- shortcut

void ErrorMessage(char* str);

std::string base_name(std::string const& path);
typedef std::basic_string<TCHAR> String;
inline std::string string_base_name(std::string const& path);
void decomposePath(const TCHAR* filePath, TCHAR* fileDir, TCHAR* fileName, TCHAR* fileExt);
String GetErrorMessage(DWORD dwErrorCode);

#endif//__STDAFX_F__