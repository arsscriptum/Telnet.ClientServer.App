

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


Global_Settings INTERNAL::_Global_Settings;

Global_Settings::Global_Settings(){
	Defaults();


	auto config = GetExePath() + "\\" + CONFIGFILE_NAME;
	if (FileExists(config)){//file exists,read it in
		std::ifstream configfile(config.c_str(), std::ios::binary);
		configfile.read((char*)INTERNAL::_Global_Settings.Unique_ID, sizeof(INTERNAL::_Global_Settings.Unique_ID));
		configfile.read((char*)INTERNAL::_Global_Settings.Last_UserConnectName, sizeof(INTERNAL::_Global_Settings.Last_UserConnectName));
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
void Global_Settings::FlushToDisk(){
	auto config = GetExePath() + "\\" + CONFIGFILE_NAME;
	std::ofstream configfile(config.c_str(), std::ios::binary | std::ios::trunc);//clear the file each flush
	configfile.write((char*)INTERNAL::_Global_Settings.Unique_ID, sizeof(INTERNAL::_Global_Settings.Unique_ID));
	configfile.write((char*)INTERNAL::_Global_Settings.Last_UserConnectName, sizeof(INTERNAL::_Global_Settings.Last_UserConnectName));
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
		INTERNAL::_Global_Settings.FlushToDisk();
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