#include "stdafx.h"
#include <afx.h>

/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1997 by Joerg Koenig and the ADG mbH, Mannheim, Germany
// All rights reserved
//
// Distribute freely, except: don't remove my name from the source or
// documentation (don't take credit for my work), mark your changes (don't
// get me blamed for your possible bugs), don't alter or remove this
// notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    J.Koenig@adg.de                 (company site)
//    Joerg.Koenig@rhein-neckar.de    (private site)
/////////////////////////////////////////////////////////////////////////////
//
// MODIFIED BY TODD C. WILSON FOR THE ROAD RUNNER NT LOGIN SERVICE.
// HOWEVER, THESE MODIFICATIONS ARE BROADER IN SCOPE AND USAGE AND CAN BE USED
// IN OTHER PROJECTS WITH NO CHANGES.
// MODIFIED LINES FLAGGED/BRACKETED BY "//!! TCW MOD"
//
/////////////////////////////////////////////////////////////////////////////


// last revised: $Date: 11.05.98 21:09 $, $Revision: 3 $


/////////////////////////////////////////////////////////////////////////////
// Acknoledgements:
//	o	Thanks to Victor Vogelpoel (VictorV@Telic.nl) for his bug-fixes
//		and enhancements.
//	o	Thanks to Todd C. Wilson (todd@mediatec.com) for the
//		"service" on Win95
//
// Changes:
//	01/21/99
//	o	Bug fixed in "DeregisterApplicationLog()"
//		thanks to Grahame Willis (grahamew@questsoftware.com.au):
//
//	04/30/98
//	o	Added two more switches to handle command line arguments:
//		-e will force a running service to stop (corresponding
//		method in this class: virtual BOOL EndService();) and
//		-s will force the service to start (method:
//		virtual BOOL StartupService())
//
//	02/05/98
//	o	Added the methods "RegisterApplicationLog()" and
//		"DeregisterApplicationLog()" (both virtual). The first one will be
//		called from "InstallService()" and creates some registry-entries
//		for a better event-log. The second one removes these entries when
//		the service will uninstall (see "RemoveService()")
//	o	The service now obtains the security identifier of the current user
//		and uses this SID for event-logging.
//	o	The memory allocated by "CommandLineToArgvW()" will now release
//		(UNICODE version only)
//	o	The service now uses a simple message catalogue for a nicer
//		event logging

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <crtdbg.h>

#include <io.h>			//!! TCW MOD
#include <fcntl.h>		//!! TCW MOD

#include "BaseService.h"
#include "BaseServiceEventLogMsg.h"


#ifndef RSP_SIMPLE_SERVICE
	#define RSP_SIMPLE_SERVICE 1
#endif
#ifndef RSP_UNREGISTER_SERVICE
	#define RSP_UNREGISTER_SERVICE 0
#endif

BOOL BaseService :: m_bInstance = FALSE;

static BaseService * gpTheService = 0;			// the one and only instance

BaseService * AfxGetService() { return gpTheService; }




static LPCTSTR gszAppRegKey = TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\");
static LPCTSTR gszWin95ServKey=TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunServices");	//!! TCW MOD


/////////////////////////////////////////////////////////////////////////////
// class CNTService -- construction/destruction

BaseService :: BaseService( LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpAccountName, LPCSTR lpPassword )
	: m_lpServiceName(lpServiceName)
	, m_lpDisplayName(lpDisplayName)
	, m_pszAccountName(lpAccountName)
	, m_pszPassword(lpPassword)
	, m_dwCheckPoint(0)
	, m_dwErr(0)
	, m_bDebug(FALSE)
	, m_sshStatusHandle(0)
	, m_dwControlsAccepted(SERVICE_ACCEPT_STOP)
	, m_pUserSID(0)
	, m_fConsoleReady(FALSE)
	// parameters to the "CreateService()" function:
	, m_dwDesiredAccess(SERVICE_ALL_ACCESS)
	, m_dwServiceType(SERVICE_WIN32_OWN_PROCESS)
	, m_dwStartType(SERVICE_AUTO_START)
	, m_dwErrorControl(SERVICE_ERROR_NORMAL)
	, m_pszLoadOrderGroup(0)
	, m_dwTagID(0)
	, m_pszDependencies(0)
	, m_Debugging(false)

{
	m_bInstance = TRUE;
	gpTheService = this;
	
	// SERVICE_STATUS members that rarely change
	m_ssStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_ssStatus.dwServiceSpecificExitCode = 0;


		/////////////////////////////////////////////////////////////////////////
		// Providing a SID (security identifier) was contributed by Victor
		// Vogelpoel (VictorV@Telic.nl).
		// The code from Victor was slightly modified.

		// Get security information of current user
		BYTE	security_identifier_buffer[ 4096 ];
		DWORD	dwSizeSecurityIdBuffer = sizeof( security_identifier_buffer );
		PSID	user_security_identifier = NULL;

		TCHAR sUserName[ 256 ];
		DWORD dwSizeUserName  =  255;

		TCHAR sDomainName[ 256 ];
		DWORD dwSizeDomainName = 255;

		SID_NAME_USE sidTypeSecurityId;

		::ZeroMemory( sUserName, sizeof( sUserName ) );
		::ZeroMemory( sDomainName, sizeof( sDomainName ) );
		::ZeroMemory( security_identifier_buffer, dwSizeSecurityIdBuffer );

		::GetUserName( sUserName, &dwSizeUserName );

		if( ::LookupAccountName(
					0,
					sUserName,
					&security_identifier_buffer,
					&dwSizeSecurityIdBuffer,
					sDomainName,
					&dwSizeDomainName,
					&sidTypeSecurityId
				)) {
			if( ::IsValidSid( PSID(security_identifier_buffer) ) ) {
				DWORD dwSidLen = ::GetLengthSid(PSID(security_identifier_buffer));
				m_pUserSID = PSID(new BYTE [dwSidLen]);
				::CopySid(dwSidLen, m_pUserSID, security_identifier_buffer);
				_ASSERTE(::EqualSid(m_pUserSID, security_identifier_buffer));
			}
		}
	
	/////////////////////////////////////////////////////////////////////////
}


BaseService :: ~BaseService() {
	_ASSERTE( m_bInstance );
	delete [] LPBYTE(m_pUserSID);
	m_bInstance = FALSE;
	gpTheService = 0;
}



BOOL BaseService :: RegisterService( ) {
	BOOL (BaseService::* fnc)() = &BaseService::StartDispatcher;
	LOG_TRACE("BaseService::RegisterService", "BaseService::StartDispatcher");
	return (this->*fnc)();
}


BOOL BaseService :: StartDispatcher() {
    // Default implementation creates a single threaded service.
	// Override this method and provide more table entries for
	// a multithreaded service (one entry for each thread).
	SERVICE_TABLE_ENTRY dispatchTable[] =
    {
        { LPTSTR(m_lpServiceName), (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { 0, 0 }
    };

	BOOL bRet = StartServiceCtrlDispatcher(dispatchTable);
	if( ! bRet ) {
		TCHAR szBuf[256];
        AddToMessageLog(GetLastErrorText(szBuf,255));
	}

	return bRet;
}


BOOL BaseService :: InstallService() {
    TCHAR szPath[1024];
	LOG_TRACE("BaseService::InstallService", "");
	SetupConsole();	//!! TCW MOD - have to show the console here for the
					// diagnostic or error reason: orignal class assumed
					// that we were using _main for entry (a console app).
					// This particular usage is a Windows app (no console),
					// so we need to create it. Using SetupConsole with _main
					// is ok - does nothing, since you only get one console.

	if( GetModuleFileName( 0, szPath, 1023 ) == 0 ) {
		TCHAR szErr[256];
		LOG_ERROR("BaseService::InstallService", "Unable to install % s - % s", m_lpDisplayName, GetLastErrorText(szErr, 256));
		
		return FALSE;
	}

	BOOL bRet = FALSE;


		// Real NT services go here.
		SC_HANDLE schSCManager =	OpenSCManager(
										0,						// machine (NULL == local)
										0,						// database (NULL == default)
										SC_MANAGER_ALL_ACCESS	// access required
									);
		if( schSCManager ) {
			SC_HANDLE schService =	CreateService(
										schSCManager,
										m_lpServiceName,
										m_lpDisplayName,
										m_dwDesiredAccess,
										m_dwServiceType,
										m_dwStartType,
										m_dwErrorControl,
										szPath,
										m_pszLoadOrderGroup,
										((m_dwServiceType == SERVICE_KERNEL_DRIVER ||
										  m_dwServiceType == SERVICE_FILE_SYSTEM_DRIVER) &&
										 (m_dwStartType == SERVICE_BOOT_START ||
										  m_dwStartType == SERVICE_SYSTEM_START)) ?
											&m_dwTagID : 0,
										m_pszDependencies,
										"LOCALSYSTEM",
										nullptr
									);

			if( schService ) {
				LOG_TRACE("BaseService::InstallService", "%s installed.", m_lpDisplayName );
				CloseServiceHandle(schService);
				bRet = TRUE;
			} else {
				TCHAR szErr[256];
				LOG_ERROR("BaseService::InstallService", "CreateService failed - %s", GetLastErrorText(szErr, 256));
			}

			CloseServiceHandle(schSCManager);
		 } else {
			TCHAR szErr[256];
			LOG_ERROR("BaseService::InstallService", "OpenSCManager failed - %s", GetLastErrorText(szErr,256));
		}

		if( bRet ) {
			// installation succeeded. Now register the message file
			RegisterApplicationLog(
				szPath,		// the path to the application itself
				EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE // supported types
			);

			AddToMessageLog("Service installed",EVENTLOG_INFORMATION_TYPE);
		}


	return bRet;
}


BOOL BaseService :: RemoveService() {
	BOOL bRet = FALSE;

	SetupConsole();	//!! TCW MOD - have to show the console here for the
					// diagnostic or error reason: orignal class assumed
					// that we were using _main for entry (a console app).
					// This particular usage is a Windows app (no console),
					// so we need to create it. Using SetupConsole with _main
					// is ok - does nothing, since you only get one console.



		// Real NT services go here.
		SC_HANDLE schSCManager = OpenSCManager(
									0,						// machine (NULL == local)
									0,						// database (NULL == default)
									SC_MANAGER_ALL_ACCESS	// access required
								);
		if( schSCManager ) {
			SC_HANDLE schService =	OpenService(
										schSCManager,
										m_lpServiceName,
										SERVICE_ALL_ACCESS
									);

			if( schService ) {
				// try to stop the service
				if (ControlService(schService, SERVICE_CONTROL_STOP, &m_ssStatus)) {
					LOG_TRACE("BaseService::RemoveService", "Stopping %s.", m_lpDisplayName);
					Sleep(1000);

					while (QueryServiceStatus(schService, &m_ssStatus)) {
						if (m_ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
							LOG_TRACE("BaseService::RemoveService", ".");
							Sleep(1000);
						}
						else
							break;
					}

					if (m_ssStatus.dwCurrentState == SERVICE_STOPPED) {
						LOG_TRACE("BaseService::RemoveService", "\n%s stopped.", m_lpDisplayName);
					}
					else {
						LOG_ERROR("BaseService::RemoveService", "\n%s failed to stop.", m_lpDisplayName);
					}
				}

				// now remove the service
				if( DeleteService(schService) ) {
					LOG_TRACE("BaseService::RemoveService", "%s removed.", m_lpDisplayName);
					bRet = TRUE;
				} else {
					TCHAR szErr[256];
					LOG_ERROR("BaseService::RemoveService", "DeleteService failed - %s", GetLastErrorText(szErr,256));
				}

				CloseServiceHandle(schService);
			} else {
				TCHAR szErr[256];
				LOG_ERROR("BaseService::RemoveService", "OpenService failed - %s", GetLastErrorText(szErr,256));
			}

			  CloseServiceHandle(schSCManager);
		 } else {
			TCHAR szErr[256];
			LOG_ERROR("BaseService::RemoveService", "OpenSCManager failed - %s", GetLastErrorText(szErr,256));
		}

		if( bRet )
			DeregisterApplicationLog();

	return bRet;
}


BOOL BaseService :: EndService() {
	BOOL bRet = FALSE;

	SC_HANDLE schSCManager = ::OpenSCManager(
								0,						// machine (NULL == local)
								0,						// database (NULL == default)
								SC_MANAGER_ALL_ACCESS	// access required
							);
	if( schSCManager ) {
		SC_HANDLE schService =	::OpenService(
									schSCManager,
									m_lpServiceName,
									SERVICE_ALL_ACCESS
								);

		if( schService ) {
			// try to stop the service
			if( ::ControlService(schService, SERVICE_CONTROL_STOP, &m_ssStatus) ) {
				LOG_TRACE("BaseService::EndService", "Stopping %s.", m_lpDisplayName);
				::Sleep(1000);

				while( ::QueryServiceStatus(schService, &m_ssStatus) ) {
					if( m_ssStatus.dwCurrentState == SERVICE_STOP_PENDING ) {
						LOG_TRACE("BaseService::EndService", ".");
						::Sleep( 1000 );
					} else
						break;
				}

				if (m_ssStatus.dwCurrentState == SERVICE_STOPPED) {
					bRet = TRUE;
					LOG_TRACE("BaseService::EndService", "\n%s stopped.", m_lpDisplayName);
				}
				else {
					LOG_ERROR("BaseService::EndService", "\n%s failed to stop.", m_lpDisplayName);
				}
                    
			}

			::CloseServiceHandle(schService);
		} else {
			TCHAR szErr[256];
			LOG_ERROR("BaseService::EndService", "OpenService failed - %s", GetLastErrorText(szErr,256));
		}

        ::CloseServiceHandle(schSCManager);
    } else {
		TCHAR szErr[256];
		LOG_ERROR("BaseService::EndService", "OpenSCManager failed - %s", GetLastErrorText(szErr,256));
	}

	return bRet;
}


BOOL BaseService :: StartupService() {
	BOOL bRet = FALSE;

	SC_HANDLE schSCManager = ::OpenSCManager(
								0,						// machine (NULL == local)
								0,						// database (NULL == default)
								SC_MANAGER_ALL_ACCESS	// access required
							);
	if( schSCManager ) {
		SC_HANDLE schService =	::OpenService(
									schSCManager,
									m_lpServiceName,
									SERVICE_ALL_ACCESS
								);

		if( schService ) {
			// try to start the service
			LOG_TRACE("BaseService::StartupService", "Starting up %s.", m_lpDisplayName);
			if( ::StartService(schService, 0, 0) ) {
				Sleep(1000);

				while( ::QueryServiceStatus(schService, &m_ssStatus) ) {
					if( m_ssStatus.dwCurrentState == SERVICE_START_PENDING ) {
						LOG_TRACE("BaseService::StartupService", ".");
						Sleep( 1000 );
					} else
						break;
				}

				if (m_ssStatus.dwCurrentState == SERVICE_RUNNING) {
					bRet = TRUE;
					LOG_TRACE("BaseService::StartupService", "%s started.", m_lpDisplayName);
				}
				else {
					LOG_ERROR("BaseService::StartupService", "%s failed to start.", m_lpDisplayName);
				}

			
                    
			} else {
				// StartService failed
				TCHAR szErr[256];
				LOG_ERROR("BaseService::StartupService", "%s failed to start: %s", m_lpDisplayName, GetLastErrorText(szErr,256));
			}

			::CloseServiceHandle(schService);
		} else {
			TCHAR szErr[256];
			LOG_ERROR("BaseService::StartupService", "OpenService failed - %s", GetLastErrorText(szErr,256));
		}

        ::CloseServiceHandle(schSCManager);
    } else {
		TCHAR szErr[256];
		LOG_ERROR("BaseService::StartupService", "OpenSCManager failed - %s", GetLastErrorText(szErr,256));
	}

	return bRet;
}




BOOL BaseService :: DebugService(int argc, char ** argv) {

	SetupConsole();	

	SetConsoleCtrlHandler(ControlHandler, TRUE);
	
    Start();

	m_Debugging = true;

	return TRUE;
}


void BaseService :: Pause() {
}


void BaseService :: Continue() {
}


void BaseService :: Shutdown() {
	m_Debugging = false;
}


/////////////////////////////////////////////////////////////////////////////
// class CNTService -- default handlers

void WINAPI BaseService :: ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv) {
	_ASSERTE( gpTheService != 0 );
	LOG_TRACE("BaseService::ServiceMain", "register our service control handler for %s", gpTheService->m_lpDisplayName);
	// register our service control handler:
	gpTheService->m_sshStatusHandle =	RegisterServiceCtrlHandler(
											gpTheService->m_lpServiceName,
											BaseService::ServiceCtrl
										);

	if( gpTheService->m_sshStatusHandle )
		LOG_TRACE("BaseService::ServiceMain", "report the status to the service control manager. SERVICE_START_PENDING");
		// report the status to the service control manager.
		if( gpTheService->ReportStatus(SERVICE_START_PENDING) ){
			gpTheService->Start( );}

	// try to report the stopped status to the service control manager.
	if( gpTheService->m_sshStatusHandle )
		gpTheService->ReportStatus(SERVICE_STOPPED);
}


void WINAPI BaseService :: ServiceCtrl(DWORD dwCtrlCode) {
	_ASSERTE( gpTheService != 0 );

	// Handle the requested control code.
	switch( dwCtrlCode ) {
		case SERVICE_CONTROL_STOP:
			// Stop the service.
			gpTheService->m_ssStatus.dwCurrentState = SERVICE_STOP_PENDING;
			gpTheService->Stop();
			break;

		case SERVICE_CONTROL_PAUSE:
			gpTheService->m_ssStatus.dwCurrentState = SERVICE_PAUSE_PENDING;
			gpTheService->Pause();
			break;

		case SERVICE_CONTROL_CONTINUE:
			gpTheService->m_ssStatus.dwCurrentState = SERVICE_CONTINUE_PENDING;
			gpTheService->Continue();
			break;

		case SERVICE_CONTROL_SHUTDOWN:
			gpTheService->Shutdown();
			break;

		case SERVICE_CONTROL_INTERROGATE:
			// Update the service status.
			gpTheService->ReportStatus(gpTheService->m_ssStatus.dwCurrentState);
			break;

		default:
			// invalid control code
			break;
	}

}


BOOL WINAPI BaseService :: ControlHandler(DWORD dwCtrlType) {
	_ASSERTE(gpTheService != 0);
	switch( dwCtrlType ) {
		case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
		case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
			LOG_TRACE("BaseService::ControlHandler", "Stopping %s.", gpTheService->m_lpDisplayName);
			gpTheService->Stop();
			return TRUE;
	}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// class CNTService -- helpers

//!! TCW MOD - added DWORD dwErrExit for error exit value. Defaults to zero
BOOL BaseService :: ReportStatus(
						DWORD dwCurrentState,
						DWORD dwWaitHint,
						DWORD dwErrExit ) {
	BOOL fResult = TRUE;

	if( !m_bDebug ) { // when debugging we don't report to the SCM
        if( dwCurrentState == SERVICE_START_PENDING)
            m_ssStatus.dwControlsAccepted = 0;
        else
            m_ssStatus.dwControlsAccepted = m_dwControlsAccepted;

        m_ssStatus.dwCurrentState = dwCurrentState;
        m_ssStatus.dwWin32ExitCode = NO_ERROR;
        m_ssStatus.dwWaitHint = dwWaitHint;

			//!! TCW MOD START - added code to support error exiting
			m_ssStatus.dwServiceSpecificExitCode = dwErrExit;
			if (dwErrExit!=0)
				m_ssStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
			//!! TCW MOD END - added code to support error exiting

        if( dwCurrentState == SERVICE_RUNNING ||
            dwCurrentState == SERVICE_STOPPED )
            m_ssStatus.dwCheckPoint = 0;
        else
            m_ssStatus.dwCheckPoint = ++m_dwCheckPoint;

        // Report the status of the service to the service control manager.
        if (!(fResult = SetServiceStatus( m_sshStatusHandle, &m_ssStatus))) {
            AddToMessageLog("SetServiceStatus() failed");
        }
    }

    return fResult;
}


void BaseService :: AddToMessageLog(const char* lpszMsg, WORD wEventType, DWORD dwEventID) {
#if USE_EVENT_LOGS
	m_dwErr = GetLastError();

	// use default message-IDs
	if( dwEventID == DWORD(-1) ) {
		switch( wEventType ) {
			case EVENTLOG_ERROR_TYPE:
				dwEventID = MSG_ERROR_1;
				break;
			case EVENTLOG_WARNING_TYPE:
				dwEventID = MSG_WARNING_1;
				break;
			case EVENTLOG_INFORMATION_TYPE:
				dwEventID = MSG_INFO_1;
				break;
			case EVENTLOG_AUDIT_SUCCESS:
				dwEventID = MSG_INFO_1;
				break;
			case EVENTLOG_AUDIT_FAILURE:
				dwEventID = MSG_INFO_1;
				break;
			default:
				dwEventID = MSG_INFO_1;
				break;
		}
	}

	// Use event logging to log the error.
	HANDLE hEventSource = RegisterEventSource(0, m_lpServiceName);

	if( hEventSource != 0 ) {
		LPCTSTR lpszMessage = lpszMsg;

		ReportEvent(
			hEventSource,	// handle of event source
			wEventType,		// event type
			0,				// event category
			dwEventID,		// event ID
			m_pUserSID,		// current user's SID
			1,				// strings in lpszStrings
			0,				// no bytes of raw data
			&lpszMessage,	// array of error strings
			0				// no raw data
		);

		::DeregisterEventSource(hEventSource);
    }
#endif
}


LPTSTR BaseService :: GetLastErrorText( LPTSTR lpszBuf, DWORD dwSize ) {
    LPTSTR lpszTemp = 0;

    DWORD dwRet =	::FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_ARGUMENT_ARRAY,
						0,
						GetLastError(),
						LANG_NEUTRAL,
						(LPTSTR)&lpszTemp,
						0,
						0
					);

    if( !dwRet || (dwSize < dwRet+14) )
        lpszBuf[0] = TEXT('\0');
    else {
        lpszTemp[_tcsclen(lpszTemp)-2] = TEXT('\0');  //remove cr/nl characters
        _tcscpy(lpszBuf, lpszTemp);
    }

    if( lpszTemp )
        LocalFree(HLOCAL(lpszTemp));

    return lpszBuf;
}

/////////////////////////////////////////////////////////////////////////////
// class CNTService -- implementation


void BaseService :: RegisterApplicationLog( LPCTSTR lpszFileName, DWORD dwTypes ) {
#if USE_EVENT_LOGS
	TCHAR szKey[256];
	_tcscpy(szKey, gszAppRegKey);
	_tcscat(szKey, m_lpServiceName);
	HKEY hKey = 0;
	LONG lRet = ERROR_SUCCESS;
	
	// Create a key for that application and insert values for
	// "EventMessageFile" and "TypesSupported"
	if( ::RegCreateKey(HKEY_LOCAL_MACHINE, szKey, &hKey) == ERROR_SUCCESS ) {
		lRet =	::RegSetValueEx(
					hKey,						// handle of key to set value for
					TEXT("EventMessageFile"),	// address of value to set
					0,							// reserved
					REG_EXPAND_SZ,				// flag for value type
					(CONST BYTE*)lpszFileName,	// address of value data
					_tcslen(lpszFileName) + 1	// size of value data
				);

		// Set the supported types flags.
		lRet =	::RegSetValueEx(
					hKey,					// handle of key to set value for
					TEXT("TypesSupported"),	// address of value to set
					0,						// reserved
					REG_DWORD,				// flag for value type
					(CONST BYTE*)&dwTypes,	// address of value data
					sizeof(DWORD)			// size of value data
				);
		::RegCloseKey(hKey);
	}

	// Add the service to the "Sources" value

	lRet =	::RegOpenKeyEx( 
				HKEY_LOCAL_MACHINE,	// handle of open key 
				gszAppRegKey,		// address of name of subkey to open 
				0,					// reserved 
				KEY_ALL_ACCESS,		// security access mask 
				&hKey				// address of handle of open key 
			);
	if( lRet == ERROR_SUCCESS ) {
		DWORD dwSize;

		// retrieve the size of the needed value
		lRet =	::RegQueryValueEx(
					hKey,			// handle of key to query 
					TEXT("Sources"),// address of name of value to query 
					0,				// reserved 
					0,				// address of buffer for value type 
					0,				// address of data buffer 
					&dwSize			// address of data buffer size 
				);

 		if( lRet == ERROR_SUCCESS ) {
			DWORD dwType;
			DWORD dwNewSize = dwSize+_tcslen(m_lpServiceName)+1;
			LPBYTE Buffer = LPBYTE(::GlobalAlloc(GPTR, dwNewSize));

			lRet =	::RegQueryValueEx(
						hKey,			// handle of key to query 
						TEXT("Sources"),// address of name of value to query 
						0,				// reserved 
						&dwType,		// address of buffer for value type 
						Buffer,			// address of data buffer 
						&dwSize			// address of data buffer size 
					);
			if( lRet == ERROR_SUCCESS ) {
				_ASSERTE(dwType == REG_MULTI_SZ);

				// check whether this service is already a known source
				register LPTSTR p = LPTSTR(Buffer);
				for(; *p; p += _tcslen(p)+1 ) {
					if( _tcscmp(p, m_lpServiceName) == 0 )
						break;
				}
				if( ! * p ) {
					// We're standing at the end of the stringarray
					// and the service does still not exist in the "Sources".
					// Now insert it at this point.
					// Note that we have already enough memory allocated
					// (see GlobalAlloc() above). We also don't need to append
					// an additional '\0'. This is done in GlobalAlloc() above
					// too.
					_tcscpy(p, m_lpServiceName);

					// OK - now store the modified value back into the
					// registry.
					lRet =	::RegSetValueEx(
								hKey,			// handle of key to set value for
								TEXT("Sources"),// address of value to set
								0,				// reserved
								dwType,			// flag for value type
								Buffer,			// address of value data
								dwNewSize		// size of value data
							);
				}
			}

			::GlobalFree(HGLOBAL(Buffer));
		}

		::RegCloseKey(hKey);
	}
#endif
}


void BaseService :: DeregisterApplicationLog() {
#if USE_EVENT_LOGS
	TCHAR szKey[256];
	_tcscpy(szKey, gszAppRegKey);
	_tcscat(szKey, m_lpServiceName);
	HKEY hKey = 0;
	LONG lRet = ERROR_SUCCESS;

	lRet = ::RegDeleteKey(HKEY_LOCAL_MACHINE, szKey);

	// now we have to delete the application from the "Sources" value too.
	lRet =	::RegOpenKeyEx( 
				HKEY_LOCAL_MACHINE,	// handle of open key 
				gszAppRegKey,		// address of name of subkey to open 
				0,					// reserved 
				KEY_ALL_ACCESS,		// security access mask 
				&hKey				// address of handle of open key 
			);
	if( lRet == ERROR_SUCCESS ) {
		DWORD dwSize;

		// retrieve the size of the needed value
		lRet =	::RegQueryValueEx(
					hKey,			// handle of key to query 
					TEXT("Sources"),// address of name of value to query 
					0,				// reserved 
					0,				// address of buffer for value type 
					0,				// address of data buffer 
					&dwSize			// address of data buffer size 
				);

 		if( lRet == ERROR_SUCCESS ) {
			DWORD dwType;
			LPBYTE Buffer = LPBYTE(::GlobalAlloc(GPTR, dwSize));
			LPBYTE NewBuffer = LPBYTE(::GlobalAlloc(GPTR, dwSize));

			lRet =	::RegQueryValueEx(
						hKey,			// handle of key to query 
						TEXT("Sources"),// address of name of value to query 
						0,				// reserved 
						&dwType,		// address of buffer for value type 
						Buffer,			// address of data buffer 
						&dwSize			// address of data buffer size 
					);
			if( lRet == ERROR_SUCCESS ) {
				_ASSERTE(dwType == REG_MULTI_SZ);

				// check whether this service is already a known source
				register LPTSTR p = LPTSTR(Buffer);
				register LPTSTR pNew = LPTSTR(NewBuffer);
				BOOL bNeedSave = FALSE;	// assume the value is already correct
				for(; *p; p += _tcslen(p)+1) {
					// except ourself: copy the source string into the destination
					if( _tcscmp(p, m_lpServiceName) != 0 ) {
						_tcscpy(pNew, p);
						pNew += _tcslen(pNew)+1;
					} else {
						bNeedSave = TRUE;		// *this* application found
						dwSize -= _tcslen(p)+1;	// new size of value
					}
				}
				if( bNeedSave ) {
					// OK - now store the modified value back into the
					// registry.
					lRet =	::RegSetValueEx(
								hKey,			// handle of key to set value for
								TEXT("Sources"),// address of value to set
								0,				// reserved
								dwType,			// flag for value type
								NewBuffer,		// address of value data
								dwSize			// size of value data
							);
				}
			}

			::GlobalFree(HGLOBAL(Buffer));
			::GlobalFree(HGLOBAL(NewBuffer));
		}

		::RegCloseKey(hKey);
	}
#endif
}

////////////////////////////////////////////////////////

//!! TCW MOD - function to create console for faceless apps if not already there
void BaseService::SetupConsole() {
	if( !m_fConsoleReady ) {
		AllocConsole();	// you only get 1 console.

		// lovely hack to get the standard io (printf, getc, etc) to the new console. Pretty much does what the
		// C lib does for us, but when we want it, and inside of a Window'd app.
		// The ugly look of this is due to the error checking (bad return values. Remove the if xxx checks if you like it that way.
		DWORD astds[3]={STD_OUTPUT_HANDLE,STD_ERROR_HANDLE,STD_INPUT_HANDLE};
		FILE *atrgs[3]={stdout,stderr,stdin};
		for( register int i=0; i<3; i++ ) {
			long hand=(long)GetStdHandle(astds[i]);
			if( hand!=(long)INVALID_HANDLE_VALUE ) {
				int osf=_open_osfhandle(hand,_O_TEXT);
				if( osf!=-1 ) {
					FILE *fp=_fdopen(osf,(astds[i]==STD_INPUT_HANDLE) ? "r" : "w");
					if( fp!=NULL ) {
						*(atrgs[i])=*fp;
						setvbuf(fp,NULL,_IONBF,0);
					}
				}
			}
		}
		m_fConsoleReady=TRUE;
	}
}
