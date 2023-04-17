// CTelnetView.cpp : implementation of the CTelnetView class
//

#include "stdafx.h"
#include "CTelnet.h"

#include "CTelnetDoc.h"
#include "CTelnetView.h"
#include "MainFrm.h"
#include "ClientSocket.h"
#include "Process.h"

#include "HostDialog.h"
#include "ScopedLogger.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMultiDocTemplate * pDocTemplate;


/////////////////////////////////////////////////////////////////////////////
// CTelnetView

IMPLEMENT_DYNCREATE(CTelnetView, CScrollView)

BEGIN_MESSAGE_MAP(CTelnetView, CScrollView)
	//{{AFX_MSG_MAP(CTelnetView)
	ON_WM_CHAR()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTelnetView construction/destruction

CTelnetView::CTelnetView()
{
	LOG_TRACE("CTelnetView::CTelnetView", "CONSTRUCTOR");
	memset(&m_bBuf, 0, ioBuffSize);
	bNegotiating = FALSE;
	cTextColor = RGB(200,200,000);
	cBackgroundColor = RGB(000,000,222);
	cSock = NULL;
	bOptionsSent = FALSE;
	TempCounter = 0;
	processedLines = 0;
	cCursX = 0;
	for(int x = 0; x < 80; x++)
	{
		for(int y = 0; y < bufferLines; y++)
		{
			cText[x][y] = ' ';
		}
	}

	hashKeyCodes.InitializeKeyCodes();
#ifdef _DEBUG
	hashKeyCodes.Test();
#endif
	
}

CTelnetView::~CTelnetView()
{
	LOG_TRACE("CTelnetView::~CTelnetView", "DESTRUCTOR");
}

BOOL WINAPI CTelnetView::ControlHandler(DWORD dwCtrlType) {

	switch (dwCtrlType) {
	case CTRL_BREAK_EVENT:  // use Ctrl+C or Ctrl+Break to simulate
		LOG_TRACE("CTelnetView::ControlHandler", "RECEIVED CTRL_BREAK_EVENT");
		return TRUE;
	case CTRL_C_EVENT:      // SERVICE_CONTROL_STOP in debug mode
		LOG_TRACE("CTelnetView::ControlHandler", "RECEIVED CTRL_C_EVENT");
		return TRUE;
	}
	return FALSE;
}

BOOL CTelnetView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CTelnetView drawing

void CTelnetView::OnDraw(CDC* pDC)
{
	CTelnetDoc* pDoc = GetDocument();
	MY_ASSERT(pDoc);

	pDC->SelectObject(GetStockObject(SYSTEM_FONT));
	//pDC->SelectObject(GetStockObject(ANSI_FIXED_FONT));

	DrawCursor(pDC,FALSE);
	DoDraw(pDC);
	DrawCursor(pDC,FALSE);
}

void CTelnetView::DoDraw(CDC* pDC)
{
	LOG_TRACE("CTelnetView::DoDraw", "DoDraw");
	CRect clip;
	pDC->GetClipBox(clip);
	clip.top -= dtY;

	pDC->SetTextColor(cTextColor);
	pDC->SetBkColor(cBackgroundColor);

	char text[2] = {0x00, 0x00};

	for(int y = 0; y < bufferLines; y++)
	{
		if(y * dtY >= clip.top)
		{
			for(int x = 0; x < 80; x++)
			{
				text[0] = cText[x][y];
				if(text[0] == ' ')
				{
					pDC->FillSolidRect(CRect(x * dtX, y * dtY, 
						x * dtX + dtX, y * dtY + dtY), 
						cBackgroundColor);
				}
				else
				{
					pDC->TextOut(x * dtX, y * dtY, text);
				}
			}
		}
	}
}

void CTelnetView::OnInitialUpdate()
{
	LOG_TRACE("CTelnetView::OnInitialUpdate","OnInitialUpdate");

	SetConsoleCtrlHandler(ControlHandler, TRUE);

	CScrollView::OnInitialUpdate();
	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = dtX * 80 + 3;
	sizeTotal.cy = dtY * bufferLines + 3;
	SetScrollSizes(MM_TEXT, sizeTotal);

	ScrollToPosition(CPoint(0, bufferLines * 1000)); //go way past the end

	CHostDialog host;
	host.DoModal();
	cHostName = host.m_HostComboText;;
	uiPort = _ttoi(host.m_strPort); 

	LOG_TRACE("CTelnetView::OnInitialUpdate", "HOSTNAME %s", cHostName);
	LOG_TRACE("CTelnetView::OnInitialUpdate", "PORT %d", uiPort);

	//create the socket and hook up to the host
	BOOL bOK;
	cSock = new CClientSocket(this);
	if(cSock != NULL)
	{
		bOK = cSock->Create();
		if(bOK == TRUE)
		{
			LOG_TRACE("CTelnetView::OnInitialUpdate", "Connect %s:%d", cHostName, uiPort);
			cSock->AsyncSelect(FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT | FD_OOB);
			cSock->Connect(cHostName, uiPort);
			GetDocument()->SetTitle(cHostName);
			Sleep(90);
		}
		else
		{
			MY_ASSERT(FALSE);  //Did you remember to call AfxSocketInit()?
			delete cSock;
			cSock = NULL;
		}
	}
	else
	{
		AfxMessageBox("Could not create new socket",MB_OK);
	}

}

/////////////////////////////////////////////////////////////////////////////
// CTelnetView printing

BOOL CTelnetView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CTelnetView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CTelnetView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CTelnetView diagnostics

#ifdef _DEBUG
void CTelnetView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CTelnetView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CTelnetDoc* CTelnetView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTelnetDoc)));
	return (CTelnetDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTelnetView message handlers



void CTelnetView::ProcessMessage(CClientSocket * pSock)
{
	CSBLOCK("ProcessMessage");
	if(!IsWindow(m_hWnd)) return;
	if(!IsWindowVisible()) return;
	int nBytes = pSock->Receive(m_bBuf ,ioBuffSize );
	if(nBytes != SOCKET_ERROR)
	{
		LOG_TRACE("CTelnetView::ProcessMessage", "Received %d byets", nBytes);
		int ndx = 0;
		while (GetLine(m_bBuf, nBytes, ndx) != TRUE);
			
		
		ProcessOptions();
		MessageReceived(m_strNormalText);
		CSize sizeTotal;
		sizeTotal.cx = dtX * 80 + 3;
		sizeTotal.cy = dtY * bufferLines + processedLines;
		LOG_TRACE("CTelnetView::SetScrollSizes", "processedLines %d", processedLines);
		SetScrollSizes(MM_TEXT, sizeTotal);
		CSize totalSize = GetTotalSize();
		LOG_TRACE("CTelnetView::SetScrollSizes", "totalSize %d", totalSize.cy);
	}
	m_strLine.Empty();
	m_strResp.Empty();
}


void CTelnetView::ProcessOptions()
{
	CSBLOCK("ProcessOptions");
	CString m_strTemp;
	CString m_strOption;
	unsigned char ch;
	int ndx;
	int ldx;
	BOOL bScanDone = FALSE;

	m_strTemp = m_strLine;

	while(!m_strTemp.IsEmpty() && bScanDone != TRUE)
	{
		ndx = m_strTemp.Find(IAC);
		if(ndx != -1)
		{
			m_strNormalText += m_strTemp.Left(ndx);
			ch = m_strTemp.GetAt(ndx + 1);
			switch(ch)
			{
			case DO:
			case DONT:
			case WILL:
			case WONT:
				m_strOption		= m_strTemp.Mid(ndx, 3);
				m_strTemp		= m_strTemp.Mid(ndx + 3);
				m_strNormalText	= m_strTemp.Left(ndx);
				m_ListOptions.AddTail(m_strOption);
				break;
			case IAC:
				m_strNormalText	= m_strTemp.Left(ndx);
				m_strTemp		= m_strTemp.Mid(ndx + 1);
				break;
			case SB:
				m_strNormalText = m_strTemp.Left(ndx);
				ldx = m_strTemp.Find(SE);
				m_strOption		= m_strTemp.Mid(ndx, ldx);
				m_ListOptions.AddTail(m_strOption);
				m_strTemp		= m_strTemp.Mid(ldx);
				AfxMessageBox(m_strOption,MB_OK);
				break;
			}
		}

		else
		{
			m_strNormalText = m_strTemp;
			bScanDone = TRUE;
		}
	} 
	
	RespondToOptions();
}


void CTelnetView::RespondToOptions()
{
	CString strOption;
	
	while(!m_ListOptions.IsEmpty())
	{
		strOption = m_ListOptions.RemoveHead();

		ArrangeReply(strOption);
	}

	DispatchMessage(m_strResp);
	m_strResp.Empty();
}

void CTelnetView::ArrangeReply(CString strOption)
{
	CSBLOCK("ArrangeReply");
	unsigned char Verb;
	unsigned char Option;
	unsigned char Modifier;
	unsigned char ch;
	BOOL bDefined = FALSE;

	if(strOption.GetLength() < 3) return;

	Verb = strOption.GetAt(1);
	Option = strOption.GetAt(2);

	switch(Option)
	{
	case 1:	// Echo
	case 3: // Suppress Go-Ahead
		bDefined = TRUE;
		break;
	}

	m_strResp += IAC;

	if(bDefined == TRUE)
	{
		switch(Verb)
		{
		case DO:
			ch = WILL;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case DONT:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WILL:
			ch = DO;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WONT:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case SB:
			Modifier = strOption.GetAt(3);
			if(Modifier == SEND)
			{
				ch = SB;
				m_strResp += ch;
				m_strResp += Option;
				m_strResp += IS;
				m_strResp += IAC;
				m_strResp += SE;
			}
			break;
		}
	}

	else
	{
		switch(Verb)
		{
		case DO:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case DONT:
			ch = WONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WILL:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		case WONT:
			ch = DONT;
			m_strResp += ch;
			m_strResp += Option;
			break;
		}
	}
}

//send to the telnet server
void CTelnetView::DispatchMessage(CString strText)
{
	CSBLOCK("DispatchMessage");
	MY_ASSERT(cSock);
	const char* cstr = (LPCTSTR)strText;
	DEJA_TRACE("CTelnetView::DispatchMessage", "SEND %s", cstr);
	cSock->Send(strText, strText.GetLength());
}

BOOL CTelnetView::GetLine( unsigned char * bytes, int nBytes, int& ndx )
{
	CSBLOCK("GetLine");
	BOOL bLine = FALSE;
	while ( bLine == FALSE && ndx < nBytes )
	{
		unsigned char ch = bytes[ndx];
		
		switch( ch )
		{
		case '\r': // ignore
			m_strLine += "\r\n"; //"CR";
			break;
		case '\n': // end-of-line
//			m_strLine += '\n'; //"LF";
//			bLine = TRUE;
			break;
		default:   // other....
			m_strLine += ch;
			break;
		} 

		ndx ++;

		if (ndx == nBytes)
		{
			processedLines++;
			bLine = TRUE;
		}
	}
	return bLine;
}

void CTelnetView::MessageReceived(LPCSTR pText)
{
	CSBLOCK("MessageReceived");
	DEJA_BOOKMARK("CTelnetView::MessageReceived","CTelnetView::MessageReceived");
	LOG_TRACE("CTelnetView::MessageReceived", "MessageReceived %s", pText);
	CDC * pDC = GetDC();
	OnPrepareDC(pDC);
	DrawCursor(pDC,FALSE);
	CMainFrame * frm = (CMainFrame*)GetTopLevelFrame();
	pDC->SetTextColor(cTextColor);
	pDC->SetBkColor(cBackgroundColor);

//	dc.SelectObject(GetStockObject(SYSTEM_FONT));
	pDC->SelectObject(GetStockObject(ANSI_FIXED_FONT));
	int length = strlen(pText);
	char text[2] = {0x00, 0x00};
	for(int loop = 0; loop < length; loop++)
	{
		switch(pText[loop])
		{
		case 8: //Backspace
			cCursX--;
			if(cCursX < 0) cCursX = 0;
			break;
		case 9: //TAB
			cCursX++; //TBD make this smarter
			break;
		case 13: //CR
			cCursX = 0;
			break;
		case 10: //LF
			{
				for(int row = 0; row < bufferLines-1; row++)
				{
					for(int  col = 0; col < 80; col++)
					{
						cText[col][row] = cText[col][row+1];
					}
				}
				for(int  col = 0; col < 80; col++)
				{
					cText[col][bufferLines-1] = ' ';
				}
				DoDraw(pDC);
			}
			break;
		default:
			{
				cText[cCursX][bufferLines-1] = pText[loop];
				text[0] = cText[cCursX][bufferLines-1];
				if(text[0] == ' ')
				{
					pDC->FillSolidRect(CRect(cCursX * dtX, (bufferLines-1) * dtY, 
						cCursX * dtX + dtX, (bufferLines-1) * dtY + dtY), 
						cBackgroundColor);
				}
				else
				{
					pDC->TextOut(cCursX * dtX, (bufferLines-1) * dtY, text);
				}
				cCursX++;
				if(cCursX >= 80)
				{
					for(int row = 0; row < bufferLines-1; row++)
					{
						for(int  col = 0; col < 80; col++)
						{
							cText[col][row] = cText[col][row+1];
						}
					}
					for(int  col = 0; col < 80; col++)
					{
						cText[col][bufferLines-1] = ' ';
					}
					cCursX = 0;
					DoDraw(pDC);
				}
			}
			break;
		}
	}
	DrawCursor(pDC,TRUE);
	ReleaseDC(pDC);
}

void CTelnetView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CSBLOCK("OnChar");
	DEJA_BOOKMARK("CTelnetView::OnChar", "CTelnetView::OnChar");
	LOG_TRACE("CTelnetView::OnChar", "OnChar %d", nChar);
	if (nChar == VK_RETURN)
	{
		DispatchMessage("\r\n");
	}
	else
	{
		CString strCh = "";
		strCh.Format("%c", nChar);
		DispatchMessage(strCh);
	}
}

void CTelnetView::DrawCursor(CDC * pDC, BOOL pDraw)
{
	CSBLOCK("DrawCursor");
	DEJA_BOOKMARK("CTelnetView::DrawCursor", "CTelnetView::DrawCursor");
	LOG_TRACE("CTelnetView::DrawCursor", "DrawCursor");
	COLORREF color;
	CMainFrame * frm = (CMainFrame*)GetTopLevelFrame();
	if(pDraw) //draw
	{
		color = cTextColor;
	}
	else //erase
	{
		color = cBackgroundColor;
	}
	CRect rect(cCursX * dtX + 2, (bufferLines-1) * dtY + 1, 
		cCursX * dtX + dtX - 2, (bufferLines-1) * dtY + dtY -1);
	pDC->FillSolidRect(rect, color);
}



void CTelnetView::OnSize(UINT nType, int cx, int cy) 
{
	CSBLOCK("OnSize");
	DEJA_BOOKMARK("CTelnetView::OnSize", "CTelnetView::OnSize");
	LOG_TRACE("CTelnetView::OnSize", "OnSize");
	CScrollView::OnSize(nType, cx, cy);
	if(IsWindow(m_hWnd))
	{
		if(IsWindowVisible())
		{
			ScrollToPosition(CPoint(0, bufferLines * 1000)); //go way past the end
		}
	}
}

BOOL CTelnetView::OnEraseBkgnd(CDC* pDC) 
{
	CSBLOCK("OnEraseBkgnd");
	DEJA_BOOKMARK("CTelnetView::OnEraseBkgnd", "CTelnetView::OnEraseBkgnd");
	LOG_TRACE("CTelnetView::OnEraseBkgnd", "OnEraseBkgnd");
	CRect clip;
	pDC->GetClipBox(clip);
	CMainFrame * frm = (CMainFrame*)GetTopLevelFrame();
	pDC->FillSolidRect(clip,cBackgroundColor);
	return TRUE;
}
