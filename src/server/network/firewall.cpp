

//==============================================================================
//
//   firewall.cpp
//
//==============================================================================
//  arsscriptum - made in quebec 2020 <guillaumeplante.qc@gmail.com>
//==============================================================================


#include "stdafx.h"
#include "firewall.h"

#include <crtdbg.h>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
#include <stdio.h>

# define net_firewall_log( ... )

#ifdef _ENABLE_FIREWALL_CODE

HRESULT WindowsFirewallIsOn(IN INetFwProfile* fwProfile, OUT BOOL* fwOn)
{
	HRESULT hr = S_OK;
	VARIANT_BOOL fwEnabled;

	_ASSERT(fwProfile != NULL);
	_ASSERT(fwOn != NULL);

	*fwOn = FALSE;

	// Get the current state of the firewall.
	hr = fwProfile->get_FirewallEnabled(&fwEnabled);
	if (FAILED(hr))
	{
		LOG_TRACE("Network::Firewall", "get_FirewallEnabled failed: 0x%08lx", hr);
		goto error;
	}

	// Check to see if the firewall is on.
	if (fwEnabled != VARIANT_FALSE)
	{
		*fwOn = TRUE;
		LOG_TRACE("Network::Firewall", "The firewall is on.");
	}
	else
	{
		LOG_TRACE("Network::Firewall", "The firewall is off.");
	}

error:

	return hr;
}


HRESULT WindowsFirewallTurnOn(IN INetFwProfile* fwProfile)
{
	HRESULT hr = S_OK;
	BOOL fwOn;

	_ASSERT(fwProfile != NULL);

	// Check to see if the firewall is off.
	hr = WindowsFirewallIsOn(fwProfile, &fwOn);
	if (FAILED(hr))
	{
		LOG_TRACE("Network::Firewall", "WindowsFirewallIsOn failed: 0x%08lx", hr);
		goto error;
	}

	// If it is, turn it on.
	if (!fwOn)
	{
		// Turn the firewall on.
		hr = fwProfile->put_FirewallEnabled(VARIANT_TRUE);
		if (FAILED(hr))
		{
			LOG_TRACE("Network::Firewall", "put_FirewallEnabled failed: 0x%08lx", hr);
			goto error;
		}

		LOG_TRACE("Network::Firewall", "The firewall is now on.");
	}

error:

	return hr;
}


HRESULT WindowsFirewallTurnOff(IN INetFwProfile* fwProfile)
{
	HRESULT hr = S_OK;
	BOOL fwOn;

	_ASSERT(fwProfile != NULL);

	// Check to see if the firewall is on.
	hr = WindowsFirewallIsOn(fwProfile, &fwOn);
	if (FAILED(hr))
	{
		LOG_TRACE("Network::Firewall", "WindowsFirewallIsOn failed: 0x%08lx", hr);
		goto error;
	}

	// If it is, turn it off.
	if (fwOn)
	{
		// Turn the firewall off.
		hr = fwProfile->put_FirewallEnabled(VARIANT_FALSE);
		if (FAILED(hr))
		{
			LOG_TRACE("Network::Firewall", "put_FirewallEnabled failed: 0x%08lx", hr);
			goto error;
		}

		LOG_TRACE("Network::Firewall", "The firewall is now off.");
	}

error:

	return hr;
}



HRESULT WindowsFirewallInitialize(OUT INetFwProfile** fwProfile)
{
	HRESULT hr = S_OK;
	INetFwMgr* fwMgr = NULL;
	INetFwPolicy* fwPolicy = NULL;


	*fwProfile = NULL;

	// Create an instance of the firewall settings manager.
	hr = CoCreateInstance(
		__uuidof(NetFwMgr),
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(INetFwMgr),
		(void**)&fwMgr
		);
	if (FAILED(hr))
	{
		goto error;
	}

	// Retrieve the local firewall policy.
	hr = fwMgr->get_LocalPolicy(&fwPolicy);
	if (FAILED(hr))
	{

		goto error;
	}

	// Retrieve the firewall profile currently in effect.
	hr = fwPolicy->get_CurrentProfile(fwProfile);
	if (FAILED(hr))
	{
		goto error;
	}

error:

	// Release the local firewall policy.
	if (fwPolicy != NULL)
	{
		fwPolicy->Release();
	}

	// Release the firewall settings manager.
	if (fwMgr != NULL)
	{
		fwMgr->Release();
	}

	return hr;
}
HRESULT WindowsFirewallAppIsEnabled(
	IN INetFwProfile* fwProfile,
	IN const TCHAR* fwProcessImageFileName,
	OUT BOOL* fwAppEnabled
	)
{
	HRESULT hr = S_OK;
	BSTR fwBstrProcessImageFileName = NULL;
	VARIANT_BOOL fwEnabled;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(fwProfile != NULL);
	_ASSERT(fwProcessImageFileName != NULL);
	_ASSERT(fwAppEnabled != NULL);

	*fwAppEnabled = FALSE;

	// Retrieve the authorized application collection.
	hr = fwProfile->get_AuthorizedApplications(&fwApps);
	if (FAILED(hr))
	{
		net_firewall_log("get_AuthorizedApplications failed: %", hr);
		goto error;
	}

	// Allocate a BSTR for the process image file name.
	fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
	if (fwBstrProcessImageFileName == NULL)
	{
		hr = E_OUTOFMEMORY;
		net_firewall_log("SysAllocString failed: %", hr);
		goto error;
	}

	// Attempt to retrieve the authorized application.
	hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
	if (SUCCEEDED(hr))
	{
		// Find out if the authorized application is enabled.
		hr = fwApp->get_Enabled(&fwEnabled);
		if (FAILED(hr))
		{
			net_firewall_log("get_Enabled failed: %", hr);
			goto error;
		}

		if (fwEnabled != VARIANT_FALSE)
		{
			// The authorized application is enabled.
			*fwAppEnabled = TRUE;

			net_firewall_log(
				"Authorized application % is enabled in the firewall.",
				fwProcessImageFileName
				);
		}
		else
		{
			net_firewall_log(
				"Authorized application % is disabled in the firewall.",
				fwProcessImageFileName
				);
		}
	}
	else
	{
		// The authorized application was not in the collection.
		hr = S_OK;

		net_firewall_log(
			"Authorized application % is disabled in the firewall.",
			fwProcessImageFileName
			);
	}

error:

	// Free the BSTR.
	SysFreeString(fwBstrProcessImageFileName);

	// Release the authorized application instance.
	if (fwApp != NULL)
	{
		fwApp->Release();
	}

	// Release the authorized application collection.
	if (fwApps != NULL)
	{
		fwApps->Release();
	}

	return hr;
}


HRESULT WindowsFirewallAddApp(
	IN INetFwProfile* fwProfile,
	IN const TCHAR* fwProcessImageFileName,
	IN const TCHAR* fwName
	)
{
	HRESULT hr = S_OK;
	BOOL fwAppEnabled;
	BSTR fwBstrName = NULL;
	BSTR fwBstrProcessImageFileName = NULL;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(fwProfile != NULL);
	_ASSERT(fwProcessImageFileName != NULL);
	_ASSERT(fwName != NULL);

	// First check to see if the application is already authorized.
	hr = WindowsFirewallAppIsEnabled(
		fwProfile,
		fwProcessImageFileName,
		&fwAppEnabled
		);
	if (FAILED(hr))
	{
		net_firewall_log("WindowsFirewallAppIsEnabled failed: %", hr);
		goto error;
	}

	// Only add the application if it isn't already authorized.
	if (!fwAppEnabled)
	{
		// Retrieve the authorized application collection.
		hr = fwProfile->get_AuthorizedApplications(&fwApps);
		if (FAILED(hr))
		{
			net_firewall_log("get_AuthorizedApplications failed: %", hr);
			goto error;
		}

		// Create an instance of an authorized application.
		hr = CoCreateInstance(
			__uuidof(NetFwAuthorizedApplication),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(INetFwAuthorizedApplication),
			(void**)&fwApp
			);
		if (FAILED(hr))
		{
			net_firewall_log("CoCreateInstance failed: %", hr);
			goto error;
		}

		// Allocate a BSTR for the process image file name.
		fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
		if (fwBstrProcessImageFileName == NULL)
		{
			hr = E_OUTOFMEMORY;
			net_firewall_log("SysAllocString failed: %", hr);
			goto error;
		}

		// Set the process image file name.
		hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
		if (FAILED(hr))
		{
			net_firewall_log("put_ProcessImageFileName failed: %", hr);
			goto error;
		}

		// Allocate a BSTR for the application friendly name.
		fwBstrName = SysAllocString(fwName);
		if (SysStringLen(fwBstrName) == 0)
		{
			hr = E_OUTOFMEMORY;
			net_firewall_log("SysAllocString failed: %", hr);
			goto error;
		}

		// Set the application friendly name.
		hr = fwApp->put_Name(fwBstrName);
		if (FAILED(hr))
		{
			net_firewall_log("put_Name failed: %", hr);
			goto error;
		}

		// Add the application to the collection.
		hr = fwApps->Add(fwApp);
		if (FAILED(hr))
		{
			net_firewall_log("Add failed: %", hr);
			goto error;
		}

		net_firewall_log(
			"Authorized application %lS is now enabled in the firewall.",
			fwProcessImageFileName
			);
	}

error:

	// Free the BSTRs.
	SysFreeString(fwBstrName);
	SysFreeString(fwBstrProcessImageFileName);

	// Release the authorized application instance.
	if (fwApp != NULL)
	{
		fwApp->Release();
	}

	// Release the authorized application collection.
	if (fwApps != NULL)
	{
		fwApps->Release();
	}

	return hr;
}



WindowsFirewall::WindowsFirewall(){

	HRESULT hr = S_OK;
	// Initialize COM.
	comInit = CoInitializeEx( 0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	// Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
	// initialized with a different mode. Since we don't care what the mode is,
	// we'll just use the existing mode.
	if (comInit != RPC_E_CHANGED_MODE)
	{
		hr = comInit;
		if (FAILED(hr))
		{
			net_firewall_log("CoInitializeEx failed: %", hr);
			return;
		}
	}

	// Retrieve the firewall profile currently in effect.
	hr = WindowsFirewallInitialize(&FirewallProfile);
	if (FAILED(hr))
	{
		net_firewall_log("WindowsFirewallInitialize failed: %", hr);
	}

}
WindowsFirewall::~WindowsFirewall(){
	if (FirewallProfile != nullptr) FirewallProfile->Release();
	// Uninitialize COM.
	if (SUCCEEDED(comInit))
	{
		CoUninitialize();
	}

}
bool WindowsFirewall::AddProgramException(STD_STRING exefullpath, STD_STRING name){
	if (FirewallProfile == nullptr) return false;
	return SUCCEEDED(WindowsFirewallAddApp(FirewallProfile, exefullpath.c_str(), name.c_str()));
}
void WindowsFirewall::RemoveProgramException(STD_STRING exefullpath, STD_STRING name){
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	TCHAR szCmd[2 * MAX_PATH];
	STD_STRING command = _T("cmd.exe /C netsh advfirewall firewall delete rule name=\"");
	command += name;
	command += _T("\"");
	command += _T(" program=\"");
	command += exefullpath;
	command += _T("\"");
	_STRCPY(szCmd, command.c_str());
	CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}

bool WindowsFirewall::IsOn(){
	if (FirewallProfile == nullptr) return false;
	BOOL onstat = FALSE;
	if (SUCCEEDED(WindowsFirewallIsOn(FirewallProfile, &onstat))) return onstat == TRUE;
	return false;
}
bool WindowsFirewall::TurnOn(){
	if (FirewallProfile == nullptr) return false;
	return SUCCEEDED(WindowsFirewallTurnOn(FirewallProfile));
}
bool WindowsFirewall::TurnOff(){
	if (FirewallProfile == nullptr) return false;
	return SUCCEEDED(WindowsFirewallTurnOff(FirewallProfile));
}

#endif