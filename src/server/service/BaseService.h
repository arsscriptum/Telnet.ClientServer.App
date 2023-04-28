/*
 * ╓──────────────────────────────────────────────────────────────────────────────────────╖
 * ║                                                                                      ║
 * ║   Covert Command and Control Service                                                 ║
 * ║   (integrated command and control program delivered covertly)                        ║
 * ║                                                                                      ║
 * ║   Copyright (c) 2017, cybercastor    <cybercastor@icloud.com>                        ║
 * ║   All rights reserved.                                                               ║
 * ║                                                                                      ║
 * ║   Version 1.0.0                                                                      ║
 * ║   https://cybercastor.github.io/projects/cccservice.html                             ║
 * ║                                                                                      ║
 * ╙──────────────────────────────────────────────────────────────────────────────────────╜
 */


#ifndef __BASE_SERVICE_H__
#define __BASE_SERVICE_H__


class BaseService {

	public:		
		BaseService(LPCSTR ServiceName, LPCSTR DisplayName, LPCSTR szAccountName, LPCSTR szPassword);
		~BaseService();

	private:	
		BaseService( const BaseService & );
		BaseService & operator=( const BaseService & );

	public:		
			
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
		virtual bool IsInteractive() = 0;

	protected:	

		virtual void	AddToMessageLog(const char* Message, WORD EventType = EVENTLOG_ERROR_TYPE, DWORD dwEventID = DWORD(-1));
		virtual void	RegisterApplicationLog(LPCTSTR lpszProposedMessageFile, DWORD dwProposedTypes);
		virtual void	DeregisterApplicationLog();

		void			SetupConsole();
		LPTSTR			GetLastErrorText(LPTSTR Buf, DWORD Size);

	protected:	
		LPCTSTR					m_lpServiceName;
		LPCTSTR					m_lpDisplayName;
		bool					m_bInteractiveMode;
		bool					m_fConsoleReady;



};


// Retrieve the one and only CNTService object:
BaseService * AfxGetService();


#endif	// __BASE_SERVICE_H__
