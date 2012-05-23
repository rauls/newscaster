// LoginEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Widgie.h"
#include "LoginEdit.h"
#include ".\loginedit.h"


// LoginEdit dialog

IMPLEMENT_DYNAMIC(CLoginEditDlg, CDialog)
CLoginEditDlg::CLoginEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginEditDlg::IDD, pParent)
{
}

CLoginEditDlg::~CLoginEditDlg()
{
}

void CLoginEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoginEditDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


//
// LoginEdit message handlers
//
int CLoginEditDlg::AcceptDetails(void)
{
	CString u,p, id;

	GetDlgItemText( IDC_USERNAME, u );
	GetDlgItemText( IDC_PASSWORD, p );
	GetDlgItemText( IDC_SLIDESHOW, id );

	// if a new ID is entered and different to the previous one, use it
	if( id.GetLength()>0 && id != CFG->cfgID )
		CFG->cfgID = id;

	// both username and password must be entered to be saved.
	if( u.GetLength()>0 && p.GetLength()>0 )
	{
		CFG->cfgUsername = u;
		CFG->cfgUserpass = p;
	}

	//save password/slideshowid to INI file.
	if( (u.GetLength()>0 && p.GetLength()>0) ||
		(id.GetLength()>0) 
		)
	{
		CFG->SaveLoginDetails();
	}

	return 0;
}

void CLoginEditDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	AcceptDetails();
	OnOK();
}

void CLoginEditDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

BOOL CLoginEditDlg::OnInitDialog(void)
{
	CDialog::OnInitDialog();

	ShowCursor( TRUE );
	ShowCursor( TRUE );

	SetDlgItemText( IDC_USERNAME, CFG->cfgUsername );
	SetDlgItemText( IDC_USERNAME, CFG->cfgUserpass );
	SetDlgItemText( IDC_SLIDESHOW, CFG->cfgID );

	ShowCursor( TRUE );
	ShowCursor( TRUE );
	return 0;
}
