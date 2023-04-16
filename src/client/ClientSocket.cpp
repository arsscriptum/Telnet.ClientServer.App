// ClientSocket.cpp : implementation file
//

#include "stdafx.h"
#include "CTelnet.h"
#include "ClientSocket.h"

#include "CTelnetView.h"
#include "CTelnetDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientSocket

CClientSocket::CClientSocket(CTelnetView * pView)
{
	LOG_TRACE("CClientSocket::CClientSocket", "CONSTRUCTOR");
	cView = pView;
}

CClientSocket::~CClientSocket()
{
	LOG_TRACE("CClientSocket::~CClientSocket()", "DESTRUCTOR");
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CClientSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CClientSocket member functions

void CClientSocket::OnClose(int nErrorCode) 
{
//	AfxMessageBox("Connection Closed",MB_OK);
	LOG_TRACE("CClientSocket::OnClose", "OnClose %d ", nErrorCode);
	CAsyncSocket::OnClose(nErrorCode);
	if(!IsWindow(cView->m_hWnd)) return;
	if(!IsWindowVisible(cView->m_hWnd)) return;
	cView->GetDocument()->OnCloseDocument();

}

void CClientSocket::OnConnect(int nErrorCode) 
{
	LOG_TRACE("CClientSocket::OnConnect", "OnConnect %d ", nErrorCode);
	CAsyncSocket::OnConnect(nErrorCode);
}

void CClientSocket::OnOutOfBandData(int nErrorCode) 
{
	MY_ASSERT(FALSE); //Telnet should not have OOB data
	CAsyncSocket::OnOutOfBandData(nErrorCode);
}

void CClientSocket::OnReceive(int nErrorCode) 
{
	LOG_TRACE("CClientSocket::OnReceive", "OnReceive %d ", nErrorCode);
	cView->ProcessMessage(this);
}

void CClientSocket::OnSend(int nErrorCode) 
{
	LOG_TRACE("CClientSocket::OnSend", "OnSend %d ", nErrorCode);
	CAsyncSocket::OnSend(nErrorCode);
}
