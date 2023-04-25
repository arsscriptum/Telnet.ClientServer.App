//==============================================================================
//
//   ServerService.cpp
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#include "stdafx.h"
#include "config.h"
#include "ServerService.h"

HANDLE ghRunThreadStopEvent = 0;
extern ServerService* pServiceControllerInst;

unsigned  __stdcall ThreadProc(LPVOID lpParam);

//this is the main process starts our process
void ServerService::Start()
{
	LOG_TRACE("ServerService::Run", "Run");
	restartCount = 0;
	requestReset = FALSE;
	LOG_TRACE("ServerService::Run", "ReportStatus(SERVICE_START_PENDING);");
	ReportStatus(SERVICE_START_PENDING);
	LOG_TRACE("ServerService::Run", "CreateEvent m_hStop");
	m_hStop = CreateEvent(0, TRUE, FALSE, 0);
	LOG_TRACE("ServerService::Run", "ReportStatus(SERVICE_RUNNING);");
	ReportStatus(SERVICE_RUNNING);

	//start the deamon thread
	masterThread = _beginthreadex(NULL, 0, ThreadProc, NULL, 0, &thrid_daemon);

	SetThreadPriority((void*)masterThread, THREAD_PRIORITY_NORMAL);

	LOG_TRACE("ServerService::Run", "Event m_hStop Received");

	WaitForSingleObjectEx(m_hStop, INFINITE, FALSE);

	LOG_TRACE("ServerService::Run", "TerminateThread;");
	TerminateThread((void*)masterThread, 0);
	CloseHandle(m_hStop);

	LOG_TRACE("ServerService::Run", "Run ReportStatus(SERVICE_STOPPED);");
	ReportStatus(SERVICE_STOPPED);
}



//signal the main process (Run) to terminate
void ServerService::Stop()
{
	LOG_TRACE("ServerService::Stop", "ReportStatus(SERVICE_STOP_PENDING);");
	if (ghRunThreadStopEvent) { 
		LOG_TRACE("ServerService::Stop", "SendEvent RunThreadStopEvent"); 
		SetEvent(ghRunThreadStopEvent); 
	}
	Sleep(200);
	if (m_hStop){ SetEvent(m_hStop); }
	Sleep(200);
	LOG_TRACE("ServerService::Stop", "ReportStatus(SERVICE_STOP_PENDING);");
	ReportStatus(SERVICE_STOP_PENDING);
}

unsigned  __stdcall ThreadProc(LPVOID lpParam)
{
	LOG_TRACE("ThreadProc", "Main Run Loop");

	ghRunThreadStopEvent = CreateEvent(0, TRUE, FALSE, 0);

	while (pServiceControllerInst->StopReceived() == false)
	{
		LOG_TRACE("ThreadProc", "Running...");
		DWORD ret = WaitForSingleObjectEx(ghRunThreadStopEvent, 1000, FALSE);
		if (ret == WAIT_OBJECT_0) {
			LOG_TRACE("ThreadProc", "Stop Signalled...");
			break;
		}
	}

	LOG_TRACE("ThreadProc", "Exiting...");

	return 0;
}

void ServerService::CheckAndHandlePauseResume(long SleepTime)
{
	return CheckAndHandlePauseResume(SleepTime, 0);
}

void ServerService::CheckAndHandlePauseResume(long SleepTime, void (CCC_CALL_CONV* ExtraPauseFunction)())
{
	if (PauseReceived() == true) {
		ReportStatus(SERVICE_PAUSED);
		
		while (StopReceived() == false && ContinueReceived() == false) {
			/* Pausing */
			if (ExtraPauseFunction) { ExtraPauseFunction(); }
			Sleep(SleepTime);
		}
		/* Running again */
		if (StopReceived() == false) {
			Start();
		}
	}
}

CCC_RETURN_CODES CCC_CALL_CONV ServerService::Step()
{
	LOG_PROFILE("ServerService::Step");
	LOG_TRACE("ServerService::Step", "Running");

#ifdef DEBUGGER_DETECTION
	bool debuggerDetected = mDebuggerSpy.CheckDebugger();
	if (debuggerDetected) {
		LOG_TRACE("ccc-service::Step", "Debugger Connection Detected! RUN!!!");
		return CCC_RETURN_CODE_DEBUGGER_DETECTED;
	}
#endif //DEBUGGER_DETECTION
	return CCC_RETURN_CODE_SUCCESS;
}