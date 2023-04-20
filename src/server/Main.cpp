
//==============================================================================
//
//   main.cpp
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================



#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <winsock2.h>
#include <stdio.h>

#pragma comment( lib, "wsock32" )
#pragma comment( lib, "advapi32" ) 

#include "NTService.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//the DoProcess function that reads config data
//and runs scripts
extern void DoProcess(void);


//----------------------------------------------------------------
//our service wrapper class.
//performs all tasks required to be
//an NT service
//----------------------------------------------------------------
class SMRemoteService : public CNTService
{
private:
	//the handle used as a synchronization mechanism
	HANDLE m_hStop;

public:
	//the run process
	void Run(DWORD argc, LPTSTR * argv);

	//the stop process
	void Stop();

	//construction code
	SMRemoteService() : CNTService(TEXT("__dev_telnet_srv"), 
		TEXT("__dev_telnet_srv")) , m_hStop(0){};
};

UINT thrid_daemon;
extern unsigned __stdcall Daemon(void*);
unsigned long masterThread;

extern int Invoke_ShellServer_Networking(void);
extern void CycleNoLogin(void);
extern void Cycle(void); 

long restartCount;
volatile BOOL requestReset;
extern void MasterReset(void);

//this is the main process starts our process
void SMRemoteService::Run(DWORD argc, LPTSTR * argv) 
{
	LOG_TRACE("SMRemoteService::Run", "Run");
	restartCount = 0;
	requestReset = FALSE;
	LOG_TRACE("SMRemoteService::Run", "Run ReportStatus(SERVICE_START_PENDING);");
	ReportStatus(SERVICE_START_PENDING);
	m_hStop = CreateEvent(0, TRUE, FALSE, 0);
	LOG_TRACE("SMRemoteService::Run", "Run ReportStatus(SERVICE_RUNNING);");
	ReportStatus(SERVICE_RUNNING);

	//start the deamon thread
	masterThread = _beginthreadex(NULL,0,Daemon,NULL,0,&thrid_daemon);

	SetThreadPriority((void*)masterThread, THREAD_PRIORITY_NORMAL);

	WaitForSingleObjectEx(m_hStop,INFINITE,FALSE);

	LOG_TRACE("SMRemoteService::Run", "TerminateThread;");
	TerminateThread((void*)masterThread,0);
	CloseHandle(m_hStop);

	LOG_TRACE("SMRemoteService::Run", "Run ReportStatus(SERVICE_STOPPED);");
	ReportStatus(SERVICE_STOPPED);
}


//signal the main process (Run) to terminate
void SMRemoteService::Stop() 
{
	LOG_TRACE("SMRemoteService::Stop", "ReportStatus(SERVICE_STOP_PENDING);");
	if( m_hStop )
	{
		SetEvent(m_hStop);
	}
	ReportStatus(SERVICE_STOP_PENDING);
}

//----------------------------------------------------------------



//----------------------------------------------------------------
//command line options
//----------------------------------------------------------------
//
//		-i			install the service (calls
//					"InstallService()" - see below)
//
//			-l <account>
//					<account> is the name of a user,
//					under which the service shall run.
//					This option is useful with -i only.
//					<account> needs to have the advanced
//					user-right "Log on as a service"
//					(see User-Manager)
//					<account> should have the following
//					format: "<Domain>\<user>"
//					"EuroS2Team\jko" for instance.
//					The domain "." is predefined as the
//					local machine. So one might use
//					".\jko" too.
//
//			-p <password>
//					The password of the user, under which
//					the service shall run. Only useful
//					with -i and -l together.
//
//		-u			uninstall the service (calls
//					"RemoveService()" - see below)
//
//		-d			debug the service (run as console
//					process; calls "DebugService()"
//					see below)
//
//		-e			end the service (if it is running)
//
//		-s			start the service (if it is not running)
//					(Note that *you* normally cannot start
//					an NT-service from the command-line.
//					The SCM can.)
//----------------------------------------------------------------

int main( int argc, char ** argv )
{
	LOG_TRACE("main", "RegisterService;");
	SMRemoteService service;
	return service.RegisterService(argc, argv);
}
