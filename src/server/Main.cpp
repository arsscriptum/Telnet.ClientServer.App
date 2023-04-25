
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


#include "cmdline.h"
#include "ServerService.h"


#pragma NOTE("LINKING WITH wsock32.lib")
#pragma NOTE("LINKING WITH advapi32.lib")
#pragma comment( lib, "wsock32" )
#pragma comment( lib, "advapi32" ) 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma message( "Compiling " __FILE__ )
#pragma message( "Last modified on " __TIMESTAMP__ )

ServerService *pServiceControllerInst = nullptr;

static void CCC_CALL_CONV ExtraPauseFunction();
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType);	
void banner() {
	std::wcout << std::endl;
	PRINT_C("recon_srv v2.1 - Remote Shell Service\n");
	PRINT_C("Built on %s\n", __TIMESTAMP__);
	PRINT_C("Copyright (C) 2000-2021 Guillaume Plante\n");
	std::wcout << std::endl;
}
void usage() {
	PRINT_C("Usage: recon_srv.exe [-h][-p][-a][-n][-s]\n");
	PRINT_C("   -p          Print process executable pats\n");
	PRINT_C("   -v          Verbose Show All process even access denied\n");
	PRINT_C("   -h          Help\n");
	PRINT_C("   -n          No banner\n");
	PRINT_C("   -i          Install Service\n");
	PRINT_C("   -u          Uninstall Service\n");
	PRINT_C("   -d          Debug Service\n");
	PRINT_C("   -s          Stop Service\n");
	std::wcout << std::endl;
}



//----------------------------------------------------------------



int main( int argc, char ** argv )
{
	DEJA_APP_LABEL("RECON SERVER");
	LOG_TRACE("main", "main;");



#ifdef UNICODE
	const char** argn = (const char**)C::Convert::allocate_argn(argc, argv);
#else
	char** argn = argv;
#endif // UNICODE

	CmdLineUtil::getInstance()->initializeCmdlineParser(argc, argn);

	CmdlineParser* inputParser = CmdLineUtil::getInstance()->getInputParser();

	CmdlineOption cmdlineOptionHelp({ "-h", "--help" }, "display this help");
	CmdlineOption cmdlineOptionVerbose({ "-v", "--verbose" }, "verbose output");
	CmdlineOption cmdlineOptionNoBanner({ "-n", "--nobanner" }, "no banner");
	CmdlineOption cmdlineOptionInstall({ "-i", "--install" }, "install service");
	CmdlineOption cmdlineOptionUninstall({ "-u", "--uninstall" }, "uninstall service");
	CmdlineOption cmdlineOptionStop({ "-s", "--stop" }, "stop service");
	CmdlineOption cmdlineOptionDebug({ "-d", "--debug" }, "debug service");

	CmdlineOption cmdlineOptionTest({ "-t", "--test" }, "test");


	inputParser->addOption(cmdlineOptionNoBanner);
	inputParser->addOption(cmdlineOptionHelp);
	inputParser->addOption(cmdlineOptionVerbose);
	inputParser->addOption(cmdlineOptionInstall);
	inputParser->addOption(cmdlineOptionUninstall);
	inputParser->addOption(cmdlineOptionStop);
	inputParser->addOption(cmdlineOptionDebug);
	inputParser->addOption(cmdlineOptionTest);

	bool optNoBanner = inputParser->isSet(cmdlineOptionNoBanner);
	bool optHelp = inputParser->isSet(cmdlineOptionHelp);
	bool optVerbose = inputParser->isSet(cmdlineOptionVerbose);
	bool optServiceInstall = inputParser->isSet(cmdlineOptionInstall);
	bool optServiceUninstall = inputParser->isSet(cmdlineOptionUninstall);
	bool optServiceStop = inputParser->isSet(cmdlineOptionStop);
	bool optServiceDebug = inputParser->isSet(cmdlineOptionDebug);
	bool optTest = inputParser->isSet(cmdlineOptionTest);

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	bool gExitRequested = false;
	if (optHelp) {
		usage();
		return 0;
	}
	if (!optNoBanner) {
		banner();
	}
	ServerService service;
	pServiceControllerInst = &service;

	LOG_TRACE("main", "RegisterService;");
	service.RegisterService();

	BOOL ret = TRUE;

	if (optServiceInstall) {
		LOG_TRACE("main", "InstallService;");
		ret = service.InstallService();
		return (ret ? 0 : -1);
	}
	else if (optServiceUninstall) {
		LOG_TRACE("main", "UninstallService;");
		ret = service.RemoveService();
		return (ret ? 0 : -2);
	}
	else if (optServiceStop) {
		LOG_TRACE("main", "StopService;");
		service.Stop();
		return 0;
	}
	else if (optServiceDebug) {
		LOG_TRACE("main", "DebugService;");
		ret = service.DebugService(argc, argv); 
		return (ret ? 0 : -3);
	}
	else if (optTest) {
		LOG_TRACE("main", "Test;");
	}

	pServiceControllerInst->Start();

	while (pServiceControllerInst->StopReceived() == false)
	{
		LOG_TRACE("Main", "Running...");
		pServiceControllerInst->CheckAndHandlePauseResume(1000, &ExtraPauseFunction);
		Sleep(1000);
		CCC_RETURN_CODES ret = pServiceControllerInst->Step();

		if (gExitRequested) {
			LOG_TRACE("Main", "EmergencyExitRequested");

			return -5;
		}
	}

	LOG_TRACE("Main", "Exiting...");
	Sleep(1000);
	return 0;
}


//==============================================================================
// ExtraPauseFunction
//==============================================================================
// This OPTIONAL function can be used to do some special actions while the   
// program is pausing. It is called repeatedly until the program is resumed  
// again. The time between to consecutive calls is specified with the        
// parameter SleepTime in the CheckAndHandlePauseResume function.  .  
//==============================================================================

static void CCC_CALL_CONV ExtraPauseFunction()
{
	COUTY("pause");
}

BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {
	switch (dwCtrlType)
	{
	case CTRL_BREAK_EVENT:
		LOG_WARNING("HandlerRoutine", "CTRL-BREAK RECEIVED");
		if (pServiceControllerInst->IsDebugging() == true) {
			PRINT_C("BREAK: Emergency stop. Terminating service.\n");
			pServiceControllerInst->Stop();
		}
		LOG_TRACE("HandlerRoutine", "BREAK: Emergency stop. Terminating service.");
		return TRUE;
	case CTRL_C_EVENT:
		LOG_TRACE("HandlerRoutine", "CTRL-C RECEIVED");
		if (pServiceControllerInst->IsDebugging() == true) {
			PRINT_C("CTRL-C: Gracefully terminating service...\n");
		}
		LOG_TRACE("ccc-service::HandlerRoutine", "CTRL-C: Gracefully terminating service...");
		pServiceControllerInst->Stop();
		// Signal is handled - don't pass it on to the next handler
		Sleep(1000);
		return TRUE;
	default:
		// Pass signal on to the next handler
		return FALSE;
	}
}