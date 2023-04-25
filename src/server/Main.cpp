
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
#include "utilities.h"

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

static int  CCC_CALL_CONV ServerMain(int argc, char* argv[]);
static void CCC_CALL_CONV ServerEventHandler(CCC_CONTROL EventCode);

void banner() {
	std::wcout << std::endl;
	cprint_cb("recon_srv v2.1 - Remote Shell Service\n");
	cprint_c("Built on %s\n", __TIMESTAMP__);
	cprint_c("Copyright (C) 2000-2021 Guillaume Plante\n");
	std::wcout << std::endl;
}
void usage() {
	cprint_cb("Usage: recon_srv.exe [-h][-p][-a][-n][-s]\n");
	cprint_c("   -p          Print process executable pats\n");
	cprint_c("   -v          Verbose Show All process even access denied\n");
	cprint_c("   -h          Help\n");
	cprint_c("   -n          No banner\n");
	cprint_c("   -i          Install Service\n");
	cprint_c("   -u          Uninstall Service\n");
	cprint_c("   -d          Debug Service\n");
	cprint_c("   -s          Stop Service\n");
	std::wcout << std::endl;
}



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

	//SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	bool gExitRequested = false;
	if (optHelp) {
		usage();
		return 0;
	}
	if (!optNoBanner) {
		banner();
	}
	
	ServerService service(ServerMain, ServerEventHandler);
	pServiceControllerInst = &service;

	BOOL ret = TRUE;

	if (optServiceInstall) {
		LOG_TRACE("main", "InstallService;");
		
		cprint_r("InstallService\n");
		ret = service.InstallService();
		return (ret ? 0 : -1);
	}
	else if (optServiceUninstall) {
		LOG_TRACE("main", "UninstallService;");
		
		cprint_r("Uninstall Service\n");
		ret = service.RemoveService();
		return (ret ? 0 : -2);
	}
	else if (optServiceStop) {
		LOG_TRACE("main", "StopService;");
		
		cprint_r("Stop Service\n");
		ret = service.EndService();
		return (ret ? 0 : -3);
	}
	else if (optServiceDebug) {
		LOG_TRACE("main", "StartInForeground;");
		
		cprint_r("Debug (start service in foreground)\nCTRL-C to QUIT.");
		ret = service.StartInForeground(argc, argv);
		return (ret ? 0 : -3);
	}
	else if (optTest) {
		LOG_TRACE("main", "Test;");
	
		cprint_r("Test Mode\n");
	}

	LOG_TRACE("Main", "StartInBackground");
	
	cprint_r("Start Service (in background mode)\n");
	ret = service.StartInBackground();
	return (ret ? 0 : -6);

	
	LOG_TRACE("Main", "Exiting in 1 second");
	Sleep(1000);
	return 0;
}


static int CCC_CALL_CONV ServerMain(int argc, char* argv[])
{
	LOG_TRACE("ServerMain", "Entry Point");

	pServiceControllerInst->Start();

	while (pServiceControllerInst->StopReceived() == false)
	{
		pServiceControllerInst->CheckAndHandlePauseResume(1000, 0);
		Sleep(1000);
		pServiceControllerInst->Step();
	}

	LOG_TRACE("ServerMain", "Exiting...");

	return 0;
}

static void CCC_CALL_CONV ServerEventHandler(CCC_CONTROL EventCode)
{
	LOG_INFO("ServerEventHandler", "Received Service Control Event: %d", EventCode);

	switch (EventCode)
	{
	case CCC_CONTROL_UNKNOWN:               break;
	case CCC_CONTROL_STOP:                  break;
	case CCC_CONTROL_PAUSE:                 break;
	case CCC_CONTROL_CONTINUE:              break;
	case CCC_CONTROL_INTERROGATE:           break;
	case CCC_CONTROL_SHUTDOWN:              break;
	case CCC_CONTROL_PARAMCHANGE:           break;
	case CCC_CONTROL_NETBINDADD:            break;
	case CCC_CONTROL_NETBINDREMOVE:         break;
	case CCC_CONTROL_NETBINDENABLE:         break;
	case CCC_CONTROL_NETBINDDISABLE:        break;
	case CCC_CONTROL_DEVICEEVENT:           break;
	case CCC_CONTROL_HARDWAREPROFILECHANGE: break;
	case CCC_CONTROL_POWEREVENT:            break;
	case CCC_CONTROL_SESSIONCHANGE:         break;
	case CCC_CONTROL_PRESHUTDOWN:           break;
	case CCC_CONTROL_CTRL_BREAK:            break;
	case CCC_CONTROL_CTRL_C:                
		LOG_TRACE("ServerEventHandler", "CTRL-C: Gracefully terminating service...");
		if (pServiceControllerInst->IsInteractive() == true) {
			cprint_c("CTRL-C: Gracefully terminating service...\n");
		}
		pServiceControllerInst->Stop();
		// Signal is handled - don't pass it on to the next handler
		Sleep(1000);
		break;
	case CCC_CONTROL_CTRL_CLOSE:            break;
	case CCC_CONTROL_CTRL_LOGOFF:           break;
	case CCC_CONTROL_CTRL_SHUTDOWN:         break;
	case CCC_CONTROL_CTRL_UNKNOWN:          break;
	default:
		break;
	}
}