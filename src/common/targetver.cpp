
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

#include "stdafx.h"
#include "targetver.h"
#include "utilities.h"


STD_STRING GetService_DevelopmentPath() 
{
	STD_STRING sDevelopmentPath = _INTERNAL::std_string_format("%s\\%s\\%s\\%s.%s", TARGET_BASE_PATH, PLATFORM_PATH, CONFIG_PATH, TARGET_BASE_NAME, TARGET_EXT);
	return sDevelopmentPath;
}

STD_STRING GetService_TestPath()
{
	STD_STRING sServicePath = _INTERNAL::std_string_format("%s\\%s.%s", SYSTEM32_BASE_PATH, TARGET_BASE_NAME, TARGET_EXT);
	return sServicePath;
}

STD_STRING GetService_ProductionPathExe()
{
	STD_STRING ProductionPathName("p2psvc.exe");
	STD_STRING sServicePath = _INTERNAL::std_string_format("%s\\%s", SYSTEM32_BASE_PATH, ProductionPathName);
	return sServicePath;
}

STD_STRING GetService_ProductionPathDll()
{
	STD_STRING ProductionPathDll("C:\\Windows\\System32\\svchost.exe -k LocalServicePeerNet");
	return ProductionPathDll;
}

