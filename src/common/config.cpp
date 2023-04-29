

//==============================================================================
//
//   config.cpp
//
//==============================================================================
//  Copyright (C) Guilaume Plante 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================

#include "stdafx.h"
#include "config.h"
#include <random>
#include "utilities.h"
#include "ini.h"

Global_Settings INTERNAL::_Global_Settings;

int Handler(void* user, const char* section, const char* name, const char* value)
{
	return INTERNAL::_Global_Settings.SettingsHandler(user, section, name, value);
}
Global_Settings::Global_Settings(){
	Defaults();


	string config = "F:\\Code\\Telnet.ClientServer.App\\servercfg.ini";
	if (FileExists(config)){//file exists,read it in

		FILE* file;
		int error;

		file = fopen(config.c_str(), "r");
		if (!file)
			return;
		error = ini_parse_file(file, Handler, "");
		fclose(file);
	}
}

void Global_Settings::Defaults(){
	memset(this, 0, sizeof(this));
	_STRNCPY(Service_Name, DEFAULT_SERVCE_NAME, ARRAYSIZE(Service_Name));
	_STRNCPY(Service_Display_Name, DEFAULT_SERVCE_PERTTY_NAME, ARRAYSIZE(Service_Display_Name));
	_STRNCPY(DefaultPort, DEFAULT_NETWORK_PORT, ARRAYSIZE(DefaultPort));
	_STRNCPY(DefaultGateway, DEFAULT_GATEWAY_HOST, ARRAYSIZE(DefaultGateway));
	_STRNCPY(DefaultProxyGetSessionURL, DEFAULT_PROXY_URL, ARRAYSIZE(DefaultProxyGetSessionURL));
	_STRNCPY(Unique_ID, _T(""), ARRAYSIZE(Unique_ID));
	_STRNCPY(Service_AccountName, DEFAULT_SERVCE_ACCOUNTNAME, ARRAYSIZE(Service_AccountName));
	_STRNCPY(Service_Password, DEFAULT_SERVCE_PASSWORD, ARRAYSIZE(Service_Password));
	_STRNCPY(ServiceBinaryPath, DEFAULT_SERVCE_PASSWORD, ARRAYSIZE(ServiceBinaryPath));

	ServiceDesiredAccess = SERVICE_ALL_ACCESS;
	ServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStartType = SERVICE_AUTO_START;
	ServiceErrorControl = SERVICE_ERROR_IGNORE;
}

 int Global_Settings::SettingsHandler(void* user, const char* section, const char* name, const char* value)
{

	if (MATCH("Service", "ServiceName")) {
		_STRNCPY(Service_Name, value, ARRAYSIZE(Service_Name));
	}
	else if (MATCH("Service", "ServiceDisplayName")) {
		_STRNCPY(Service_Display_Name, value, ARRAYSIZE(Service_Display_Name));
	}
	else if (MATCH("Network", "ListenPort")) {
		_STRNCPY(DefaultPort, value, ARRAYSIZE(DefaultPort));
	}
	else {
		return 0;
	}
	return 1;
}


TCHAR* Service_Name(){
	return INTERNAL::_Global_Settings.Service_Name;
}
TCHAR* Service_Display_Name(){
	return INTERNAL::_Global_Settings.Service_Display_Name;
}
TCHAR* DefaultPort(){
	return INTERNAL::_Global_Settings.DefaultPort;
}
TCHAR* DefaultGateway(){
	return INTERNAL::_Global_Settings.DefaultGateway;
}
TCHAR* DefaultProxyGetSessionURL(){
	return INTERNAL::_Global_Settings.DefaultProxyGetSessionURL;
}

TCHAR* Unique_ID(){

	if (INTERNAL::_Global_Settings.Unique_ID[0] == 0 || INTERNAL::_Global_Settings.Unique_ID[0] == L'\n'){//dont have an id try to load or generate it
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<double> dist(MININT32, MAXINT32);
		int randint = (int)dist(mt);
		auto uniqueid = TO_STD_STRING(randint);
		//update file and settings
		_STRNCPY(INTERNAL::_Global_Settings.Unique_ID, uniqueid.c_str(), uniqueid.size()+1);
	}
	return INTERNAL::_Global_Settings.Unique_ID;
}

TCHAR* Service_AccountName() {
	return INTERNAL::_Global_Settings.Service_AccountName;
}

TCHAR* Service_Password() {
	return INTERNAL::_Global_Settings.Service_Password;
}

TCHAR* Service_BinaryPath() { return INTERNAL::_Global_Settings.ServiceBinaryPath; };
DWORD Service_DesiredAccess() { return INTERNAL::_Global_Settings.ServiceDesiredAccess; };
DWORD Service_Type() { return INTERNAL::_Global_Settings.ServiceType; };
DWORD Service_StartType() { return INTERNAL::_Global_Settings.ServiceStartType; };
DWORD Service_ErrorControl() { return INTERNAL::_Global_Settings.ServiceErrorControl; };