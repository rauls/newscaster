// tstDlg.h : header file
//

#if !defined(AFX_TSTDLG_H__FCE53796_E033_11D2_9258_B07006C10000__INCLUDED_)
#define AFX_TSTDLG_H__FCE53796_E033_11D2_9258_B07006C10000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTstDlg dialog

class CTstDlg : public CDialog
{
// Construction
public:
	CTstDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTstDlg)
	enum { IDD = IDD_TST_DIALOG };
	CString	m_strPassword;
	CString	m_strPhoneNumber;
	CString	m_strUserName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTstDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	bool DialUp();
	bool HangUp();

	// Generated message map functions
	//{{AFX_MSG(CTstDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonConnect();
	afx_msg void OnButtonHangUp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSTDLG_H__FCE53796_E033_11D2_9258_B07006C10000__INCLUDED_)
