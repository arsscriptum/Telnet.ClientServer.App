#if !defined(AFX_HOSTDIALOG_H__FE95BC99_9860_11D3_8CD5_00C0F0405B24__INCLUDED_)
#define AFX_HOSTDIALOG_H__FE95BC99_9860_11D3_8CD5_00C0F0405B24__INCLUDED_
#include "TTComboBox.h"
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// HostDialog.h : header file
//

#define MIN_CONNECTION_PORT 1024
#define MAX_CONNECTION_PORT 65535
/////////////////////////////////////////////////////////////////////////////
// CHostDialog dialog

class CHostDialog : public CDialog
{
// Construction
public:
	CHostDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHostDialog)
	enum { IDD = IDD_HOST };
	CTTComboBox m_HostCombo;
	CString m_HostComboText;
	CTTComboBox	m_DropDown;
	int		m_nDropListIndex;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHostDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHostDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeHost();
	afx_msg void OnCbnSetfocusComboHost();
	afx_msg void OnBnClickedOk();
	afx_msg void OnSimpleEditupdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedClear();
	afx_msg void OnEnChangeEditPort();
	CString m_strPort;
	unsigned int m_uiPort;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HOSTDIALOG_H__FE95BC99_9860_11D3_8CD5_00C0F0405B24__INCLUDED_)
