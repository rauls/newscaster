// CMovieDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Widgie.h"
#include "WidgieDlg.h"
#include "swFlashDialog.h"
#include "utils.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMovieDialog dialog
//
//  Implements the dialog that displays Shockwave media
//
//


CMovieDialog::CMovieDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMovieDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMovieDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	moviePaused =
	movieVisible = FALSE;
}


CMovieDialog::~CMovieDialog()
{
	OutDebugs( "CMovieDialog deconstructing" );
}


void CMovieDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMovieDialog)
	DDX_Control(pDX, IDC_SHOCKWAVEFLASH1, movie);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMovieDialog, CDialog)
	//{{AFX_MSG_MAP(CMovieDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMovieDialog message handlers


/**
 *  Initialises the FlashDialog
 *
 *  Sets up the initial state of the dialog and starts the
 *  timer that checks if the Flash moveie has finished playing.
 *
 */


BOOL CMovieDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if( this->m_hWnd ){
		MoveWindow(	CFG->cfgLeft, -CFG->cfgBottom,
					CFG->cfgWidth(), CFG->cfgHeight(), FALSE );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMovieDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
//	OutDebugs( "******* ON SHOWWINDOW  ********" );
	CDialog::OnShowWindow(bShow, nStatus);


	// TODO: Add your message handler code here
	if (bShow == TRUE)
	{
		this->movie.Play();
		//WindowToTopCorner();
		// move window into viewable area...
		//PlayMovie();
	}
  	
}
	
/**
 *  Event handler for a Dialog timer
 *
 *  Check is the flash movie has finished playing.
 *  If the movie has finished this dialog is hidden
 *
 */

void CMovieDialog::OnTimer(UINT nIDEvent) 
{
	long currentframe, totalframes;

	currentframe = this->movie.CurrentFrame();
	totalframes = this->movie.GetTotalFrames();


	// TODO: Add your message handler code here and/or call default
    if( currentframe <= 100 && movieVisible == FALSE && moviePlaying )
	{
		movieVisible = TRUE;
		//this->movie.StopPlay();
		ShowWindow(SW_SHOWNORMAL);
		WindowToTopCorner();
		((CWidgieDlg *) GetParent())->ShowNewsDialog();
	} else
    if( currentframe > 1 && currentframe < 10 )
	{
		((CWidgieDlg *) GetParent())->ShowNewsDialog();
		//OutDebugs( "Moving Flash window to top corner" );
//		WindowToTopCorner();
	} else
    if( (currentframe >= (totalframes) - 1) ||
		(movie.IsPlaying() == FALSE && moviePaused == FALSE )
		)
	{
		OutDebugs( "Flash Timer - Movie has finished, cleaning up" );
      	this->KillTimer(CHECK_FLASH_END_TIMER);

		this->movie.Stop();
		this->movie.SetPlaying(FALSE);

		((CWidgieDlg *) GetParent())->StopMoviePlayback();
		//((CWidgieDlg *) GetParent())->PostMessage(WM_KEYDOWN, 'X', 0);

    }
	CDialog::OnTimer(nIDEvent);
}



void CMovieDialog::OnClose()
{
	OutDebugs( "CMovieDialog OnClose called" );

	CDialog::OnClose();

}




void CMovieDialog::OnPaint() 
{
	//OutDebugs( "******* ON PAINT  ********" );
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		//dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	//WindowToTopCorner();
}

void CMovieDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
    OutDebugs( "KeyPressed in FLASH player" );
    
    ((CWidgieDlg *) GetParent())->PostMessage(WM_KEYDOWN, nChar, 0);
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CMovieDialog::Hide(void)
{
	MoveWindow(	CFG->cfgLeft, -CFG->cfgBottom, 	CFG->cfgWidth(), CFG->cfgHeight(), FALSE );
	RECT rc = { 0,0, CFG->cfgWidth(), CFG->cfgHeight() };
	InvalidateRect( &rc, FALSE );
    //ShowWindow(SW_HIDE);
	movieVisible = FALSE;
}

void CMovieDialog::WindowToTopCorner(void)
{
	MoveWindow(	CFG->cfgLeft, CFG->cfgTop, CFG->cfgWidth(), CFG->cfgHeight()-CFG->cfgBarHeight, FALSE );

	RECT rc = { 0,0, CFG->cfgWidth(), CFG->cfgHeight() - CFG->cfgBarHeight };
	InvalidateRect( &rc, FALSE );
	movieVisible = TRUE;
}


int CMovieDialog::StopMovie(void)
{
	this->movie.StopPlay();
    this->movie.Stop();
    this->movie.SetPlaying(FALSE);

    //this->movie.GotoFrame(movie.GetTotalFrames());

    while (this->movie.IsPlaying() == TRUE)
    {
        this->movie.StopPlay();
    }

	moviePlaying = FALSE;

	OutDebugs( "Stopping flash movie" );
	//Hide();
	DestroyWindow();

	return 0;
}

int CMovieDialog::PauseMovie(void)
{
	if( moviePaused == FALSE )
	{
		OutDebugs( "Pausing flash movie" );
		movie.StopPlay();
		//movie.SetPlaying( FALSE );
		moviePaused = TRUE;
	} else 
	{
		OutDebugs( "Re-playing flash movie" );
		moviePaused = FALSE;
		//movie.SetPlaying( TRUE );
		movie.Play();
	}
	return 1;
}

int CMovieDialog::PlayMovie(void)
{
	//cout << "OnShowWindow , showing SWF\n";
	if( moviePlaying == FALSE )
	{
		OutDebugs( "Flash - Play Movie - scalemode=1 (x=%d,y=%d, w=%d,h=%d)", CFG->cfgLeft, CFG->cfgRight,   CFG->cfgWidth(), CFG->cfgHeight() );
		moviePlaying = TRUE;

		this->movie.SetScaleMode(1);
		
		/* make the backgroung white */
		this->movie.SetBackgroundColor(0x00FFFFFF);
		this->movie.SetFrameNum(0);

		this->movie.MoveWindow(	CFG->cfgLeft, CFG->cfgTop,
								CFG->cfgWidth(), CFG->cfgHeight(), TRUE);

		// start playing it
		this->movie.Play();

		// setup the timer that  checks is the movie has ended 
		this->SetTimer(CHECK_FLASH_END_TIMER, 400, NULL);

//		ShowWindow(SW_SHOWNORMAL);
	} else 
	if( moviePaused )
	{
		this->movie.Play();
		moviePaused = FALSE;
	} else
	{
		OutDebugs( "ERROR: Cannot play flash movie while it is already playing" );
	}
	return 0;
}
