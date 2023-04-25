/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1997 by Joerg Koenig
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


// last revised: $Date: 5.05.98 21:22 $, $Revision: 2 $


#ifndef __BASE_SERVICE_H__
#define __BASE_SERVICE_H__


class BaseService {
	static BOOL				m_bInstance;			// only one CNTService object per application
	
	public:		// construction/destruction
			
		BaseService(LPCSTR ServiceName, LPCSTR DisplayName, LPCSTR szAccountName, LPCSTR szPassword);
		~BaseService();

	private:	// forbidden functions
		BaseService( const BaseService & );
		BaseService & operator=( const BaseService & );

	public:		// overridables
			
		virtual void	Start() = 0;
		virtual void	Stop() = 0;

		virtual void	Pause();
		virtual void	Continue();
		virtual void	Shutdown();
		virtual BOOL	RegisterService();
		virtual BOOL	StartDispatcher();
		virtual BOOL	InstallService();
		virtual BOOL	RemoveService();
		virtual BOOL	EndService();
		virtual BOOL	StartupService();
		virtual BOOL	DebugService(int argc, char **argv);	//!! TCW MOD - added FACELESS parm to allow Win95 usage.

		static void WINAPI	ServiceCtrl(DWORD CtrlCode);
		static void WINAPI	ServiceMain(DWORD argc, LPTSTR* argv);
		static BOOL WINAPI	ControlHandler(DWORD CtrlType);

		bool IsRunning() { return (m_ssStatus.dwCurrentState == SERVICE_RUNNING); }
		bool StopReceived() { return (m_ssStatus.dwCurrentState == SERVICE_STOP_PENDING); }
		bool PauseReceived() { return (m_ssStatus.dwCurrentState == SERVICE_PAUSE_PENDING); }
		bool ContinueReceived() { return (m_ssStatus.dwCurrentState == SERVICE_CONTINUE_PENDING); }
		bool IsDebugging() { return (IsRunning() && m_Debugging); };

	protected:	// data members

		virtual void	AddToMessageLog(const char* Message, WORD EventType = EVENTLOG_ERROR_TYPE, DWORD dwEventID = DWORD(-1));
		virtual void	RegisterApplicationLog(LPCTSTR lpszProposedMessageFile, DWORD dwProposedTypes);
		virtual void	DeregisterApplicationLog();

	protected:	// data members
		LPCTSTR					m_lpServiceName;
		LPCTSTR					m_lpDisplayName;
		DWORD					m_dwCheckPoint;
		DWORD					m_dwErr;
		BOOL					m_bDebug;			// TRUE if -d was passed to the program
		SERVICE_STATUS			m_ssStatus;			// current status of the service
		SERVICE_STATUS_HANDLE	m_sshStatusHandle;
		DWORD					m_dwControlsAccepted;	// bit-field of what control requests the
														// service will accept
														// (dflt: SERVICE_ACCEPT_STOP)
		PSID					m_pUserSID;			// the current user's security identifier
		
		BOOL					m_fConsoleReady;

		// parameters to the "CreateService()" function:
		DWORD			m_dwDesiredAccess;		// default: SERVICE_ALL_ACCESS
		DWORD			m_dwServiceType;		// default: SERVICE_WIN32_OWN_PROCESS
		DWORD			m_dwStartType;			// default: SERVICE_AUTO_START
		DWORD			m_dwErrorControl;		// default: SERVICE_ERROR_NORMAL
		LPCTSTR			m_pszLoadOrderGroup;	// default: NULL
		DWORD			m_dwTagID;				// retrieves the tag identifier
		LPCTSTR			m_pszDependencies;		// default: NULL
		LPCTSTR			m_pszAccountName;
		LPCTSTR			m_pszPassword;			// default: NULL
		bool			m_Debugging;
	protected:	// helpers
		void			SetupConsole();
		LPTSTR			GetLastErrorText(LPTSTR Buf, DWORD Size);
		BOOL			ReportStatus(DWORD CurState, DWORD WaitHint = 3000, DWORD ErrExit = 0);

};


// Retrieve the one and only CNTService object:
BaseService * AfxGetService();


#endif	// __BASE_SERVICE_H__
