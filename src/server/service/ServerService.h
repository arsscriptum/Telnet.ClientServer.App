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
#include "cthread.h"
extern HANDLE ghRunThreadStopEvent;

class ServerService : public BaseService, public CThread
{

public:


	//the run process
	virtual void Start();

	//the stop process
	void Stop();

	void	Pause();
	void	Continue();
	void	Shutdown() {};
	
	//construction code
	ServerService(int (RECON_CALL_CONV* MainFunction)(int argc, char* argv[])) : BaseService(Service_Name(), Service_Display_Name(), Service_AccountName(), Service_Password())
	{
		ServerService(MainFunction, 0);
	}
	ServerService(int (RECON_CALL_CONV* MainFunction)(int argc, char* argv[]), void (RECON_CALL_CONV* EventNotifierFunction)(RECON_CONTROL EventCode))  : BaseService(Service_Name(), Service_Display_Name(), Service_AccountName(), Service_Password())
	{
		_MainFunction = MainFunction;
		_EventNotifierFunction = EventNotifierFunction;
		memset(&m_CurrentStatus, 0, sizeof(m_CurrentStatus));
		m_CurrentStatus.CheckPoint = 1;
		m_hWinStatusHandle = 0;
		mArgv = nullptr;
		mArgc = 0;
	}
	virtual STD_STRING ServerService::CurrentStateString();
	virtual bool IsRunning() { return (m_CurrentStatus.CurrentState == SERVICE_RUNNING); }
	virtual bool StopReceived() { return (m_CurrentStatus.CurrentState == SERVICE_STOP_PENDING); }
	virtual bool IsStopped() { return (m_CurrentStatus.CurrentState == SERVICE_STOPPED); }
	virtual bool PauseReceived() { return (m_CurrentStatus.CurrentState == SERVICE_PAUSE_PENDING); }
	virtual bool ContinueReceived() { return (m_CurrentStatus.CurrentState == SERVICE_CONTINUE_PENDING); }
	virtual bool IsInteractive() { return (IsRunning() && m_bInteractiveMode); };

	//==============================================================================
	//  Starting and running the service
	//==============================================================================
	long  StartInBackground();
	long  StartInForeground(int argc, char* argv[]);
	long  StartDll(int argc, char* argv[]);
	bool  IsStartedInForeground() { return bInteractiveMode; }
	bool  IsStartedInBackground() { return !bInteractiveMode; }
	RECON_RETURN_CODES   Step();
	void  CheckAndHandlePauseResume(long SleepTime);
	void  CheckAndHandlePauseResume(long SleepTime, void (RECON_CALL_CONV* ExtraPauseFunction)());

protected:
	virtual unsigned long Process(void* parameter);
private:
	ServerService();
	//==============================================================================
	// Additional Control&Reporting
	//==============================================================================
	static long WINAPI SetAndReportStatus(RECON_CURRENT_STATES CurrentState, long ExitCode, long ExpectedTimeMillisecs);
	static long WINAPI ReportStatusToOS(const RECON_STATUS& CurrentStatus);

	static BOOL WINAPI ConsoleCtrlHandler(DWORD CtrlCode);

	static VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode);
	static void WINAPI ServiceExecute(DWORD argc, LPTSTR* argv);

	static int (RECON_CALL_CONV* _MainFunction)(int argc, char* argv[]);
	static void (RECON_CALL_CONV* _EventNotifierFunction)(RECON_CONTROL EventCode);		
	static RECON_STATUS                m_CurrentStatus;
	static SERVICE_STATUS_HANDLE	 m_hWinStatusHandle;
	BOOL bInteractiveMode;
	int mArgc;
	char** mArgv;
};

#endif	// __SERVER_SERVICE_H__
