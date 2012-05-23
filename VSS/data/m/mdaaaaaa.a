#pragma once


// LoginEdit dialog

class CLoginEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CLoginEditDlg)

public:
	CLoginEditDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginEditDlg();

// Dialog Data
	enum { IDD = IDD_LOGINEDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int AcceptDetails(void);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	BOOL OnInitDialog(void);
};
