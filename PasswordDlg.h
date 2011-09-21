#pragma once


// PasswordDlg dialog

class PasswordDlg : public CDHtmlDialog
{
	DECLARE_DYNCREATE(PasswordDlg)

public:
	PasswordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~PasswordDlg();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_DIALOG1, IDH = IDR_HTML_PASSWORDDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
