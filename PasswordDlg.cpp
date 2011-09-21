// PasswordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Widgie.h"
#include "PasswordDlg.h"


// PasswordDlg dialog

IMPLEMENT_DYNCREATE(PasswordDlg, CDHtmlDialog)

PasswordDlg::PasswordDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(PasswordDlg::IDD, PasswordDlg::IDH, pParent)
{
}

PasswordDlg::~PasswordDlg()
{
}

void PasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL PasswordDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	SetDlgItemText( IDC_USERNAME, CFG->cfgUsername );
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(PasswordDlg, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(PasswordDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// PasswordDlg message handlers

HRESULT PasswordDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	CString u,p;

	u = GetDlgItemText( IDC_USERNAME, &u );
	p = GetDlgItemText( IDC_PASSWORD, &u );

	if( u.GetLength()>0 )
		CFG->cfgUsername = u;

	if( p.GetLength()>0 )
		CFG->cfgUserpass = p;

	OnOK();
	return S_OK;  // return TRUE  unless you set the focus to a control
}

HRESULT PasswordDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;  // return TRUE  unless you set the focus to a control
}
