
//==============================================================================
//
//   telnetd.cpp
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
#include "win32.h"
#include "shell_server.h"

#define ERR_BUFF_LEN 1024

#pragma comment( lib, "wsock32" )
#pragma comment( lib, "advapi32" ) 

extern long restartCount;

extern volatile BOOL requestReset;

SOCKET client_socket[10];
int max_clients = 10;
void (*_EventNotifierFunction)(WORD EventCode);
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#endif

extern unsigned int gNetClientIndex;

int Invoke_ShellServer_Networking();

//Winsock Data block
WSADATA wi;
//Thread handles
UINT thrid_sock;
UINT thrid_console;
UINT thrid_error;
//Stdin/out handles
HANDLE stdinput;
HANDLE stdoutput;
HANDLE stderror;
// "Input" pipe for the console.
HANDLE readInput;
HANDLE writeInput;
// Console "Output" pipe.
HANDLE readOutput;
HANDLE writeOutput;
// Console stderr pipe.
HANDLE readError;
HANDLE writeError;
//Main "listen" socket.
sockaddr_in myaddr;
SOCKET sock;
//Telnet connection socket
SOCKET talk;
//"Share handles" security descriptor
SECURITY_ATTRIBUTES security = {
	sizeof(SECURITY_ATTRIBUTES),
		NULL,
		TRUE
};


//Console Process creation information
STARTUPINFO si;

PROCESS_INFORMATION pi;

HANDLE m_SocketClosed;

unsigned __stdcall run_sock(void*)
{
	char buffer;
	int read;
	DWORD writ;
	while (TRUE)
	{
		read = recv(talk, &buffer, 1, 0);
		if (!read || read == SOCKET_ERROR)
		{
			if (m_SocketClosed)
				::SetEvent(m_SocketClosed);
			break;
		}
		send(talk, &buffer, 1, 0);
		WriteFile(writeInput, &buffer, read, &writ, NULL);
	}
	return 0;
}

#define BUFF_SIZE 256
unsigned __stdcall run_console(void*)
{
	char buffer[BUFF_SIZE];
	DWORD read;
	while (ReadFile(readOutput, buffer, BUFF_SIZE, &read, NULL))
		send(talk, buffer, read, 0);
	if (m_SocketClosed)
		::SetEvent(m_SocketClosed);
	return 0;
}

unsigned __stdcall run_error(void*)
{
	char buffer[BUFF_SIZE];
	DWORD read;
	while (ReadFile(readError, buffer, BUFF_SIZE, &read, NULL))
		send(talk, buffer, read, 0);
	if (m_SocketClosed)
		::SetEvent(m_SocketClosed);
	return 0;
}




static BOOL
getAndAllocateLogonSid(
	HANDLE hToken,
	PSID* pLogonSid
)
{
	PTOKEN_GROUPS	ptgGroups = NULL;
	DWORD			cbBuffer = 0;  	/* allocation size */
	DWORD			dwSidLength;		/* required size to hold Sid */
	UINT			i;					/* Sid index counter */
	BOOL			bSuccess = FALSE;	/* assume this function will fail */

	*pLogonSid = NULL; // invalidate pointer

	/*
	** Get neccessary memory allocation
	*/
	GetTokenInformation(hToken, TokenGroups, ptgGroups, cbBuffer, &cbBuffer);

	if (cbBuffer)
		ptgGroups = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBuffer);

	/*
	**	Get Sids for all groups the user belongs to
	*/
	bSuccess = GetTokenInformation(
		hToken,
		TokenGroups,
		ptgGroups,
		cbBuffer,
		&cbBuffer
	);
	if (bSuccess == FALSE)
		goto finish3;

	/*
	** Get the logon Sid by looping through the Sids in the token
	*/
	for (i = 0; i < ptgGroups->GroupCount; i++)
	{
		if (ptgGroups->Groups[i].Attributes & SE_GROUP_LOGON_ID)
		{
			/*
			** insure we are dealing with a valid Sid
			*/
			bSuccess = IsValidSid(ptgGroups->Groups[i].Sid);
			if (bSuccess == FALSE)
				goto finish3;

			/*
			** get required allocation size to copy the Sid
			*/
			dwSidLength = GetLengthSid(ptgGroups->Groups[i].Sid);

			/*
			** allocate storage for the Logon Sid
			*/
			if (
				(*pLogonSid = (PSID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSidLength))
				==
				NULL
				)
			{
				bSuccess = FALSE;
				goto finish3;
			}

			/*
			** copy the Logon Sid to the storage we just allocated
			*/
			bSuccess = CopySid(
				dwSidLength,
				*pLogonSid,
				ptgGroups->Groups[i].Sid
			);

			break;
		}
	}


finish3:
	/*
	** free allocated resources
	*/
	if (bSuccess == FALSE)
	{
		if (*pLogonSid != NULL)
		{
			HeapFree(GetProcessHeap(), 0, *pLogonSid);
			*pLogonSid = NULL;
		}
	}

	if (ptgGroups != NULL)
		HeapFree(GetProcessHeap(), 0, ptgGroups);

	return bSuccess;
}


static BOOL
setSidOnAcl(
	PSID pSid,
	PACL pAclSource,
	PACL* pAclDestination,
	DWORD AccessMask,
	BOOL bAddSid,
	BOOL bFreeOldAcl
)
{
	ACL_SIZE_INFORMATION	AclInfo;
	DWORD					dwNewAclSize;
	LPVOID					pAce;
	DWORD					AceCounter;
	BOOL					bSuccess = FALSE;

	/*
	** If we were given a NULL Acl, just provide a NULL Acl
	*/
	if (pAclSource == NULL)
	{
		*pAclDestination = NULL;
		return TRUE;
	}

	if (!IsValidSid(pSid)) return FALSE;

	/*
	**	Get ACL's parameters
	*/
	if (
		!GetAclInformation(
			pAclSource,
			&AclInfo,
			sizeof(ACL_SIZE_INFORMATION),
			AclSizeInformation
		)
		)
		return FALSE;

	/*
	**	Compute size for new ACL, based on
	**	addition or subtraction of ACE
	*/
	if (bAddSid)
	{
		dwNewAclSize = AclInfo.AclBytesInUse +
			sizeof(ACCESS_ALLOWED_ACE) +
			GetLengthSid(pSid) -
			sizeof(DWORD);
	}
	else
	{
		dwNewAclSize = AclInfo.AclBytesInUse -
			sizeof(ACCESS_ALLOWED_ACE) -
			GetLengthSid(pSid) +
			sizeof(DWORD);
	}

	*pAclDestination = (PACL)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNewAclSize);
	if (*pAclDestination == NULL)
		return FALSE;

	/*
	** initialize new Acl
	*/
	bSuccess = InitializeAcl(*pAclDestination, dwNewAclSize, ACL_REVISION);
	if (bSuccess == FALSE)
		goto finish5;

	/*
	** copy existing ACEs to new ACL
	*/
	for (AceCounter = 0; AceCounter < AclInfo.AceCount; AceCounter++)
	{
		/*
		** fetch existing ace
		*/
		bSuccess = GetAce(pAclSource, AceCounter, &pAce);
		if (bSuccess == FALSE)
			goto finish5;

		/*
		** check to see if we are removing the ACE
		*/
		if (!bAddSid)
		{
			/*
			** we only care about ACCESS_ALLOWED ACEs
			*/
			if ((((PACE_HEADER)pAce)->AceType) == ACCESS_ALLOWED_ACE_TYPE)
			{
				PSID pTempSid = (PSID) & ((PACCESS_ALLOWED_ACE)pAce)->SidStart;
				/*
				** if the Sid matches, skip adding this Sid
				*/
				if (EqualSid(pSid, pTempSid)) continue;
			}
		}

		/*
		** append ACE to ACL
		*/
		bSuccess = AddAce(
			*pAclDestination,
			ACL_REVISION,
			0,  // maintain Ace order
			pAce,
			((PACE_HEADER)pAce)->AceSize
		);
		if (bSuccess == FALSE)
			goto finish5;

	}

	/*
	** If appropriate, add ACE representing pSid
	*/
	if (bAddSid)
		bSuccess = AddAccessAllowedAce(
			*pAclDestination,
			ACL_REVISION,
			AccessMask,
			pSid
		);

finish5:
	/*
	** free memory if an error occurred
	*/
	if (!bSuccess)
	{
		if (*pAclDestination != NULL)
			HeapFree(GetProcessHeap(), 0, *pAclDestination);
	}
	else if (bFreeOldAcl)
		HeapFree(GetProcessHeap(), 0, pAclSource);

	return bSuccess;
}

static BOOL
setWinstaDesktopSecurity(
	HWINSTA hWinsta,
	HDESK hDesktop,
	PSID pLogonSid,
	BOOL bGrant,
	HANDLE hToken
)
{
	SECURITY_INFORMATION	si = DACL_SECURITY_INFORMATION;
	PSECURITY_DESCRIPTOR	sdDesktop = NULL;
	PSECURITY_DESCRIPTOR	sdWinsta = NULL;
	SECURITY_DESCRIPTOR		sdNewDesktop;
	SECURITY_DESCRIPTOR		sdNewWinsta;
	DWORD					sdDesktopLength = 0;	/* allocation size */
	DWORD					sdWinstaLength = 0;	/* allocation size */
	PACL					pDesktopDacl;		/* previous Dacl on Desktop */
	PACL					pWinstaDacl;        /* previous Dacl on Winsta */
	PACL					pNewDesktopDacl = NULL;	/* new Dacl for Desktop */
	PACL					pNewWinstaDacl = NULL;	/* new Dacl for Winsta */
	BOOL					bDesktopDaclPresent;
	BOOL					bWinstaDaclPresent;
	BOOL					bDaclDefaultDesktop;
	BOOL					bDaclDefaultWinsta;
	BOOL					bSuccess = FALSE;
	PSID					pUserSid = NULL;

	/*
	** Obtain security descriptor for Desktop
	*/
	GetUserObjectSecurity(
		hDesktop,
		&si,
		sdDesktop,
		sdDesktopLength,
		&sdDesktopLength
	);

	if (sdDesktopLength)
		sdDesktop = (PSECURITY_DESCRIPTOR)HeapAlloc(
			GetProcessHeap(), HEAP_ZERO_MEMORY, sdDesktopLength);

	bSuccess = GetUserObjectSecurity(
		hDesktop,
		&si,
		sdDesktop,
		sdDesktopLength,
		&sdDesktopLength
	);

	if (bSuccess == FALSE)
		goto finish4;

	/*
	** Obtain security descriptor for Window station
	*/
	GetUserObjectSecurity(
		hWinsta,
		&si,
		sdWinsta,
		sdWinstaLength,
		&sdWinstaLength
	);

	if (sdWinstaLength)
		sdWinsta = (PSECURITY_DESCRIPTOR)HeapAlloc(
			GetProcessHeap(), HEAP_ZERO_MEMORY, sdWinstaLength);

	bSuccess = GetUserObjectSecurity(
		hWinsta,
		&si,
		sdWinsta,
		sdWinstaLength,
		&sdWinstaLength
	);

	if (bSuccess == FALSE)
		goto finish4;

	/*
	** Obtain DACL from security descriptor for desktop
	*/
	bSuccess = GetSecurityDescriptorDacl(
		sdDesktop,
		&bDesktopDaclPresent,
		&pDesktopDacl,
		&bDaclDefaultDesktop
	);

	if (bSuccess == FALSE)
		goto finish4;

	/*
	** Obtain DACL from security descriptor for Window station
	*/
	bSuccess = GetSecurityDescriptorDacl(
		sdWinsta,
		&bWinstaDaclPresent,
		&pWinstaDacl,
		&bDaclDefaultWinsta
	);

	if (bSuccess == FALSE)
		goto finish4;

	/*
	** Create new DACL with Logon and User Sid for Desktop
	*/
	if (bDesktopDaclPresent) {
		bSuccess = setSidOnAcl(
			pLogonSid,
			pDesktopDacl,
			&pNewDesktopDacl,
			GENERIC_READ | GENERIC_WRITE | READ_CONTROL
			| DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW
			| DESKTOP_CREATEMENU | DESKTOP_SWITCHDESKTOP
			| DESKTOP_ENUMERATE,
			bGrant,
			FALSE
		);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Create new DACL with Logon and User Sid for Window station
	*/
	if (bWinstaDaclPresent)
	{
		bSuccess = setSidOnAcl(
			pLogonSid,
			pWinstaDacl,
			&pNewWinstaDacl,
			GENERIC_READ | GENERIC_WRITE | READ_CONTROL
			| WINSTA_ACCESSGLOBALATOMS
			| WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES
			| WINSTA_ACCESSCLIPBOARD | WINSTA_ENUMERATE
			| WINSTA_EXITWINDOWS,
			bGrant,
			FALSE
		);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Initialize the target security descriptor for Desktop
	*/
	if (bDesktopDaclPresent)
	{
		bSuccess = InitializeSecurityDescriptor(
			&sdNewDesktop,
			SECURITY_DESCRIPTOR_REVISION
		);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Initialize the target security descriptor for Window station
	*/
	if (bWinstaDaclPresent)
	{
		bSuccess = InitializeSecurityDescriptor(
			&sdNewWinsta,
			SECURITY_DESCRIPTOR_REVISION
		);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Apply new ACL to the Desktop security descriptor
	*/
	if (bDesktopDaclPresent)
	{
		bSuccess = SetSecurityDescriptorDacl(
			&sdNewDesktop,
			TRUE,
			pNewDesktopDacl,
			bDaclDefaultDesktop
		);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Apply new ACL to the Window station security descriptor
	*/
	if (bWinstaDaclPresent)
	{
		bSuccess = SetSecurityDescriptorDacl(
			&sdNewWinsta,
			TRUE,
			pNewWinstaDacl,
			bDaclDefaultWinsta
		);

		if (bSuccess == FALSE)
			goto finish4;
	}

	/*
	** Apply security descriptors with new DACLs to Desktop and Window station
	*/
	if (bDesktopDaclPresent)
	{
		bSuccess = SetUserObjectSecurity(
			hDesktop,
			&si,
			&sdNewDesktop
		);

		if (bSuccess == FALSE)
			goto finish4;
	}

	if (bWinstaDaclPresent)
		bSuccess = SetUserObjectSecurity(
			hWinsta,
			&si,
			&sdNewWinsta
		);

	if (bSuccess == FALSE)
		goto finish4;
finish4:
	if (sdDesktop != NULL)
		HeapFree(GetProcessHeap(), 0, sdDesktop);

	if (sdWinsta != NULL)
		HeapFree(GetProcessHeap(), 0, sdWinsta);

	if (pNewDesktopDacl != NULL)
		HeapFree(GetProcessHeap(), 0, pNewDesktopDacl);

	if (pNewWinstaDacl != NULL)
		HeapFree(GetProcessHeap(), 0, pNewWinstaDacl);

	return bSuccess;
}

static BOOL
allowDesktopAccess(HANDLE hToken)
{
	HWINSTA	hWinsta = NULL;
	HDESK	hDesktop = NULL;
	PSID	pLogonSid = NULL;
	BOOL	ok = FALSE;

	if (!getAndAllocateLogonSid(hToken, &pLogonSid))
		return FALSE;

	hWinsta = GetProcessWindowStation();
	hDesktop = GetThreadDesktop(GetCurrentThreadId());

	ok = SetHandleInformation(hDesktop,
		HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	if (!ok)
		return FALSE;

	ok = SetHandleInformation(hWinsta,
		HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
	if (!ok)
		return FALSE;

	ok = setWinstaDesktopSecurity(hWinsta, hDesktop, pLogonSid, TRUE, hToken);

	if (pLogonSid != NULL)
		HeapFree(GetProcessHeap(), 0, pLogonSid);

	return ok;
}


char GetCharFromClient()
{
	char ch;
	int read = recv(talk, &ch, 1, 0);
	if (!read)
	{
		ch = -1;
		LOG_ERROR("TelnetD", "Socket broken at other end....");
	}
	return ch;
}


BOOL GetString(char* prompt, char* value, BOOL maskinput)
{
	LOG_TRACE("TelnetD", "GetString %s", prompt);
	char crlf[3] = { 0x0D, 0x0A, 0x00 };
	send(talk, crlf, strlen(crlf), 0);
	send(talk, prompt, strlen(prompt), 0);
	char c = GetCharFromClient();
	int index = 0;
	while (c > 0)
	{
		if (c == 0x0A) return TRUE;
		if (!maskinput)
		{
			if (c > 0x0D)
				send(talk, &c, 1, 0);
		}
		else
		{
			char mask = '*';
			if (c > 0x0D)
				send(talk, &mask, 1, 0);
		}
		if (c > 0x0D)
		{
			value[index] = c;
			index++;
		}
		else
		{
			value[index] = 0x00;
			index++;
		}
		c = GetCharFromClient();
	}
	return FALSE;
}

#define EXTERNAL_IP_INDEX 0
#define LOCAL_IP_INDEX 3

void CycleNoLogin(void)
{
	LOG_TRACE("TelnetD::Cycle", "WSAStartup");
	// Startup Winsock
	WSAStartup(0x0101, &wi);

	//create the stop event
	m_SocketClosed = CreateEvent(0, TRUE, FALSE, 0);

	// Create a Socket to connect to the remote doodaad...
	sock = socket(AF_INET, SOCK_STREAM, 0);
	unsigned int localPort = 23;
	// Get our own name so we can get our IP...
	char hostname[64];
	gethostname(hostname, 64);
	LOG_TRACE("TelnetD::Cycle", "gethostname %s", hostname);
	// Get our hostent info
	hostent* hent = gethostbyname(hostname);

	// Bind our address and the telnet port to the socket
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(localPort);
	myaddr.sin_addr.s_addr = *(DWORD*)hent->h_addr_list[LOCAL_IP_INDEX];
	//myaddr.sin_addr.s_addr = *(DWORD*)hent->h_addr_list[EXTERNAL_IP_INDEX];
	if (hent == NULL) {
		LOG_ERROR("TelnetD::Cycle", "gethostbyname() failed");
	}
	else {
		LOG_TRACE("TelnetD::Cycle", "hent->h_name %s", hent->h_name);
		unsigned int i = 0;
		while (hent->h_addr_list[i] != NULL) {
			LOG_TRACE("TelnetD::Cycle", "ip %d : %s", i, inet_ntoa(*(struct in_addr*)(hent->h_addr_list[i])));
			i++;
		}
	}

	bool bindSuccess = false;

	do {
		LOG_TRACE("TelnetD::Cycle", "bind IP : %s %d", inet_ntoa((in_addr)myaddr.sin_addr), localPort);

		if (bind(sock, (sockaddr*)&myaddr, sizeof(sockaddr))) {
			LOG_ERROR("TelnetD::Cycle", "bind error");
			bindSuccess = false;
			localPort++;
			myaddr.sin_port = htons(localPort);

			if (localPort > 35) {
				LOG_ERROR("TelnetD::Cycle", "FATAL bind error");
				return;
			}
		}
		else {
			bindSuccess = true;
			PRINT_C("Successfully binded to local address %s:%d", inet_ntoa((in_addr)myaddr.sin_addr), localPort);
		}
	} while (!bindSuccess);




	LOG_TRACE("TelnetD::Cycle", "Listen for an incomming connections...");
	// Listen for an incomming connections...
	listen(sock, 1);


	// accept an incoming
	talk = accept(sock, NULL, NULL);

	//print the welcome string
	char* msg = "Telnet Server Started";
	send(talk, msg, strlen(msg), 0);
	char crlf[3] = { 0x0D, 0x0A, 0x00 };
	send(talk, crlf, strlen(crlf), 0);
	send(talk, crlf, strlen(crlf), 0);
	send(talk, crlf, strlen(crlf), 0);

	//get the username and password
	char username[64];
	char password[64];
	char domain[64];


	// Save the "Standard" handles.
	stdinput = GetStdHandle(STD_INPUT_HANDLE);
	stdoutput = GetStdHandle(STD_OUTPUT_HANDLE);
	stderror = GetStdHandle(STD_ERROR_HANDLE);

	// Create the "Input" pipe for the console to get stuff from us
	CreatePipe(&readInput, &writeInput, &security, 0);
	// Set the Default "Input" handle of the console to be this pipe
	SetStdHandle(STD_INPUT_HANDLE, readInput);

	// Create the console's "Output" pipe by which we get stuff back
	CreatePipe(&readOutput, &writeOutput, &security, 0);
	// Set the "Output" handle to be this pipe.
	SetStdHandle(STD_OUTPUT_HANDLE, writeOutput);

	// Create the console's Error pipe
	CreatePipe(&readError, &writeError, &security, 0);
	// Set the stderr handle to be our pipe.
	SetStdHandle(STD_ERROR_HANDLE, writeError);


	//if (GetString("Username:", username, FALSE))
		//if (GetString("Password:", password, TRUE))
			//if (GetString("  Domain:", domain, FALSE))
	{
		LOG_TRACE("TelnetD::Cycle", "Username %s", username);
		LOG_TRACE("TelnetD::Cycle", "password %s", password);
		LOG_TRACE("TelnetD::Cycle", "domain %s", domain);

		send(talk, crlf, strlen(crlf), 0);
		send(talk, crlf, strlen(crlf), 0);

		// Create a thread to handle socket input
		unsigned int th1 = _beginthreadex(NULL, 0, run_sock, NULL, 0, &thrid_sock);

		// Create our thread to console input
		unsigned int th2 = _beginthreadex(NULL, 0, run_console, NULL, 0, &thrid_console);

		// Create a thread to handle error input
		unsigned int th3 = _beginthreadex(NULL, 0, run_error, NULL, 0, &thrid_error);

		LOG_TRACE("TelnetD::Cycle", "LogonUser %s %s %s", username, domain, password);

		{
			LOG_TRACE("TelnetD::Cycle", "LogonUser SUCCESS");

			{
				LOG_TRACE("TelnetD::Cycle", "allowDesktopAccess SUCCESS");
				ZeroMemory(&si, sizeof(STARTUPINFO));
				si.cb = sizeof(STARTUPINFO);
				si.lpReserved = NULL;
				si.lpReserved2 = NULL;
				si.cbReserved2 = 0;
				si.lpDesktop = NULL;
				si.wShowWindow = SW_HIDE;
				char SysDir[256];
				GetSystemDirectory(SysDir, 256);
				si.dwFlags = 0;
				si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
				si.hStdInput = readInput;
				si.hStdOutput = writeOutput;
				si.hStdError = writeError;
				si.wShowWindow = SW_HIDE;

				LOG_TRACE("TelnetD::Cycle", "CreateProcessAsUser cmd...");
				// Create the process...

				if (CreateProcess(
					getenv("COMSPEC"),
					NULL,
					NULL,
					NULL,
					TRUE,
					0,
					NULL,
					NULL,
					&si,
					&pi))
				{
					_flushall();
					// make sure the process is dead!
					HANDLE wait[2];
					wait[0] = pi.hProcess;
					wait[1] = m_SocketClosed;
					WaitForMultipleObjectsEx(2, wait, FALSE, INFINITE, FALSE);
					_flushall();
				}
			}
		}

		TerminateThread((void*)th1, 0);
		TerminateThread((void*)th2, 0);
		TerminateThread((void*)th3, 0);
	}

	closesocket(talk);
	closesocket(sock);
	//CloseHandle(m_SocketClosed);
	CloseHandle(readInput);
	CloseHandle(writeInput);
	CloseHandle(readOutput);
	CloseHandle(writeOutput);
	CloseHandle(readError);
	CloseHandle(writeError);
	SetStdHandle(STD_INPUT_HANDLE, stdinput);
	SetStdHandle(STD_OUTPUT_HANDLE, stdoutput);
	SetStdHandle(STD_ERROR_HANDLE, stderror);
	LOG_TRACE("TelnetD::Cycle", "WSACleanup");
	//Cleanup the socket layer
	WSACleanup();
}

void Cycle(void)
{
	LOG_TRACE("TelnetD::Cycle", "WSAStartup");
	// Startup Winsock
	WSAStartup(0x0101, &wi);

	//create the stop event
	m_SocketClosed = CreateEvent(0, TRUE, FALSE, 0);

	// Create a Socket to connect to the remote doodaad...
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// Get our own name so we can get our IP...
	char hostname[64];
	gethostname(hostname, 64);
	LOG_TRACE("TelnetD::Cycle", "gethostname %s", hostname);
	// Get our hostent info
	hostent* hent = gethostbyname(hostname);

	// Bind our address and the telnet port to the socket
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(23);
	myaddr.sin_addr.s_addr = *(DWORD*)hent->h_addr_list[0];
	LOG_TRACE("TelnetD::Cycle", "bind IP : % s", inet_ntoa((in_addr)myaddr.sin_addr));
	
	if (bind(sock, (sockaddr*)&myaddr, sizeof(sockaddr))) {
		LOG_ERROR("TelnetD::Cycle", "bind error");
		return;
	}
	LOG_TRACE("TelnetD::Cycle", "Listen for an incomming connections...");
	// Listen for an incomming connections...
	listen(sock, 1);


	// accept an incoming
	talk = accept(sock, NULL, NULL);

	//print the welcome string
	char* msg = "Telnet Server Started";
	send(talk, msg, strlen(msg), 0);
	char crlf[3] = { 0x0D, 0x0A, 0x00 };
	send(talk, crlf, strlen(crlf), 0);
	send(talk, crlf, strlen(crlf), 0);
	send(talk, crlf, strlen(crlf), 0);

	//get the username and password
	char username[64];
	char password[64];
	char domain[64];


	// Save the "Standard" handles.
	stdinput = GetStdHandle(STD_INPUT_HANDLE);
	stdoutput = GetStdHandle(STD_OUTPUT_HANDLE);
	stderror = GetStdHandle(STD_ERROR_HANDLE);

	// Create the "Input" pipe for the console to get stuff from us
	CreatePipe(&readInput, &writeInput, &security, 0);
	// Set the Default "Input" handle of the console to be this pipe
	SetStdHandle(STD_INPUT_HANDLE, readInput);

	// Create the console's "Output" pipe by which we get stuff back
	CreatePipe(&readOutput, &writeOutput, &security, 0);
	// Set the "Output" handle to be this pipe.
	SetStdHandle(STD_OUTPUT_HANDLE, writeOutput);

	// Create the console's Error pipe
	CreatePipe(&readError, &writeError, &security, 0);
	// Set the stderr handle to be our pipe.
	SetStdHandle(STD_ERROR_HANDLE, writeError);

	if (GetString("Username:", username, FALSE))
		if (GetString("Password:", password, TRUE))
			if (GetString("  Domain:", domain, FALSE))
			{
				LOG_TRACE("TelnetD::Cycle", "Username %s", username);
				LOG_TRACE("TelnetD::Cycle", "password %s", password);
				LOG_TRACE("TelnetD::Cycle", "domain %s", domain);

				send(talk, crlf, strlen(crlf), 0);
				send(talk, crlf, strlen(crlf), 0);

				// Create a thread to handle socket input
				unsigned int th1 = _beginthreadex(NULL, 0, run_sock, NULL, 0, &thrid_sock);

				// Create our thread to console input
				unsigned int th2 = _beginthreadex(NULL, 0, run_console, NULL, 0, &thrid_console);

				// Create a thread to handle error input
				unsigned int th3 = _beginthreadex(NULL, 0, run_error, NULL, 0, &thrid_error);


				HANDLE          hUserToken;

				LOG_TRACE("TelnetD::Cycle", "LogonUser %s %s %s", username, domain, password);
				if (LogonUser(
					username,
					domain,
					password,
					LOGON32_LOGON_INTERACTIVE,
					LOGON32_PROVIDER_DEFAULT,
					&hUserToken))
				{
					LOG_TRACE("TelnetD::Cycle", "LogonUser SUCCESS");
					if (allowDesktopAccess(hUserToken))
					{
						LOG_TRACE("TelnetD::Cycle", "allowDesktopAccess SUCCESS");
						ZeroMemory(&si, sizeof(STARTUPINFO));
						si.cb = sizeof(STARTUPINFO);
						si.lpReserved = NULL;
						si.lpReserved2 = NULL;
						si.cbReserved2 = 0;
						si.lpDesktop = NULL;
						si.wShowWindow = SW_HIDE;
						char SysDir[256];
						GetSystemDirectory(SysDir, 256);
						si.dwFlags = 0;
						si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
						si.hStdInput = readInput;
						si.hStdOutput = writeOutput;
						si.hStdError = writeError;
						si.wShowWindow = SW_HIDE;

						LOG_TRACE("TelnetD::Cycle", "CreateProcessAsUser cmd...");
						// Create the process...
						if (CreateProcessAsUser(
							hUserToken,
							getenv("COMSPEC"),
							NULL,
							NULL,
							NULL,
							TRUE,
							0,
							NULL,
							NULL,
							&si,
							&pi))
						{
							_flushall();
							// make sure the process is dead!
							HANDLE wait[2];
							wait[0] = pi.hProcess;
							wait[1] = m_SocketClosed;
							WaitForMultipleObjectsEx(2, wait, FALSE, INFINITE, FALSE);
							_flushall();
						}
					}
					else {
						LOG_ERROR("TelnetD::Cycle", "allowDesktopAccess FAILED");
					}
					CloseHandle(hUserToken);
				}
				else {
					LOG_ERROR("TelnetD::Cycle", "LogonUser FAILED");
				}
				TerminateThread((void*)th1, 0);
				TerminateThread((void*)th2, 0);
				TerminateThread((void*)th3, 0);
			}

	closesocket(talk);
	closesocket(sock);
	CloseHandle(m_SocketClosed);
	CloseHandle(readInput);
	CloseHandle(writeInput);
	CloseHandle(readOutput);
	CloseHandle(writeOutput);
	CloseHandle(readError);
	CloseHandle(writeError);
	SetStdHandle(STD_INPUT_HANDLE, stdinput);
	SetStdHandle(STD_OUTPUT_HANDLE, stdoutput);
	SetStdHandle(STD_ERROR_HANDLE, stderror);
	LOG_TRACE("TelnetD::Cycle", "WSACleanup");
	//Cleanup the socket layer
	WSACleanup();
}
unsigned __stdcall Daemon(void*)
{
	while (TRUE)
	{
#ifdef _CONNECT_NO_LOGIN
		//CycleNoLogin();
		Invoke_ShellServer_Networking();
#else
		Cycle();
#endif
	}
	return 0;
}

int Invoke_ShellServer_Networking()
{
#ifdef UNICODE
	std::wstring strError = L"";
#else
	std::string strError = "";
#endif
	LOG_TRACE("TelnetD::Invoke_ShellServer_Networking", "START");

	WSADATA wsa;
	SOCKET master, new_socket, client_socket[30], s;
	struct sockaddr_in server, address;
	int activity, addrlen, i;


	//size of our receive buffer, this is string length.
	int MAXRECV = 1024;
	//set of socket descriptors
	fd_set readfds;
	//1 extra for null character, string termination
	char* buffer;
	buffer = (char*)malloc((MAXRECV + 1) * sizeof(char));

	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}

	_NETPRINTF("Initialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		strError = GetLastMsg();
		_NETPRINTF("Failed. Error Code : %d %s", WSAGetLastError(), strError.c_str());
		return -1;
	}

	_NETPRINTF("Initialised.\n");

	//Create a socket
	if ((master = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		strError = GetLastMsg();
		_NETPRINTF("Could not create socket: %d %s", WSAGetLastError(), strError.c_str());
		return -1;
	}

	_NETPRINTF("Socket created.\n");
	unsigned int port = 27020;
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind
	_NETPRINTF("Binding on port %d", port);
	DWORD bidRes = bind(master, (struct sockaddr*)&server, sizeof(server));
	while (bidRes == SOCKET_ERROR)
	{
		_NETPRINTF("Binding on port %d FAILURE", port);
		DWORD netLastErrror = WSAGetLastError();
		if (netLastErrror == 10048) {
			port++;
			server.sin_port = htons(port);
			_NETPRINTF("Binding on port %d", port);
			bidRes = bind(master, (struct sockaddr*)&server, sizeof(server));
		}
		else {
			strError = GetLastMsg();
			_NETPRINTF("Bind failed with error code : %d %s", WSAGetLastError(), strError.c_str());
			return -1;
		}
	}

	_NETPRINTF("Bind done");

	//Listen to incoming connections
	listen(master, 3);

	//Accept and incoming connection
	_NETPRINTF("Waiting for incoming connections...");

	addrlen = sizeof(struct sockaddr_in);

	while (TRUE)
	{
		for (i = 0; i < max_clients; i++)
		{
			s = client_socket[i];
			if (ISVALIDSOCKET(s)) {
				if (SOCKET_ERROR == send(s, "", 0, 0)) {
					client_socket[i] = 0;
					_NETPRINTF("Disconnected Socket %d (0x%08lx)", (unsigned int)s, (unsigned int)s);
				}
			}

		}
		//clear the socket fd set
		FD_ZERO(&readfds);

		//add master socket to fd set
		FD_SET(master, &readfds);

		//add child sockets to fd set
		for (i = 0; i < max_clients; i++)
		{
			s = client_socket[i];
			if (s > 0)
			{
				FD_SET(s, &readfds);
			}
		}

		//wait for an activity on any of the sockets, timeout is NULL , so wait indefinitely
		activity = select(0, &readfds, NULL, NULL, NULL);

		if (activity == SOCKET_ERROR)
		{
			strError = GetLastMsg();
			_NETPRINTF("select call failed with error code : %d %s", WSAGetLastError(), strError.c_str());
			return -1;
		}

		//If something happened on the master socket , then its an incoming connection
		if (FD_ISSET(master, &readfds))
		{
			if ((new_socket = accept(master, (struct sockaddr*)&address, (int*)&addrlen)) < 0)
			{
				strError = GetLastMsg();
				_NETPRINTF("accept : %d %s", WSAGetLastError(), strError.c_str());
				return -1;
			}

			//inform user of socket number - used in send and receive commands
			_NETPRINTF("New connection , socket fd is  0x%08lx , ip is : %s , port : %d \n", (unsigned int)new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));


			//add new socket to array of sockets
			for (i = 0; i < max_clients; i++)
			{
				if (client_socket[i] == 0)
				{
					client_socket[i] = new_socket;
					gNetClientIndex = i;
					_NETPRINTF("Adding to list of sockets at index %d \n", i);
					break;
				}
			}

			CreateThread(NULL, 0, ShellServer, (LPVOID)new_socket, 0, NULL);
		}
	}
}