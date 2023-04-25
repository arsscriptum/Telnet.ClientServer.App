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

	ServerService();
	//the run process
	virtual void Start();

	//the stop process
	void Stop();

	void	Pause();
	void	Continue();
	void	Shutdown() {};

	//construction code
	ServerService(int (CCC_CALL_CONV* MainFunction)(int argc, char* argv[])) : BaseService(Service_Name(), Service_Display_Name(), Service_AccountName(), Service_Password())
	{
		ServerService(MainFunction, 0);
	}
	ServerService(int (CCC_CALL_CONV* MainFunction)(int argc, char* argv[]), void (CCC_CALL_CONV* EventNotifierFunction)(CCC_CONTROL EventCode))  : BaseService(Service_Name(), Service_Display_Name(), Service_AccountName(), Service_Password())
	{
		_MainFunction = MainFunction;
		_EventNotifierFunction = EventNotifierFunction;
		memset(&mCurrentStatus, 0, sizeof(mCurrentStatus));
		mCurrentStatus.CheckPoint = 1;
		hWinStatusHandle = 0;
	}
	virtual STD_STRING ServerService::CurrentStateString();
	virtual bool IsRunning() { return (mCurrentStatus.CurrentState == SERVICE_RUNNING); }
	virtual bool StopReceived() { return (mCurrentStatus.CurrentState == SERVICE_STOP_PENDING); }
	virtual bool PauseReceived() { return (mCurrentStatus.CurrentState == SERVICE_PAUSE_PENDING); }
	virtual bool ContinueReceived() { return (mCurrentStatus.CurrentState == SERVICE_CONTINUE_PENDING); }
	virtual bool IsInteractive() { return (IsRunning() && m_bInteractiveMode); };
	//==============================================================================
//  Starting and running the service
//==============================================================================
	CCC_INTERFACE_DEF(long)  StartInBackground();
	CCC_INTERFACE_DEF(long)  StartInForeground(int argc, char* argv[]);
	CCC_INTERFACE_DEF(bool)  IsStartedInForeground() { return bInteractiveMode; }
	CCC_INTERFACE_DEF(bool)  IsStartedInBackground() { return !bInteractiveMode; }
	CCC_INTERFACE_DEF(CCC_RETURN_CODES)   Step();
	CCC_INTERFACE_DEF(void)  CheckAndHandlePauseResume(long SleepTime);
	CCC_INTERFACE_DEF(void)  CheckAndHandlePauseResume(long SleepTime, void (CCC_CALL_CONV* ExtraPauseFunction)());
	//==============================================================================
	// Additional Control&Reporting
	//==============================================================================
	static long SetAndReportStatus(CCC_CURRENT_STATES CurrentState, long ExitCode, long ExpectedTimeMillisecs);
	static void ServiceReportEvent(CCC_LOG_TYPES LogType, const TCHAR* ProcessName, TCHAR* Message);
	static long ReportStatusToOS(const CCC_STATUS& CurrentStatus);

	static BOOL        ConsoleCtrlHandler(DWORD CtrlCode);

	static VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode);
	static void WINAPI	ServiceMain(DWORD argc, LPTSTR* argv);
	static BOOL WINAPI	ControlHandler(DWORD CtrlType);
	static int  (CCC_CALL_CONV* _MainFunction)(int argc, char* argv[]);				// Main
	static void (CCC_CALL_CONV* _EventNotifierFunction)(CCC_CONTROL EventCode);		// Event Notifier CB
	static CCC_STATUS                mCurrentStatus;
	static SERVICE_STATUS_HANDLE	 hWinStatusHandle;
	BOOL bInteractiveMode;
};

#endif	// __SERVER_SERVICE_H__
