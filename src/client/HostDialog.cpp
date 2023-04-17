// HostDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CTelnet.h"
#include "HostDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHostDialog dialog


CHostDialog::CHostDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CHostDialog::IDD, pParent)
	, m_HostComboText(_T(""))
	, m_strPort(_T("35010"))
	, m_uiPort(35010)
{
	//{{AFX_DATA_INIT(CHostDialog)

	//}}AFX_DATA_INIT
}


void CHostDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHostDialog)
	DDX_Control(pDX, IDC_COMBO2, m_HostCombo);
	DDX_CBString(pDX, IDC_COMBO2, m_HostComboText);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT_PORT, m_strPort);

	m_uiPort = _ttoi(m_strPort);
	if (m_uiPort > MAX_CONNECTION_PORT) {
		m_uiPort = MAX_CONNECTION_PORT;
	}
	if (m_uiPort < MIN_CONNECTION_PORT) {
		m_uiPort = MIN_CONNECTION_PORT;
	}
	m_strPort.Format(_T("%d"), m_uiPort);
}


BEGIN_MESSAGE_MAP(CHostDialog, CDialog)
	//{{AFX_MSG_MAP(CHostDialog)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CHostDialog::OnCbnSelchangeHost)
	ON_CBN_SETFOCUS(IDC_COMBO2, &CHostDialog::OnCbnSetfocusComboHost)
	ON_CBN_EDITUPDATE(IDC_COMBO2, &CHostDialog::OnSimpleEditupdate)
	ON_BN_CLICKED(IDOK, &CHostDialog::OnBnClickedOk)	
	ON_BN_CLICKED(IDCANCEL, &CHostDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDCLEAR, &CHostDialog::OnBnClickedClear)
	ON_EN_CHANGE(IDC_EDIT_PORT, &CHostDialog::OnEnChangeEditPort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHostDialog message handlers

void CHostDialog::OnOK() 
{
	UpdateData(TRUE);
	CDialog::OnOK();
}


void CHostDialog::OnCbnSelchangeHost()
{

}


void CHostDialog::OnCbnSetfocusComboHost()
{

	
}

void CHostDialog::OnBnClickedCancel()
{
	
	CDialog::OnCancel();
}


void CHostDialog::OnBnClickedClear()
{
}


void CHostDialog::OnEnChangeEditPort()
{

}

void CHostDialog::OnBnClickedOk()
{
	
	CDialog::OnOK();
}
BOOL CHostDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	DWORD dw = 0x000000FF;
	int idx = m_HostCombo.AddString(TEXT("192.168.0.101"));
	m_HostCombo.SetItemData(idx, dw);
	dw = 0x0000FF00;
	idx = m_HostCombo.AddString(TEXT("172.29.64.1"));
	m_HostCombo.SetItemData(idx, dw);
	dw = 0x00FF0000;
	idx = m_HostCombo.AddString(TEXT("127.0.0.1"));
	m_HostCombo.SetItemData(idx, dw);

	UpdateData(TRUE);
	return TRUE;
}

void CHostDialog::OnSimpleEditupdate()
{

	UpdateData(TRUE);
}