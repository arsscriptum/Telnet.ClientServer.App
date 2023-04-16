// stdafx.cpp : source file that includes just the standard includes
//	CTelnet.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "ScopedLogger.h"

ScoppedLogger::ScoppedLogger(const char* pId)
{
	strncpy(sId,pId,SCOPPEDLOGGER_ID_LEN-1);
	DEJA_TRACE("ScoppedLogger","ENTERING %s", sId);
}
 
ScoppedLogger::~ScoppedLogger()
{
	DEJA_TRACE("ScoppedLogger","LEAVING %s", sId);
}
 
