// HtmlWindow.cpp : implementation file
//

#include "stdafx.h"

#include "HtmlCtrl.h"
#include "HtmlWindow.h"

#define	IDC_HTML_CONTROL	10001

// CHtmlWindow dialog

IMPLEMENT_DYNAMIC(CHtmlWindow, CDialog)

CHtmlWindow::CHtmlWindow(CWnd* pParent /*=NULL*/ )
	: CDialog(CHtmlWindow::IDD, pParent)
{
}

CHtmlWindow::~CHtmlWindow()
{
}

void CHtmlWindow::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHtmlWindow, CDialog)
END_MESSAGE_MAP()

BOOL CHtmlWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	GetWindowRect(&m_InitRect);

	//VERIFY(m_page.CreateFromStatic(IDC_HTMLSTATIC, this));
	VERIFY(m_page.CreateFromWindow(IDC_HTML_CONTROL, this));

	if( m_HtmlFile.IsEmpty() )
	{
		ShowWindow( SW_HIDE );
	} else
	{
		m_page.Navigate2(_T("file://") + m_HtmlFile);
	}

	m_page.GetWindowRect(&m_InitPageRect);		// current list rect

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHtmlWindow::Hide( void ) 
{
	if( IsWindow(m_page.m_hWnd) )
	{
		ShowWindow( SW_HIDE );
	}
}


BOOL CHtmlWindow::ShowPageURL( CString url ) 
{
	if( IsWindow(m_page.m_hWnd) )
	{
		m_page.Navigate2(url);
		ShowWindow( SW_SHOW );
		return TRUE;
	}
	return FALSE;
}

void CHtmlWindow::Blank( void ) 
{
	ShowPageURL( "About:blank" );
}


void CHtmlWindow::PostNcDestroy() 
{
	delete this;
}

// CHtmlWindow message handlers
