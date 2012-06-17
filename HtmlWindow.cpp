// HtmlWindow.cpp : implementation file
//

#include "stdafx.h"
#include "Widgie.h"
#include "WidgieDlg.h"
#include "utils.h"
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
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CHtmlWindow::OnInitDialog() 
{
	CDialog::OnInitDialog();

	moviePlaying = FALSE;
	
	GetWindowRect(&m_InitRect);

	if( this->m_hWnd ){
		MoveWindow(	CFG->cfgLeft, CFG->cfgTop,
					CFG->cfgWidth(), CFG->cfgHeight(), FALSE );
	}

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


BOOL CHtmlWindow::ShowPageURL( CString url ) 
{
	if( IsWindow(m_page.m_hWnd) )
	{
		m_page.Navigate2(url);

		ShowWindow( SW_SHOW );
		moviePlaying = TRUE;
		m_startTime = time(NULL);
		// setup the timer that  checks is the movie has ended 
		this->SetTimer(1, 400, NULL);

		return TRUE;
	} else {
		OutDebugs( "Showpage has no window ctrl" );
	}
	return FALSE;
}

BOOL CHtmlWindow::ChangePageURL( CString url ) 
{
	if( IsWindow(m_page.m_hWnd) )
	{
		m_page.Navigate2(url);
		return TRUE;
	}
	return FALSE;
}

void CHtmlWindow::Blank( void ) 
{
	ChangePageURL( "About:blank" );
}

void CHtmlWindow::Black( void ) 
{
	CFile hfile;
	CString filename = m_mediaDir + "black.html";
    BOOL fileExists = hfile.Open(filename, CFile::modeRead);

	ChangePageURL( "file:///" + filename );
}


void CHtmlWindow::Hide( void ) 
{
	if( IsWindow(m_page.m_hWnd) )
	{
		Black();
		ShowWindow( SW_HIDE );
	}
}



//
//
// ##### ON TIMER
//
//
void CHtmlWindow::OnTimer(UINT nIDEvent) 
{
	if( time(NULL) > m_startTime + m_playLength )
	{
		StopMovie();
	}
	CDialog::OnTimer(nIDEvent);
}


void CHtmlWindow::ResetTimer()
{
    this->KillTimer(1);
    this->SetTimer(1, 400, NULL);
}


int CHtmlWindow::StopMovie(void)
{
	Hide();

	moviePlaying = FALSE;

	KillTimer(1);

	OutDebugs( "Stopping Html" );
	
	Hide();
	//DestroyWindow();

	return 0;
}

void CHtmlWindow::PostNcDestroy() 
{
	delete this;
}

// CHtmlWindow message handlers
