// logoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Widgie.h"
#include "logoDialog.h"
#include "WidgieDlg.h"
#include "utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




/////////////////////////////////////////////////////////////////////////////
// logoDialog dialog


logoDialog::logoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(logoDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(logoDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

logoDialog::~logoDialog()
{
	logoImage.Destroy();
	m_dlgVisible = FALSE;
	OutDebugs( "logoDialog::~logoDialog() Done" );
}



void logoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(logoDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(logoDialog, CDialog)
	//{{AFX_MSG_MAP(logoDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// logoDialog message handlers

BOOL logoDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	OutDebugs( "logoDialog/OnInitDialog - this wnd = %08lx", this->m_hWnd );

	// TODO: Add extra initialization here
    m_dlgVisible = false;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void logoDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	OutDebugs( "logoDialog - OnShowWindow" );
	
	if( m_dlgVisible )
	    BringWindowToTop();
}



/**
 *  Responds to key presses
 *
 *  Only used for debugging can be left in for release
 *  but will not be used since the users do not have a PC keyboard
 *
 */
void logoDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	OutDebugs( "logoDialog - Pressed char '%c'",nChar );

	/* send all messages to the main dialog*/
    ((CWidgieDlg *) GetParent())->PostMessage(WM_KEYDOWN, nChar, 0);
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


void logoDialog::OnLButtonDown(UINT nChar, CPoint p) 
{
	ShowDialog( TRUE );
}

void logoDialog::ShowDialog( BOOL forceit )
{
	// only show  the window if it isn't already visible
    if (m_dlgVisible == FALSE || forceit == TRUE )
    {
		RECT rc = { CFG->cfgLeft, CFG->cfgBottom, 0, 0 };
		rc.right = rc.left + logoImage.GetWidth();
		rc.top = CFG->cfgBottom - logoImage.GetHeight();
		rc.bottom = CFG->cfgBottom;


		OutDebugs( "logoDialog - ShowDialog (forced = %d)", forceit );

		// position window at the bottom left corner
		MoveWindow( CFG->cfgLeft, CFG->cfgBottom - logoImage.GetHeight(),
					logoImage.GetWidth(), logoImage.GetHeight(), FALSE );

		ShowWindow( SW_SHOWNORMAL );

		if( logoImage.IsNull() == FALSE )
		{
			CDC* logoDC;
			
			int tries = 1000;
			// Display the logo
			while( !(logoDC=GetDC()) && tries>0 )
				tries--;
			if( logoDC )
			{
				CRect logoRect;
				GetClientRect(&logoRect);

				if( AFXDIALOG->brightnessLevelLow )
				{
					RECT destRC = {0,0,0,0};
					destRC.right = logoImage.GetWidth();
					destRC.bottom = logoImage.GetHeight();

					logoDC->FillSolidRect( &destRC, 0 );
					logoImage.AlphaBlend( logoDC->m_hDC, destRC, destRC, 190, AC_SRC_OVER );
				} else 
				{
					logoImage.Draw(logoDC->m_hDC, logoRect);
				}

				ReleaseDC(logoDC);
			}
		}


		InvalidateRect( NULL, FALSE );

		UpdateWindow();
		BringWindowToTop();

		m_dlgVisible = TRUE;
		//OutDebugs( "logoDialog - ShowDialog Done." );
    }
}


void logoDialog::Hide( void )
{
	//OutDebugs( "logoDialog.Hide" );
//	ShowWindow( SW_HIDE );
	m_dlgVisible = FALSE;

	MoveWindow(	-CFG->cfgLeft-logoImage.GetWidth(), CFG->cfgBottom,
				logoImage.GetWidth(), logoImage.GetHeight(), FALSE );

	RECT rc = { CFG->cfgLeft, CFG->cfgBottom, 0, 0 };
	rc.right = rc.left + logoImage.GetWidth();
	rc.top = CFG->cfgBottom - logoImage.GetHeight();
	rc.bottom = CFG->cfgBottom;
	//InvalidateRect( &rc, FALSE );
}


void logoDialog::LoadImage( void )
{
	// load image in content folder, failing that try root folder, and if that fails, try it as a jpeg instead in the content folder
	logoImage.Destroy();

	CString logoFile = DEFPATH_IMAGES + CFG->cfgNewsLogoFileName;

	if( logoImage.Load( logoFile ) != S_OK  )
	{
		logoFile = DEFPATH_IMAGES;
		logoFile += NEWSLOGO_FN;
		if ( logoImage.Load( logoFile ) != S_OK  )
		{
			logoFile.Replace( ".bmp", ".jpg" );
			if( logoImage.Load(logoFile) != S_OK )
				OutDebugs( "ERROR: Could not open logo image %s", logoFile.GetBuffer(0) );
		}
	}
}





