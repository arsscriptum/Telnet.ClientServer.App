//==============================================================================
//
//   ServerService.h
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#ifndef __SERVER_SERVICE_H__
#define __SERVER_SERVICE_H__

#include "stdafx.h"
#include "config.h"
#include "BaseService.h"

extern HANDLE ghRunThreadStopEvent;
//----------------------------------------------------------------
//our service wrapper class.
//performs all tasks required to be
//an NT service
//----------------------------------------------------------------
class ServerService : public BaseService
{
private:
	//the handle used as a synchronization mechanism
	HANDLE m_hStop;
	long restartCount;
	volatile BOOL requestReset;
	unsigned long masterThread;
	UINT thrid_daemon;

public:
	//the run process
	void Start();

	//the stop process
	void Stop();

	//construction code
	ServerService() : BaseService(Service_Name(), Service_Display_Name(), Service_AccountName(), Service_Password()) { m_hStop = 0; };

	CCC_INTERFACE_DEF(CCC_RETURN_CODES)   Step();
	CCC_INTERFACE_DEF(void)  CheckAndHandlePauseResume(long SleepTime);
	CCC_INTERFACE_DEF(void)  CheckAndHandlePauseResume(long SleepTime, void (CCC_CALL_CONV* ExtraPauseFunction)());
};

#endif	// __SERVER_SERVICE_H__
