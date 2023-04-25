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

		virtual void	Pause() = 0;
		virtual void	Continue() = 0;
		virtual void	Shutdown() = 0;

		virtual BOOL	InstallService();
		virtual BOOL	RemoveService();
		virtual BOOL	EndService();
		virtual BOOL	StartupService();


		virtual STD_STRING CurrentStateString() = 0;
		virtual bool IsRunning() = 0;
		virtual bool StopReceived() = 0;
		virtual bool PauseReceived() = 0;
		virtual bool ContinueReceived() = 0;
		virtual bool IsDebugging() = 0;

	protected:	// data members

		virtual void	AddToMessageLog(const char* Message, WORD EventType = EVENTLOG_ERROR_TYPE, DWORD dwEventID = DWORD(-1));
		virtual void	RegisterApplicationLog(LPCTSTR lpszProposedMessageFile, DWORD dwProposedTypes);
		virtual void	DeregisterApplicationLog();

	protected:	// data members
		LPCTSTR					m_lpServiceName;
		LPCTSTR					m_lpDisplayName;

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
		bool			m_fConsoleReady;
	protected:	// helpers
		void			SetupConsole();
		LPTSTR			GetLastErrorText(LPTSTR Buf, DWORD Size);
		BOOL			ReportStatus(DWORD CurState, DWORD WaitHint = 3000, DWORD ErrExit = 0);

};


// Retrieve the one and only CNTService object:
BaseService * AfxGetService();


#endif	// __BASE_SERVICE_H__
