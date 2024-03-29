//==============================================================================
//
//   main.cpp
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#ifdef _SERVICE_DLL

#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"
#include <stdlib.h>
#include <windows.h>
#include <process.h>
#include <winsock2.h>
#include <stdio.h>


#include "cmdline.h"
#include "ServerService.h"
#include "utilities.h"
#include "windows-api-ex.h"
#define NTSTATUS LONG

typedef NTSTATUS(WINAPI* LPSTART_RPC_SERVER) (RPC_WSTR, RPC_IF_HANDLE);
typedef NTSTATUS(WINAPI* LPSTOP_RPC_SERVER) (RPC_IF_HANDLE);
typedef NTSTATUS(WINAPI* LPSTOP_RPC_SERVER_EX) (RPC_IF_HANDLE);

typedef VOID(WINAPI* LPNET_BIOS_OPEN) (VOID);
typedef VOID(WINAPI* LPNET_BIOS_CLOSE) (VOID);
typedef DWORD(WINAPI* LPNET_BIOS_RESET) (unsigned char);

typedef DWORD(WINAPI* LPREGISTER_STOP_CALLBACK) (HANDLE*, PCWSTR, HANDLE, WAITORTIMERCALLBACK, PVOID, DWORD);
typedef struct _SVCHOST_GLOBAL_DATA {
	PSID NullSid;                               // S-1-0-0
	PSID WorldSid;                              // S-1-1-0
	PSID LocalSid;                              // S-1-2-0
	PSID NetworkSid;                            // S-1-5-2
	PSID LocalSystemSid;                        // S-1-5-18
	PSID LocalServiceSid;                       // S-1-5-19
	PSID NetworkServiceSid;                     // S-1-5-20
	PSID BuiltinDomainSid;                      // S-1-5-32
	PSID AuthenticatedUserSid;                  // S-1-5-11
	PSID AnonymousLogonSid;                     // S-1-5-7
	PSID AliasAdminsSid;                        // S-1-5-32-544
	PSID AliasUsersSid;                         // S-1-5-32-545
	PSID AliasGuestsSid;                        // S-1-5-32-546
	PSID AliasPowerUsersSid;                    // S-1-5-32-547
	PSID AliasAccountOpsSid;                    // S-1-5-32-548
	PSID AliasSystemOpsSid;                     // S-1-5-32-549
	PSID AliasPrintOpsSid;                      // S-1-5-32-550
	PSID AliasBackupOpsSid;                     // S-1-5-32-551
	LPSTART_RPC_SERVER StartRpcServer;
	LPSTOP_RPC_SERVER StopRpcServer;
	LPSTOP_RPC_SERVER_EX StopRpcServerEx;
	LPNET_BIOS_OPEN NetBiosOpen;
	LPNET_BIOS_CLOSE NetBiosClose;
	LPNET_BIOS_RESET NetBiosReset;

} SVCHOST_GLOBAL_DATA;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma message( "Compiling " __FILE__ )
#pragma message( "Last modified on " __TIMESTAMP__ )

	
std::unique_ptr<ServerService> pServiceControllerInst;

static int  RECON_CALL_CONV DllRun(int argc, char* argv[]);
static void RECON_CALL_CONV DllEventHandler(RECON_CONTROL EventCode);


BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{

	return true;
}

// NOTE Service Main calls StartDll which starts a thread to execute ServiceExecute. In normal circomstances, ServiceMain must return.
// Maybe, we dont want to log event i this case, but we eiwll block events in another way.
// 
// In version 6.0, SVCHOST logs each call to ServiceMain as an event, such that it may be retrieved through the Event Viewer in the Diagnostic channel 
// under the heading Microsoft Windows Services Svchost Performance Diagnostic Provider.Each call is preceded by event 101 and each return is marked 
// as event 102. For both, the event data is the service name.
// This event log provides ready confirmation that many services implemented in service DLLs do not return from ServiceMain until stopped.
// https://www.geoffchappell.com/studies/windows/win32/services/svchost/dll/servicemain.htm

extern "C" __declspec(dllexport) VOID WINAPI ServiceMain(DWORD argc, LPCWSTR * argv)
{
	char** argn = (char**)C::Convert::StringToString(*argv);

	// unique ptr to avoid leak on exit
	pServiceControllerInst = std::unique_ptr<ServerService>(new ServerService(DllRun, DllEventHandler)); 
	
	LOG_TRACE("DLL::ServiceMain", "ServiceMain will Create Thread");
	pServiceControllerInst->StartDll(argc, argn);
	
	Sleep(1000);

	LOG_WARNING("DLL::ServiceMain", "ServiceMain exits...");
}

// SVCHOST provides service DLLs with access to shared codeand data.To learn the addresses, a service DLL should export 
// a function named SvchostPushServiceGlobals, which SVCHOST calls before each call to any ServiceMain function in the DLL.

extern "C" __declspec(dllexport) VOID WINAPI SvchostPushServiceGlobals(SVCHOST_GLOBAL_DATA * lpGlobalData)
{
	LOG_WARNING("DLL::SvchostPushServiceGlobals", "SvchostPushServiceGlobals");
	Sleep(1000);
}


static int RECON_CALL_CONV DllRun(int argc, char* argv[])
{
	LOG_TRACE("DLL::DllRun", "ServerMain Entry Point");

	pServiceControllerInst->Start();

	while (pServiceControllerInst && pServiceControllerInst->StopReceived() == false)
	{
		LOG_TRACE("DLL::DllRun", "LOOP");
		pServiceControllerInst->CheckAndHandlePauseResume(1000, 0);
		Sleep(1000);
		pServiceControllerInst->Step();
	}

	LOG_WARNING("DLL::DllRun", "EXITING...");

	return 0;
}

void DllEventHandler(RECON_CONTROL EventCode)
{
	LOG_TRACE("DllEventHandler", "Received Service Control Event: %d", EventCode);

	switch (EventCode)
	{
	case RECON_CONTROL_UNKNOWN:               break;
	case RECON_CONTROL_STOP:                  
		LOG_TRACE("DLL::DllEventHandler", "RECON_CONTROL_STOP");
		break;
	case RECON_CONTROL_PAUSE:                 break;
	case RECON_CONTROL_CONTINUE:              break;
	case RECON_CONTROL_INTERROGATE:           break;
	case RECON_CONTROL_SHUTDOWN:              break;
	case RECON_CONTROL_PARAMCHANGE:           break;
	case RECON_CONTROL_NETBINDADD:            break;
	case RECON_CONTROL_NETBINDREMOVE:         break;
	case RECON_CONTROL_NETBINDENABLE:         break;
	case RECON_CONTROL_NETBINDDISABLE:        break;
	case RECON_CONTROL_DEVICEEVENT:           break;
	case RECON_CONTROL_HARDWAREPROFILECHANGE: break;
	case RECON_CONTROL_POWEREVENT:            break;
	case RECON_CONTROL_SESSIONCHANGE:         break;
	case RECON_CONTROL_PRESHUTDOWN:           break;
	case RECON_CONTROL_CTRL_BREAK:            break;
	case RECON_CONTROL_CTRL_C:                
		LOG_TRACE("DLL::DllEventHandler", "CTRL-C: Gracefully terminating service...");
		if (pServiceControllerInst->IsInteractive() == true) {
			cprint_c("CTRL-C: Gracefully terminating service...\n");
		}
		if (pServiceControllerInst) {
			pServiceControllerInst->Stop();
		}
		
		// Signal is handled - don't pass it on to the next handler
		Sleep(1000);
		break;
	case RECON_CONTROL_CTRL_CLOSE:            break;
	case RECON_CONTROL_CTRL_LOGOFF:           break;
	case RECON_CONTROL_CTRL_SHUTDOWN:         break;
	case RECON_CONTROL_CTRL_UNKNOWN:          break;
	default:
		break;
	}
}

#endif