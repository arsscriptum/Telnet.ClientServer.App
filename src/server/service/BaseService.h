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
 * ╟──────────────────────────────────────────────────────────────────────────────────────╢
 * ║                                                                                      ║
 * ║   Redistribution and use in source and binary forms, with or without                 ║
 * ║   modification, are permitted provided that the following conditions are met:        ║
 * ║                                                                                      ║
 * ║   * Redistributions of source code must retain the above copyright notice, this      ║
 * ║     list of conditions and the following disclaimer.                                 ║
 * ║                                                                                      ║
 * ║   * Redistributions in binary form must reproduce the above copyright notice, this   ║
 * ║     list of conditions and the following disclaimer in the documentation and/or      ║
 * ║     other materials provided with the distribution.                                  ║
 * ║                                                                                      ║
 * ║   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND    ║
 * ║   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED      ║
 * ║   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE             ║
 * ║   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR   ║
 * ║   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES     ║
 * ║   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;       ║
 * ║   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON     ║
 * ║   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT            ║
 * ║   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS      ║
 * ║   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                       ║
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
		virtual bool IsInteractive() = 0;

	protected:	

		virtual void	AddToMessageLog(const char* Message, WORD EventType = EVENTLOG_ERROR_TYPE, DWORD dwEventID = DWORD(-1));
		virtual void	RegisterApplicationLog(LPCTSTR lpszProposedMessageFile, DWORD dwProposedTypes);
		virtual void	DeregisterApplicationLog();

		void			SetupConsole();
		LPTSTR			GetLastErrorText(LPTSTR Buf, DWORD Size);

	protected:	// data members
		LPCTSTR					m_lpServiceName;
		LPCTSTR					m_lpDisplayName;

		bool					m_bInteractiveMode;
		bool					m_fConsoleReady;



};


// Retrieve the one and only CNTService object:
BaseService * AfxGetService();


#endif	// __BASE_SERVICE_H__
