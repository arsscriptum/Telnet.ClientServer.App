
//==============================================================================
//
//   firewall.cpp
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#ifndef __FIREWALL_WIN_NETWORK_SERVER_H__
#define __FIREWALL_WIN_NETWORK_SERVER_H__
#include "stdafx.h"
#include <string>
#include <netfw.h>


class WindowsFirewall
{
	HRESULT comInit = E_FAIL;
	INetFwProfile* FirewallProfile = nullptr;

	public:
		WindowsFirewall();
		~WindowsFirewall();

		bool IsOn();
		bool TurnOn();
		bool TurnOff();

		bool AddProgramException(STD_STRING exefullpath, STD_STRING name);
		static void RemoveProgramException(STD_STRING exefullpath, STD_STRING name);
		bool IsProgamEnabled(std::wstring exefullpath);
};


#endif //__FIREWALL_WIN_NETWORK_SERVER_H__