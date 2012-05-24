// NewsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Widgie.h"
#include "WidgieDlg.h"
#include "NewsDialog.h"
#include "NewsflashInfo.h"
#include "Newsflash.h"
#include "utils.h"


#undef CDX


#ifdef CDX
#define  CDXINCLUDEALL
#define NTDX3
#include <cdx.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/**
 *  CLASS NewsDialog
 *
 *  Implements the dialog that News Bar graphics can be displayed on.
 *
 */


/**
 *  Constructor for NewsDialog
 *
 *  Extracts initialisation data from file and its own data menbers
 *
 */
#ifdef CDX
CDXScreen   *           Screen = NULL;
CDX_LPDIRECTDRAW        cdx_DD;
#endif

NewsDialog::NewsDialog(CWnd* pParent /*=NULL*/) : CDialog(NewsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(NewsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_captionCount = -1;
	m_stillScrolling = FALSE;
    m_bkInit = FALSE;
	m_nextnews = FALSE;
	m_zipoff = FALSE;
	m_timerActive = FALSE;
	m_insideTimer = FALSE;
	m_newsbarActive = FALSE;

	m_runcount = 0;

	m_pauseendtime =
	m_pausetime = 0;

	QueryPerformanceFrequency( &m_freqCounter );

	{
		DEVMODE DevMode;

		if( EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &DevMode ) )
		{
			m_screenHz = DevMode.dmDisplayFrequency;
		}
		else
			m_screenHz = 60;

		m_usinterval = ( (1000000/(double)m_screenHz) - 0 );
		m_msinterval = m_usinterval/1000.0;
	}
}


NewsDialog::~NewsDialog()
{
	OutDebugs( "NewsDialog::~NewsDialog() ..." );
	//EndCleanup();
	OutDebugs( "NewsDialog::~NewsDialog() Done" );
}



void NewsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NewsDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NewsDialog, CDialog)
	//{{AFX_MSG_MAP(NewsDialog)
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NewsDialog message handlers



/**
 *  Initialises the NewsDialog
 *
 *  Sets up the initial state of the dialog and starts the
 *  timer used displaying and hiding the news bar.
 *
 */
BOOL NewsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	OutDebugs( "NewsDialog/OnInitDialog - this wnd = %08lx", this->m_hWnd );

	BringWindowToTop();

	OutDebugs( "NewsDialog - BringWindowToTop" );

	newsflash1 = NULL;
	newsflash1 = new Newsflash(this);

#ifdef CDX
	Screen = NULL;
    Screen = new CDXScreen();
    if (Screen)
	    cdx_DD = Screen->GetDD();
	else
		OutDebugs( "EEEK CDXScreen failed" );
#endif
	if( CFG->cfgShowScrollLogo )
	{
		/* Create the Logo Dialog*/
		logoDlg.Create(IDD_LOGO_DIALOG, NULL);
		logoDlg.LoadImage();
	}

	//StartTimers();			// This could be ok here...
	OutDebugs( "NewsDialog - OnInitDialog Completed." );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void NewsDialog::ReloadLogo( void )
{
	if( CFG->cfgShowScrollLogo )
	{
		logoDlg.LoadImage();
		newsflash1->LoadScrollImage();
		ShowDialog( TRUE );
	} else
		ShowDialog( FALSE );
}

void NewsDialog::EndCleanup( void )
{
	OutDebugs( "NewsDialog - EndCleanup ..." );

	OutDebugs( "NewsDialog - logoDlg.DestroyWindow" );
	//logoDlg.DestroyWindow();

	CloseWindow();

	StopTimers();

	if( newsflash1 )
	{
		OutDebugs( "NewsDialog - delete newsflash1" );
		delete newsflash1;
		newsflash1 = NULL;
	}

#ifdef CDX
	if( Screen )
	{
		OutDebugs( "NewsDialog - delete Screen" );
		delete Screen;
		Screen = NULL;
	}
#endif

	OutDebugs( "NewsDialog - EndCleanup Done." );
}



int NewsDialog::ShowNextCaption(void)
{
	BOOL proceed;

	proceed = LoadNextCaption();

	// try again , just incase it has failed.....
	if( !proceed )
		proceed = LoadNextCaption();

	if( proceed )
    {            
		//OutDebugs( "ShowNextCaption ... " );
        /* If there was a change in the display settings we need to make sure
            the newsflash presentation settings rflect this */
        CFG->cfgBarHeight = CFG->cfgTempBarHeight;

        /* Set up the background colour of the newsflash display*/
		LONG rgb = CFG->cfgBackgroundColor;
        HBRUSH bgBrush = CreateSolidBrush(rgb);    
        RECT nfRect;
        nfRect.bottom = CFG->cfgHeight();
        nfRect.left = 0;
        nfRect.right = CFG->cfgWidth();
        nfRect.top = 0;

		if( newsflash1 )
		{
			FillRect(newsflash1->BlankImage, &nfRect, bgBrush);

			/* Set up the text colour of the newsflash display*/
			SetTextColor( newsflash1->tempImage, CFG->cfgTextColor );


			/* Set up the font of the newsflash display
  			HFONT newsFont;
			newsFont = CreateFont(CFG->cfgFontHeight,0, 0, 0,FW_NORMAL, FALSE, FALSE, FALSE,
 			  						ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS,
 									PROOF_QUALITY, DEFAULT_PITCH + FF_DONTCARE, CFG->cfgFontFace);


			SelectObject(newsflash1->tempImage, newsFont);
			*/
		}
		return 1;
    } else
		return 0;
}



/**
 *  Event handler used to display, hide and advance the position of the 
 *  graphics of the newsbar
 *
 */
void NewsDialog::OnTimer(UINT nIDEvent) 
{
	m_insideTimer = TRUE;
	BOOL b_movieplaying = CFG->m_mainDlg->moviePlaying();

	// Pause for X seconds if m_pausetime is set... (ie do nothing during the scroller timer)
	if( m_pausetime  )
	{
		if( !m_pauseendtime && m_pausetime != -1 )					// init when to finish the pause time.
		{
			m_pauseendtime = time( NULL );
			m_pauseendtime+= m_pausetime;
		} else
		if( time(NULL) > m_pauseendtime && m_pausetime != -1 )		// if our pause time expired, stop pausing
		{
			m_pauseendtime = 0;
			m_pausetime = 0;
		} else
			Sleep( 10 );	// do nothing for a bit.
		//logoDlg.Hide();	
	} else
    if( nIDEvent == SCROLL_SPEED_TIMER && newsflash1 && CFG->cfgStepSpeed != 0 )
    {
	    //if (barPosition == CFG->cfgBarHeight)
		// if ( scrollerOn == TRUE )
	    {
            CDC* dialogDC;
            dialogDC = GetDC();

			// ok start next news item, clear the news bar to blank
			if( m_nextnews && m_newsbarActive )
			{
				BOOL shownextf;
				shownextf = ShowNextCaption();
				if( shownextf == FALSE )
					PauseScroller( 60 );
				m_nextnews = FALSE;
			}

			m_stillScrolling = newsflash1->AdvanceText();				// go scroll it ....

			// ok lets load up the next news....
            if (!m_stillScrolling)
			{
				m_nextnews = TRUE;
				// reshow newsbar logo
				if( m_newsbarActive &&  CFG->cfgShowScrollLogo )
				{
					ShowLogo();
				}
			}

            if (m_bkInit != TRUE)
            {
                CRect myRect;

                GetClientRect(&myRect);

                CBrush bgBrush;
                bgBrush.CreateSolidBrush( CFG->cfgBackgroundColor );
                dialogDC->FillRect( &myRect, &bgBrush );
				//bgBrush.DeleteObject();
                m_bkInit = TRUE;
            }
            ReleaseDC( dialogDC );

			// zip the scroller away fast...
			if (m_zipoff)
			{
				m_zipoff++;

				//newsflash1->speed ++;
				if( m_zipoff > 20 )
					m_zipoff = 0;
			}
		}
	} else
    if( nIDEvent == KEEP_FRONT_TIMER )
    {
		if( logoDlg )
		{
			if( m_runcount == 0 &&  CFG->m_mainDlg->moviePlaying() == FALSE )
			{
				logoDlg.BringWindowToTop();
				ShowLogo();
			}
			//else
			//if( !(m_runcount%10) && m_runcount < 50 && logoDlg.m_dlgVisible )
		}
		m_runcount++;
	}

	m_insideTimer = FALSE;
	CDialog::OnTimer(nIDEvent);
}


void NewsDialog::DoTimer(void)
{
	// only really do a timer event if we are running and not twice inside us.
	if( !m_insideTimer && m_newsbarActive )
		OnTimer(SCROLL_SPEED_TIMER);
}





/**
 *  Responds to key presses
 *
 *  Only used for debugging can be left in for release
 *  but will not be used since the users do not have a PC keyboard
 *
 */
void NewsDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	OutDebugs( "NewsDialog - Pressed char '%c'",nChar );

	/* send all messages to the main dialog*/
    ((CWidgieDlg *) GetParent())->PostMessage(WM_KEYDOWN, nChar, 0);
	
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

/**
 *  Makes the pointer of newsflashes in the graphics point to the
 *  caption data in the main thread
 */
void NewsDialog::BindCaptions(CList <NewsflashData, NewsflashData>* captionRef)
{
    m_captionList = captionRef;
}

/**
 *  Makes the reference of critical section that is used to access the playlists
 *  point to the critical section in the main thread.
 */
void NewsDialog::BindSection(LPCRITICAL_SECTION lpCRITICAL_SECTION)
{
    dataChangeSection = lpCRITICAL_SECTION;
}



/**
 *  Loads a the next caption into the display string
 *  
 *  Checks first is the caption if the caption has expired if it
 *  has it moves on to the next caption in the list.
 *
 *  If no valid captions exist then the display string is empty.
 *
 *  @return     if there is no valid captions this function retrurns
                false otherwise it returns TRUE.
 */

BOOL NewsDialog::LoadNextCaption()
{
    /* only load the next caption if we are not reforming the display lists */
    if( TryEnterCriticalSection(dataChangeSection) )
    {
		//OutDebugs( "NewsDialog - LoadNextCaption" );
		
        NewsflashInfo m_nextnewsflash;

        int lastCaption = m_captionCount;

		if( m_captionList == NULL )
		{
			OutDebugs( "NEWSDIALOG - No news caption list" );
			return FALSE;
		}

        int numberOfCaptions = m_captionList->GetCount();
    
        /* caption count is initialised to -1 in the constructor in
           order to make sure that the first caption that is diplayed
           is the first one on the list so at the beginning we need
           to set the previous caption to the last caption in the list */
        if (lastCaption < 0)
        {
            lastCaption = numberOfCaptions - 1;
        }
    
        /* make sure that newsflash date is valid before slecting it for display */
    
        /* m_captionCount != lastCaption means that if we go through
           all of the captions and noe are valid to break out of the
           loop even though we have found nothing valid */

        m_captionCount++;    
    
        BOOL validNewsFound = FALSE;
    
    
        if (m_captionList->GetCount() > 0) // if no captions where found dont do anything
        {
            while ((m_captionCount != lastCaption) && !(validNewsFound))
            {
                if (m_captionCount >= numberOfCaptions)
                {
                    m_captionCount = 0;
                }

                POSITION newPos = m_captionList->FindIndex(m_captionCount);
                m_nextnewsflash.data = m_captionList->GetAt(newPos);
                if (!m_nextnewsflash.Expired())
                {
                    validNewsFound = TRUE;
                }
                else
                {
                    m_captionCount++;
                }
            }
        }
   
        /* even if we do wrap around we still need to display the one and only valid caption */

        if (m_captionCount == lastCaption)
        {
            POSITION newPos = m_captionList->FindIndex(m_captionCount);

            m_nextnewsflash.data = m_captionList->GetAt(newPos);
            if (!m_nextnewsflash.Expired())
            {
                validNewsFound = TRUE;
            }
            else
            {
                /* none of the newsflashes are valid*/
                validNewsFound = validNewsFound;
            }
        }
        
		if( newsflash1 )
		{
			if (validNewsFound)
			{
				newsflash1->NewsString = m_nextnewsflash.data.text;
			}
			else
			{
				newsflash1->NewsString = "";
				OutDebugs( "NEWSDIALOG - NO VALID NEWS" );
			}
		}
   
        LeaveCriticalSection(dataChangeSection);
        return(validNewsFound);
    }
    else
    {
		OutDebugs( "NEWSDIALOG - FAILED TO AQUIRE SECTION" );
        return (FALSE);
    }
}



void NewsDialog::displayCentered(CString displayString, int pausefor )
{
	if( newsflash1 )
	{
		if( displayString )
		{
			m_pausetime = pausefor;
			OutDebugs( "displayCentered - %s", displayString.GetBuffer(0) );
			newsflash1->displayCentered( displayString );
		} else {
//			newsflash1->displayCentered( "" );
			PauseOff();
		}
	}
}

void NewsDialog::PauseScroller( int seconds )
{
	m_pausetime = seconds;
	m_pauseendtime = time( NULL ) + seconds;
}

void NewsDialog::PauseOff( void )
{
	m_pausetime = 0;
	m_pauseendtime = 0;
}


/**
 *  Responds to many times of messages
 *
 *  used only for sensing user pressing ENTER of ESC  
 *
 *  Only used for debugging can be left in for release
 *  but will not be used since the users do not have a PC keyboard
 *
 */
LRESULT NewsDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    if(message==WM_COMMAND)
    {
        if(wParam==IDOK||wParam==IDCANCEL)
        {
            ((CWidgieDlg *) GetParent())->PostMessage(WM_KEYDOWN, VK_ESCAPE, 0);
            return 1;
        }
    }
	return CDialog::WindowProc(message, wParam, lParam);
}

void NewsDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	OutDebugs( "NewsDialog - OnShowWindow" );
	
	// TODO: Add your message handler code here
    CRect myRect;
    CDC* dialogDC;

    GetClientRect(&myRect);
    dialogDC = GetDC();

    CBrush bgBrush;
    bgBrush.CreateSolidBrush(CFG->cfgBackgroundColor);
    dialogDC->FillRect(&myRect, &bgBrush);
	//bgBrush.DeleteObject();			// not sure if we need this...

    ReleaseDC(dialogDC);

	m_bkInit = TRUE;
}



DWORD NewsDialog::timeGetTime_us( void )
{
	LARGE_INTEGER counter;

	QueryPerformanceCounter( &counter );

	DWORD us_time = (DWORD)((counter.QuadPart) / (double)(m_freqCounter.QuadPart/1000000.0));

	return us_time;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// start mmTimer
// http://www.codeguru.com/Cpp/G-M/multimedia/timers/article.php/c1581/
/////////////////////////////////////////////////////////////////////////////////////////////

DWORD       gdwScanLine = 0;
DWORD       gdwScanLineLastTime = 0;
volatile    BOOL        gbUsed  = FALSE;

UINT        ScreenHalfHeight = GetSystemMetrics(SM_CYSCREEN) / 2;


extern "C"
void CALLBACK internalTimerProc(UINT id, UINT msg,DWORD dwUser, DWORD dw1, DWORD dw2)
{
	NewsDialog *	news = (NewsDialog *)dwUser;

#ifdef CDX
	if( cdx_DD )
	    cdx_DD->GetScanLine( &gdwScanLine );

	// works only if our hardware can do vblank scanline detection
	if( gdwScanLine || gdwScanLineLastTime )
    {
		static long us_last = news->timeGetTime_us(),		// last us time of scroll movement
					framecount = 0;							// frame counter
		static long scanLineHistory[64], hIndex=0;			// store scan line history here
		static long timeHistory[64], timeIndex=0;			// store the time difference in the history
		static long timeOut = (long)news->m_usinterval;			// timeout of one frame (1000/hz)

		// store last 32 scanline hits to analyze
		scanLineHistory[ hIndex++ ] = gdwScanLine;
		hIndex &= 0x3f;


		// check if we have crossed the bottom of the screen into a new frame
        if (gdwScanLine < gdwScanLineLastTime)
        {
            gbUsed = FALSE;
        }

		// if we are into a new screen frame, then lets update the scroller (poor mans vblank)
		if (!gbUsed )
		{
			if ( (gdwScanLine >= 0 //&& gdwScanLine < (ScreenHalfHeight*2)-45
				//|| (news->timeGetTime_us()-us_last>(news->m_usinterval+200)) 
				) )
			{
				long us_now = news->timeGetTime_us();
				long us_diff = us_now - us_last;


				timeHistory[ timeIndex++ ] = us_diff;
				timeIndex &= 0x3f;

				if( us_diff > (news->m_usinterval*2) )
					OutDebugs( "Frame %d : we are really late, missed frame - %ld us", framecount, us_diff );

				us_last = news->timeGetTime_us();

				framecount++;
				long scrollTime = news->timeGetTime_us();
				news->DoTimer();
				scrollTime = news->timeGetTime_us() - scrollTime;
				gbUsed = TRUE;
			} else
				OutDebugs( "past critical line %d", gdwScanLine );
		}

        gdwScanLineLastTime = gdwScanLine;
    } else
#endif
	// 1ms timer, so skip 20 to make a 20 ms timer.  or at 60hz its every 16.666ms or 166666us (we hope)
	{
#define	INTERVAL	((1000/(float)news->m_screenHz))

		static double us_t = 0;
		static LARGE_INTEGER pcounter, counter;

		QueryPerformanceCounter( &counter );

		double diffus = (counter.QuadPart - pcounter.QuadPart) / (double)(news->m_freqCounter.QuadPart/1000000.0);
		double diffms = (counter.QuadPart - pcounter.QuadPart) / (double)(news->m_freqCounter.QuadPart/1000.0);

		// make sure we only do this every 16.6ms, unless we were late last time, we go earlier
		if( diffus > us_t )
		{					//diffus > news->m_usinterval
			pcounter = counter;
			// ok lets now check if we are early or late and change our next trigger time +- a few us
			// old code;us_t = news->m_usinterval + (news->m_usinterval-diffus);
			// new next time = 16666us - latetime;
			if( us_t == 0 )
				us_t = news->m_usinterval;
			else
				us_t = news->m_usinterval - (diffus-us_t);
			if( us_t > 10000 )
			{
				//OutDebugs( "internalTimerProc has been triggered (diffus=%.0f, us_t=%.0f)", diffus, us_t );
				if( news )
		            news->DoTimer();
			}
		}
	}
}



bool NewsDialog::StartMMTimer(UINT period, bool oneShot, UINT resolution)
{
    bool        res = false;
    MMRESULT    result;

	OutDebugs( "NewsDialog - StartMMTImer" );

	{
		TIMECAPS    tc;

		int err = timeGetDevCaps(&tc, sizeof(TIMECAPS) );
		if (TIMERR_NOERROR == err)
		{
			m_timerRes = min(max(tc.wPeriodMin, resolution), tc.wPeriodMax);
			timeBeginPeriod(m_timerRes);
			OutDebugs( "timeGetDevCaps has succeeded, period =%d", m_timerRes );
		}
		else 
		{
			OutDebugs( "timeGetDevCaps has failed - %d", err );
			return false;
		}   
	}

	{
		result = timeSetEvent(
			period,
			m_timerRes,                                                 // (LPTIMECALLBACK)hEvent
			internalTimerProc,
			(DWORD)this,                                                          // (DWORD)this,
			(oneShot ? TIME_ONESHOT : TIME_PERIODIC)                    // TIME_KILL_SYNCHRONOUS
			);                                                          // CALLBACK_EVENT_SET
		if (NULL != result)
		{
			m_timerId = (UINT)result;
			OutDebugs( "timeSetEvent has succeeded" );
			res = true;
		} else
			OutDebugs( "timeSetEvent has failed - %d", result );
	}

    return res;
}


/////////////////////////////////////////////////////////////////////////////////////////////
// stop mmTimer
// could be modified in XP with TIME_KILL_SYNCHRONOUS
/////////////////////////////////////////////////////////////////////////////////////////////
bool NewsDialog::StopMMTimer(bool bEndTime)
{
    MMRESULT    result;

    result = timeKillEvent(m_timerId);
    if (TIMERR_NOERROR == result)
    {
        m_timerId = 0;
        if (bEndTime )
        {
            for (volatile int i=0; i<10; i++)
            {
                Sleep(10);                          //TIME_KILL_SYNCHRONOUS
            }
        }
    }

    if (0 != m_timerRes)
    {
        timeEndPeriod(m_timerRes);
        m_timerRes = 0;
    }

    return TIMERR_NOERROR == result;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Resume mmTimer
/////////////////////////////////////////////////////////////////////////////////////////////
void NewsDialog::SafeStartTimer( int rate )
{
	// default rate is 2ms
    if (!StartMMTimer(rate, FALSE, 0))
    {
		OutDebugs( "StartMMTimer has failed :( ,reverting to WM_TIMER" );
		SetTimer( SCROLL_SPEED_TIMER, CFG->cfgScrollSpeed, NULL );
		//PostMessage(WM_CLOSE, 0, 0);
    }
}


void NewsDialog::StartTimers( int mstimer )
{
	if( m_timerActive )
	{
		//OutDebugs( "NewsDialog - StartTimers/Already active." );
		//StopTimers();
	}

	if( m_timerActive == FALSE )
	{
		//OutDebugs( "NewsDialog - StartTimers" );

		PauseOff();

		SetTimer( KEEP_FRONT_TIMER, 100, NULL );

#ifndef USE_PEEKMSG
		SafeStartTimer( mstimer );
#endif

		m_timerActive = TRUE;
#ifndef CDX
		OutDebugs( "Starting WM_TIMER event for scroller" );
		//SetTimer(SCROLL_SPEED_TIMER, CFG->cfgScrollSpeed, NULL );
#endif
	}
}

void NewsDialog::StopTimers(void)
{
	//OutDebugs( "NewsDialog - StopTimers() ..." );
	// wait till we finish timer tasks..
	while( m_insideTimer )
		Sleep(10);

	//OutDebugs( "NewsDialog - Kill Timers" );
	KillTimer(KEEP_FRONT_TIMER);
	KillTimer(SCROLL_SPEED_TIMER);

	if( m_timerActive )
		StopMMTimer( false );

	m_timerActive = FALSE;

	if( newsflash1 )
		newsflash1->textPosition = CFG->cfgWidth() + 10;
	//OutDebugs( "NewsDialog - StopTimers() Done" );
}

// Hides the news bar so it cannot be seen
void NewsDialog::Hide() 
{
	OutDebugs( "NewsDialog - Hide" );

	if( newsflash1 )
		StopTimers();

	m_newsbarActive = FALSE;

	RECT rc = { CFG->cfgLeft,  CFG->cfgBottom, 
				CFG->cfgWidth(), CFG->cfgBarHeight };

	MoveWindow(	&rc, FALSE );

	InvalidateRect( &rc, FALSE );

	if( CFG->cfgShowScrollLogo && logoDlg )
		logoDlg.Hide();		// hide our little logo too
}




void NewsDialog::ShowDialog( BOOL showLogo )
{
	//OutDebugs( "NewsDialog - ShowDialog" );

	RECT rc = { CFG->cfgLeft,  CFG->cfgBottom-CFG->cfgBarHeight,
				CFG->cfgWidth(), CFG->cfgBarHeight };


	MoveWindow( CFG->cfgLeft,  CFG->cfgBottom-CFG->cfgBarHeight,
		        CFG->cfgWidth(), CFG->cfgBarHeight, FALSE );

	//BringWindowToTop();
	////MoveWindow( &rc, TRUE );
	ShowWindow(SW_NORMAL);

	InvalidateRect( &rc, FALSE );

	//UpdateWindow();
	//if( CFG->cfgShowScrollLogo && showLogo )
	{
		PauseOff();
		if( newsflash1 )
			StartTimers();
	}

	if( CFG->cfgShowScrollLogo && showLogo && CFG->m_mainDlg->moviePlaying() == FALSE )
	{
		BringWindowToTop();
		ShowLogo();
	} else {
		BringWindowToTop();
		HideLogo();
	}

	m_newsbarActive = TRUE;
}



void NewsDialog::HideLogo() 
{
	if( CFG->cfgShowScrollLogo && logoDlg )
		logoDlg.Hide();		// hide our little logo too


	RECT myRect = { CFG->cfgBottom - CFG->cfgBarHeight, 0, 
					 CFG->cfgBottom , 120 };
	CBrush bgBrush;
	bgBrush.CreateSolidBrush( CFG->cfgBackgroundColor );


	InvalidateRect( NULL, FALSE );
}

void NewsDialog::ShowLogo( void )
{
	if( CFG->cfgShowScrollLogo && logoDlg )
	{
		logoDlg.BringWindowToTop();		// now make sure its up top
		logoDlg.ShowDialog( TRUE );		// show the logo window
	} else
		logoDlg.ShowDialog( FALSE );	// show the logo window
}


