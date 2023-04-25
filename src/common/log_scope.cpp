
//==============================================================================
//
//   ScopedLogger.cpp
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#include "stdafx.h"
#include "log_scope.h"

ScoppedLogger::ScoppedLogger(const char* pId)
{
	strncpy(sId,pId,SCOPPEDLOGGER_ID_LEN-1);
	DEJA_TRACE("ScoppedLogger","ENTERING %s", sId);
}
 
ScoppedLogger::~ScoppedLogger()
{
	DEJA_TRACE("ScoppedLogger","LEAVING %s", sId);
}
 
