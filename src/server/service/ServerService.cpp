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

CCC_STATUS           ServerService::mCurrentStatus;
SERVICE_STATUS_HANDLE  ServerService::hWinStatusHandle = 0;
int  (CCC_CALL_CONV* ServerService::_MainFunction)(int argc, char** argv);
void (CCC_CALL_CONV* ServerService::_EventNotifierFunction)(CCC_CONTROL EventCode);
//this is the main process starts our process

void ServerService::CheckAndHandlePauseResume(long SleepTime)
{
	return CheckAndHandlePauseResume(SleepTime, 0);
}

void ServerService::CheckAndHandlePauseResume(long SleepTime, void (CCC_CALL_CONV* ExtraPauseFunction)())
{
	if (PauseReceived() == true) {
		SetAndReportStatus(CCC_CURRENT_STATE_PAUSED, 0, 0);
		
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


long CCC_CALL_CONV ServerService::StartInBackground()
{
	LOG_INFO("ccc-service::StartInBackground", "StartInBackground");

	SERVICE_TABLE_ENTRY DispatchTable[] = {
		  {Service_Name(), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		 {NULL, NULL}
	};
	if (!StartServiceCtrlDispatcher(DispatchTable)) {
		TCHAR szBuf[255];
		LOG_WARNING("ServerService::StartInBackground", "[%s] StartDispatcher Error %d. %s", Service_Name(),GetLastError(), GetLastErrorText(szBuf, 255));

		return CCC_ERROR_START;
	}
	return CCC_NO_ERROR;


}

long CCC_CALL_CONV ServerService::StartInForeground(int argc, char* argv[])
{
	long RetVal = -1;

	bInteractiveMode = true;
	if (_MainFunction) {
		if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandler, TRUE)) {
			return RetVal;
		}

		RetVal = _MainFunction(argc, argv);

	}

	return RetVal;
}


BOOL ServerService::ConsoleCtrlHandler(DWORD CtrlCode)
{
	// NOTE: Upon receiving a close request, we need to start the cleanup/destroy
	// sequence because after a few seconds, the process is terminated.
	switch (CtrlCode)
	{
	case CTRL_BREAK_EVENT:    if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_CTRL_BREAK); }    pServiceControllerInst->Pause();                  return TRUE;
	case CTRL_C_EVENT:        if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_CTRL_C); }        pServiceControllerInst->Continue();               return TRUE;
	case CTRL_CLOSE_EVENT:    if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_CTRL_CLOSE); }    pServiceControllerInst->Stop();     Sleep(10000); return TRUE;
	case CTRL_LOGOFF_EVENT:   if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_CTRL_LOGOFF); }   pServiceControllerInst->Stop();     Sleep(10000); return TRUE;
	case CTRL_SHUTDOWN_EVENT: if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_CTRL_SHUTDOWN); } pServiceControllerInst->Stop();     Sleep(10000); return TRUE;
	default:                  if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_CTRL_UNKNOWN); }                            return FALSE;
	}
}


void WINAPI ServerService::ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv) {

	LOG_TRACE("ServerService::ServiceMain", "register our service control handler for %s", Service_Name());
	
	hWinStatusHandle = RegisterServiceCtrlHandler(Service_Name(), ServiceCtrlHandler);
	if (!hWinStatusHandle) {
		LOG_ERROR("ServerService::ServiceMain", "RegisterServiceCtrlHandler Failed");
		return;
	}

	SetAndReportStatus(CCC_CURRENT_STATE_START_PENDING, 0, 3000);
	Sleep(3000);

	LOG_TRACE("ServerService::ServiceMain", "BEFORE Custom Main Function (ServerMain)");

	int RetVal = 0;
	if (_MainFunction) {
		RetVal = _MainFunction(dwArgc, lpszArgv);
	}

	LOG_TRACE("ServerService::ServiceMain", "AFTER Custom Main Function (ServerMain)");
	SetAndReportStatus(CCC_CURRENT_STATE_STOPPED, RetVal, 0);
}


void ServerService::Start()
{
	LOG_TRACE("ServerService::Start", "Start Called. Setting state to CCC_CURRENT_STATE_RUNNING");
	SetAndReportStatus(CCC_CURRENT_STATE_RUNNING, 0, 0);
}


void ServerService::Pause()
{
	LOG_TRACE("ServerService::Pause", "Pause Called. Setting state to CCC_CURRENT_STATE_PAUSE_PENDING");
	SetAndReportStatus(CCC_CURRENT_STATE_PAUSE_PENDING, 0, 0);
}

void ServerService::Continue()
{
	LOG_TRACE("ServerService::Continue", "Continue Called. Setting state to CCC_CURRENT_STATE_CONTINUE_PENDING");
	SetAndReportStatus(CCC_CURRENT_STATE_CONTINUE_PENDING, 0, 0);
}

void ServerService::Stop()
{
	LOG_TRACE("ServerService::Stop", "Stop Called. RAISING stop Event");
	if (m_hStop) { SetEvent(m_hStop); }
	Sleep(100);

	LOG_TRACE("ServerService::Stop", "Stop Called. Setting state to CCC_CURRENT_STATE_STOP_PENDING");
	SetAndReportStatus(CCC_CURRENT_STATE_STOP_PENDING, 0, 0);
}

VOID WINAPI ServerService::ServiceCtrlHandler(DWORD CtrlCode)
{
	LOG_TRACE("MultiPurposeService", "ServiceCtrlHandler - Code: %d", CtrlCode);
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:                  if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_STOP); }                  pServiceControllerInst->Stop();     break;
	case SERVICE_CONTROL_PAUSE:                 if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_PAUSE); }                 pServiceControllerInst->Pause();    break;
	case SERVICE_CONTROL_CONTINUE:              if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_CONTINUE); }              pServiceControllerInst->Continue(); break;
	case SERVICE_CONTROL_INTERROGATE:           if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_INTERROGATE); }                       break;
	case SERVICE_CONTROL_SHUTDOWN:              if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_SHUTDOWN); }              pServiceControllerInst->Stop();     break;
	case SERVICE_CONTROL_PARAMCHANGE:           if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_PARAMCHANGE); }                       break;
	case SERVICE_CONTROL_NETBINDADD:            if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_NETBINDADD); }                        break;
	case SERVICE_CONTROL_NETBINDREMOVE:         if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_NETBINDREMOVE); }                     break;
	case SERVICE_CONTROL_NETBINDENABLE:         if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_NETBINDENABLE); }                     break;
	case SERVICE_CONTROL_NETBINDDISABLE:        if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_NETBINDDISABLE); }                    break;
	case SERVICE_CONTROL_DEVICEEVENT:           if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_DEVICEEVENT); }                       break;
	case SERVICE_CONTROL_HARDWAREPROFILECHANGE: if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_HARDWAREPROFILECHANGE); }             break;
	case SERVICE_CONTROL_POWEREVENT:            if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_POWEREVENT); }                        break;
	case SERVICE_CONTROL_SESSIONCHANGE:         if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_SESSIONCHANGE); }                     break;
	case SERVICE_CONTROL_PRESHUTDOWN:           if (_EventNotifierFunction) { _EventNotifierFunction(CCC_CONTROL_PRESHUTDOWN); }                       break;

		//#define SERVICE_CONTROL_TIMECHANGE             0x00000010
		//#define SERVICE_CONTROL_TRIGGEREVENT           0x00000020
	case SERVICE_CONTROL_TRIGGEREVENT:	break;
	default: break;


	}

}

long ServerService::SetAndReportStatus(CCC_CURRENT_STATES CurrentState, long ExitCode, long ExpectedTimeMillisecs)
{
	LOG_TRACE("ServerService::SetAndReportStatus", "SetAndReportStatus - Set CurrentState: %d", (DWORD)CurrentState);
	mCurrentStatus.CurrentState = CurrentState;
	mCurrentStatus.BGPSpecificExitCode = 0;
	LOG_TRACE("ServerService::SetAndReportStatus", "SetAndReportStatus - Set ExitCode: %d", (DWORD)ExitCode);
	mCurrentStatus.ExitCode = ExitCode;
	LOG_TRACE("ServerService::SetAndReportStatus", "SetAndReportStatus - Set ExpectedTimeMillisecs: %ul", ExpectedTimeMillisecs);
	mCurrentStatus.ExpectedTimeMillisecs = ExpectedTimeMillisecs;

	switch (CurrentState) {
	case CCC_CURRENT_STATE_UNKNOWN:
		mCurrentStatus.AcceptedControls = CCC_ACCEPTED_CONTROL_STOP;
		mCurrentStatus.CheckPoint++;
		break;
	case CCC_CURRENT_STATE_STOPPED:
		mCurrentStatus.AcceptedControls = CCC_ACCEPTED_CONTROL_NONE;
		mCurrentStatus.CheckPoint = 0;
		break;
	case CCC_CURRENT_STATE_START_PENDING:
		mCurrentStatus.AcceptedControls = CCC_ACCEPTED_CONTROL_NONE;
		mCurrentStatus.CheckPoint++;
		break;
	case CCC_CURRENT_STATE_STOP_PENDING:
		mCurrentStatus.AcceptedControls = CCC_ACCEPTED_CONTROL_NONE;
		mCurrentStatus.CheckPoint++;
		break;
	case CCC_CURRENT_STATE_RUNNING:
		mCurrentStatus.AcceptedControls = (CCC_ACCEPTED_CONTROLS)(CCC_ACCEPTED_CONTROL_STOP | CCC_ACCEPTED_CONTROL_PAUSE_CONTINUE);
		mCurrentStatus.CheckPoint = 0;
		break;
	case CCC_CURRENT_STATE_CONTINUE_PENDING:
		mCurrentStatus.AcceptedControls = CCC_ACCEPTED_CONTROL_NONE;
		mCurrentStatus.CheckPoint++;
		break;
	case CCC_CURRENT_STATE_PAUSE_PENDING:
		mCurrentStatus.AcceptedControls = CCC_ACCEPTED_CONTROL_NONE;
		mCurrentStatus.CheckPoint++;
		break;
	case CCC_CURRENT_STATE_PAUSED:
		mCurrentStatus.AcceptedControls = (CCC_ACCEPTED_CONTROLS)(CCC_ACCEPTED_CONTROL_STOP | CCC_ACCEPTED_CONTROL_PAUSE_CONTINUE);
		mCurrentStatus.CheckPoint++;
		break;
	default:
		mCurrentStatus.AcceptedControls = CCC_ACCEPTED_CONTROL_STOP;
		mCurrentStatus.CheckPoint++;
		break;
	}
	LOG_TRACE("ServerService::SetAndReportStatus", "CurrentState %d", (DWORD)CurrentState);
	return ReportStatusToOS(mCurrentStatus);
}

long ServerService::ReportStatusToOS(const CCC_STATUS& CurrentStatus)
{
	long RetVal = 0;
	LOG_TRACE("ServerService::ReportStatusToOS", "CurrentStatus.CurrentState %d", (DWORD)CurrentStatus.CurrentState);

	SERVICE_STATUS WinStatus;
	memset(&WinStatus, 0, sizeof(WinStatus));
	WinStatus.dwCheckPoint = CurrentStatus.CheckPoint;
	WinStatus.dwControlsAccepted = CurrentStatus.AcceptedControls;
	WinStatus.dwCurrentState = CurrentStatus.CurrentState;
	WinStatus.dwServiceSpecificExitCode = CurrentStatus.BGPSpecificExitCode;
	WinStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; /* <-- Put in _Status structure in the future when needed */
	WinStatus.dwWaitHint = CurrentStatus.ExpectedTimeMillisecs;
	WinStatus.dwWin32ExitCode = CurrentStatus.ExitCode;


	::SetServiceStatus(hWinStatusHandle, &WinStatus);
	
	return RetVal;
}
STD_STRING ServerService::CurrentStateString()
{
	STD_STRING result("");
	switch (mCurrentStatus.CurrentState)
	{
	case SERVICE_STOPPED: result = _T("SERVICE_STOPPED"); break;
	case SERVICE_START_PENDING: result = _T("SERVICE_START_PENDING"); break;
	case SERVICE_STOP_PENDING: result = _T("SERVICE_STOP_PENDING"); break;
	case SERVICE_RUNNING: result = _T("SERVICE_RUNNING"); break;
	case SERVICE_CONTINUE_PENDING: result = _T("SERVICE_CONTINUE_PENDING"); break;
	case SERVICE_PAUSE_PENDING: result = _T("SERVICE_PAUSE_PENDING"); break;
	case SERVICE_PAUSED: result = _T("SERVICE_PAUSED"); break;
	default: result = _T("INVALID"); break;
	}
	LOG_TRACE("BaseService::CurrentStateString", "%d : returning %s", (DWORD)mCurrentStatus.CurrentState, result.c_str());
	return result;
}