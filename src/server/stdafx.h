
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
#include "macros.h"
#include "nowarns.h"
#include "log.h"
#include "service_types.h"
#define PRINT_OUT

#define WIN32_LEAN_AND_MEAN
#include "targetver.h"

#include <windows.h>
#include <process.h>
#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <AccCtrl.h>
#include <winbase.h>
#include <wtsapi32.h>
#include <userenv.h>
#include <windowsx.h>
#include <shlobj.h>

#include <commctrl.h>       // InitCommonControlsEx, etc.
#include <stdexcept>


#ifdef RECON_STATIC_LINKING
#  define RECON_CALL_CONV WINAPI
#  define RECON_INTERFACE_DEF_NOCC
#  define RECON_INTERFACE_DEF(x)   x
#else
#  define RECON_CALL_CONV  WINAPI
#  ifdef RECON_EXPORTS
#    define RECON_INTERFACE_DEF_NOCC __declspec(dllexport)                    
#    define RECON_INTERFACE_DEF(x)   __declspec(dllexport) x RECON_CALL_CONV 
#  else
#    define RECON_INTERFACE_DEF_NOCC __declspec(dllimport)                    
#    define RECON_INTERFACE_DEF(x)   __declspec(dllimport) x RECON_CALL_CONV
#  endif
#endif



#ifdef UNICODE
#  define           _PRINTF         wprintf
#  define			_STRCPY		    wstrcpy
#  define			_STRCMP		    wstrcmp
#  define			_STRRCHR		wcsrchr
#  define			_STRLEN			wcslen
#  define			__SNPRINTF		_snwprintf
#  define			__SPRINTF		_swprintf
#define				_STRNCPY		wcsncpy
#  define			_STRNLEN		wcsnlen
#  define			_STRNCPY_S		wcsncpy_s
#  define			_STRNCAT_S		wcsncat_s
#  define			TO_STD_STRING	std::to_string
#  define			STD_STRING		std::wstring
#  define			MATCH(s, n) wstrcmp(section, s) == 0 && wstrcmp(name, n) == 0
#else
#  define           _PRINTF         printf
#  define			_STRCPY		    strcpy
#  define			_STRCMP		    strcmp
#  define			_STRRCHR		strrchr
#  define			_STRLEN			strlen
#  define			__SNPRINTF		_snprintf
#  define			__SPRINTF		_sprintf
#  define			_STRNCPY		strncpy
#  define			_STRNLEN		strnlen
#  define			TO_STD_STRING	std::to_string
#  define			STD_STRING		std::string
#  define			MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#endif
#if defined _WIN32
#  define			__PATH_SEPARATOR  _T('\\')
#else
#  define			__PATH_SEPARATOR  _T('/')
#endif


#define DESKTOP_ALL (DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW | \
DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD | \
DESKTOP_JOURNALPLAYBACK | DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | \
DESKTOP_SWITCHDESKTOP | STANDARD_RIGHTS_REQUIRED)

#define WINSTA_ALL (WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | \
WINSTA_ACCESSCLIPBOARD | WINSTA_CREATEDESKTOP | \
WINSTA_WRITEATTRIBUTES | WINSTA_ACCESSGLOBALATOMS | \
WINSTA_EXITWINDOWS | WINSTA_ENUMERATE | WINSTA_READSCREEN | \
STANDARD_RIGHTS_REQUIRED)


#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | \
GENERIC_EXECUTE | GENERIC_ALL)


#define bzero(a) memset(a,0,sizeof(a)) //easier -- shortcut

void ErrorMessage(char* str);

std::string base_name(std::string const& path);
typedef std::basic_string<TCHAR> String;
inline std::string string_base_name(std::string const& path);
void decomposePath(const TCHAR* filePath, TCHAR* fileDir, TCHAR* fileName, TCHAR* fileExt);
String GetErrorMessage(DWORD dwErrorCode);

#endif//__STDAFX_F__