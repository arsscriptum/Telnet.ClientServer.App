
//==============================================================================
//
//   targetver.h
//
//   Platform detection and defines
//   If you wish to build your application for a previous Windows platform, 
//   include WinSDKVer.h and set the _WIN32_WINNT macro to the platform 
//   you wish to support before including SDKDDKVer.h.
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <cybercastor@icloud.com>
//==============================================================================

#ifndef __TARGET_COMMON_PREPROCESSOR_DEFINITIONS__
#define __TARGET_COMMON_PREPROCESSOR_DEFINITIONS__


#include <SDKDDKVer.h>


// platform detection
// from: https://gcc.gnu.org/onlinedocs/cpp/Predefined-Macros.html
#if defined(_WIN32) // for Windows (32-bit and 64-bit, this part is common)
#define PLATFORM_PC
#ifdef _WIN64 // for Windows (64-bit only)
#define PLATFORM_WIN64
#else // _WIN64
#define PLATFORM_WIN32 // for Windows (32-bit only)
//#pragma message("PLATFORM_WIN32 DETECTED")  
#endif // _WIN64
#elif defined(__APPLE__)
#pragma message("PLATFORM_MAC DETECTED")  
#define PLATFORM_MAC
#include "TargetConditionals.h"

#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR // for iPhone simulator   
#define PLATFORM_IPHONESIM
#pragma message("PLATFORM_IPHONESIM DETECTED")  
#elif defined(TARGET_OS_IPHONE) // for iPhone
#define PLATFORM_IPHONE
#pragma message("PLATFORM_IPHONE DETECTED")  
#else
#define TARGET_OS_OSX 1
#define PLATFORM_OSX
#pragma message("PLATFORM_OSX DETECTED")  
#endif
#elif defined(__linux)
#define PLATFORM_LINUX
#pragma message("PLATFORM_LINUX DETECTED")   
#else
#pragma message("UNSUPPORTED PLATFORM !")  // this might be Unix or Cygwin... unsupported.
#error Failing compilation because of platform
#endif // _WIN32



#ifdef PLATFORM_PC
#include <conio.h>
#endif // PLATFORM_PC

#ifdef PLATFORM_LINUX
#include <sys/ioctl.h>
#include <termios.h>

bool kbhit();
#endif

#include <string.h>


#ifdef _WIN64
#  define PLATFORM_PATH _T("x64")
#else
#  define PLATFORM_PATH _T("Win32")
#endif


#ifdef _TARGET_FINAL
#  define CONFIG_PATH _T("Final")
#elif _TARGET_DEBUG
#  define CONFIG_PATH _T("Debug")
#elif _TARGET_DEBUG_DLL
#  define CONFIG_PATH _T("DebugDLL")
#elif _DEBUG_DLL_DYNAMIC_MFC
#  define CONFIG_PATH _T("DebugDLL_DynamicMFC")
#elif _TARGET_RELEASE 
#  define CONFIG_PATH _T("Release")
#elif _TARGET_RELEASE_DLL
#  define CONFIG_PATH _T("ReleaseDLL")
#elif _TARGET_RELEASE_DLL_DYNAMIC_MFC
#  define CONFIG_PATH _T("ReleaseDLL_DynamicMFC")
#else
#pragma message("UNSUPPORTED CONFIGURATION !") 
#endif

#define TARGET_BASE_PATH _T("F:\\Code\\Telnet.ClientServer.App\\bin")
#define TARGET_BASE_NAME _T("recon_srv")

#define SYSTEM32_BASE_PATH _T("C:\\Windows\\System32")



#ifdef _SERVICE_DLL
#  define TARGET_EXT _T(".dll")
#else
#  define TARGET_EXT _T("exe")
#endif


std::string GetService_DevelopmentPath();
std::string GetService_TestPath();
std::string GetService_ProductionPathExe();
std::string GetService_ProductionPathDll();

#endif //__TARGET_COMMON_PREPROCESSOR_DEFINITIONS__


