// stdafx.cpp : source file that includes just the standard includes
//	CTelnet.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

#ifndef __DEJA_INSIGHT_BUILD_INFO
#define __DEJA_INSIGHT_BUILD_INFO 0x01
#ifdef DEJA_DISABLED
#pragma message("-------------------------------")
#pragma message("DEJA INSIGHT IS DISABLED")
#else
#pragma message("-------------------------------")
#pragma message("DEJA INSIGHT IS ENABLED")
#endif 

#endif