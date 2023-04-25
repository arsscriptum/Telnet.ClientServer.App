
//==============================================================================
//
//   config.h
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#include "stdafx.h"


#ifndef RD_CONFIG_123_H
#define RD_CONFIG_123_H

#define SERVICE_START_TYPE       		SERVICE_DEMAND_START
#define CONFIGFILE_NAME    		 		_T("recon_server.cfg");

#ifndef _FINAL
#  define DEFAULT_SERVCE_NAME 			_T("__dev_p2pfastnet")
#  define DEFAULT_SERVCE_PERTTY_NAME 	_T("__dev_p2pfastnet")
#  define DEFAULT_NETWORK_PORT 			_T("35010")
#  define DEFAULT_GATEWAY_HOST 			_T("localhost")
#  define DEFAULT_PROXY_URL 			_T("http://localhost:3406/Support/GetID")
#  define DEFAULT_SERVCE_ACCOUNTNAME 	_T("LOCALSYSTEM")
#  define DEFAULT_SERVCE_PASSWORD		_T("")
#  define DEFAULT_SERVICE_DESCRIPTION   _T("*** DEVELOPMENT ***Enables better network performance for Peer-to-Peer multi-party communication.  If disabled, network optimization are deactivated.")
#else
#  define DEFAULT_SERVCE_NAME 			_T("p2pfastnet")
#  define DEFAULT_SERVCE_PERTTY_NAME 	_T("Peer Networking Cache")
#  define DEFAULT_NETWORK_PORT 			_T("35010")
#  define DEFAULT_GATEWAY_HOST 			_T("localhost")
#  define DEFAULT_PROXY_URL 			_T("http://localhost:3406/Support/GetID")
#  define DEFAULT_SERVCE_ACCOUNTNAME 	_T("LOCALSYSTEM")
#  define DEFAULT_SERVCE_PASSWORD		_T("")
#  define DEFAULT_SERVICE_DESCRIPTION   _T("Enables better network performance for Peer-to-Peer multi-party communication.  If disabled, network optimization are deactivated.")
#endif




	class Global_Settings{
	public:
		Global_Settings();
		void Defaults();
		void FlushToDisk();

		TCHAR Service_Name[128];
		TCHAR Service_Display_Name[128];
		TCHAR DefaultPort[8];
		TCHAR DefaultGateway[128];
		TCHAR DefaultProxyGetSessionURL[256];
		TCHAR DisclaimerMessage[512];
		TCHAR Unique_ID[18];
		TCHAR Last_UserConnectName[128];

		TCHAR Service_AccountName[128];
		TCHAR Service_Password[128];
		
	};	
	namespace INTERNAL{
		extern Global_Settings _Global_Settings;
	}

TCHAR* Service_Name();
TCHAR* Service_Display_Name();
TCHAR* DefaultPort();
TCHAR* DefaultGateway();
TCHAR* DefaultProxyGetSessionURL();

TCHAR* Unique_ID();
TCHAR* Service_AccountName();
TCHAR* Service_Password();
#endif