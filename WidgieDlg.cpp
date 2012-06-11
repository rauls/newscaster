/*
// WidgieDlg.cpp : implementation file
//
*/

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>


#include "windows.h"
#include "winuser.h"
#include <afxstr.h>
#include <atlimage.h> 

#include <wininet.h>

#include "Widgie.h"
#include "WidgieDlg.h"
#include "WidgieXML.h"
#include "Picture.h"
#include "NewsBarThread.h"
#include "LoginEdit.h"
#include "history.h"
#include "utils.h"
#include "version.h"
#include "netadapter.h"
#include "winnet_io.h"
#include "SmartInfo.h"

#include <mmsystem.h>
#include ".\widgiedlg.h"

#include ".\httpinterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	ISNETWORKPRESENT	GetSystemMetrics( SM_NETWORK )
#define	FONT_HEIGHT			16


#define	BUTTONKEY_FORW		'F'
#define	BUTTONKEY_BACK		'B'

BOOL	g_logdebug;

BOOL AudioPlayResource(LPSTR lpName)
{
	BOOL bRtn = FALSE;
	LPSTR lpRes; 
	HANDLE hResInfo, hRes;      // Find the WAVE resource.  

	hResInfo = FindResource(AfxGetInstanceHandle(), lpName, "WAVE");
	if (hResInfo == NULL)
	{
		bRtn = sndPlaySound( NULL, SND_NODEFAULT );
		bRtn = sndPlaySound( lpName, SND_ASYNC );
	} else
	{
		if( hRes = LoadResource(AfxGetInstanceHandle(), (HRSRC)hResInfo) )
		{      // Load the WAVE resource.  
			lpRes = (LPSTR)LockResource(hRes);     // Lock the WAVE resource and play it.  
			if (lpRes ) 
			{ 
				//bRtn = sndPlaySound( NULL, SND_MEMORY | SND_ASYNC  | SND_NODEFAULT );
				bRtn = sndPlaySound( lpRes, SND_MEMORY | SND_ASYNC  | SND_NODEFAULT );
				UnlockResource(hRes);	
			} else
				bRtn = 0;

			FreeResource(hRes);      // Free the WAVE resource and return success or failure. 
		}
	}
	return bRtn;
} 




/////////////////////////////////////////////////////////////////////////////
// CWidgieDlg dialog

CWidgieDlg::CWidgieDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWidgieDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWidgieDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	OutDebugs("MainDlg created....");
	newsThread = NULL;
	m_lastStatusTime = 0;
    
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	ptheApp = (CWidgieApp *)AfxGetApp();

	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0);

	m_slideshowLoopCount = 0;

	buttonSoundPtr[0] = NULL;
	buttonSoundPtr[1] = NULL;
	buttonSoundPtr[2] = NULL;
	buttonSoundPtr[3] = NULL;
	buttonSoundPtr[4] = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CWidgieDlg::~CWidgieDlg()
{
	OutDebugs("MainDlg ended.");
}


void CWidgieDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWidgieDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWidgieDlg, CDialog)
	//{{AFX_MSG_MAP(CWidgieDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SHOWWINDOW()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWidgieDlg message handlers




BOOL CWidgieDlg::InitApplication()
{
	textColor = 0x00FF00;			// green
	textColorBG = 0x00;				// black

	OutDebugs( "DEBUG: InitApplication() Starting" );

	CString vstr = CString(AfxGetAppName()) + " Version " PRODUCT_VERSION_STR;

	ptheApp->PrintText( vstr.GetBuffer(0) );

	ptheApp->PrintText( "INIT: Decoding XML files..." );
    AppData  = new CWidgieXML(ptheApp->cfgLanguage);
	AppData->m_currentDownloaded = 0;
	AppData->SetImpressionList( 0 );
    AppData->ParseAll();
	ptheApp->PrintText( "INIT: Decoding XML files Done" );
	    
    /* Later when the list is downloaded and parsed again or when the
       list is accessed this critical section will have to be entered
       before anything else can be done */
    InitializeCriticalSection(&playlistSection);
    InitializeCriticalSection(&statusSection);
    InitializeCriticalSection(&loadimageSection);
	OutDebugs( "INIT: CRITICAL SECTION, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

    imageCount = 0;

	ptheApp->PrintText( "INIT: Initing video dialogs..." );
	// create  movie dialog... but hide it 
	if( videoDlg.Create( IDD_VIDEO_DIALOG, NULL ) )
		ptheApp->PrintText( "INIT: Initing video dialogs... Ok" );
	else
		ptheApp->PrintText( "INIT: Initing video dialogs... Failed" );
	
	/* Display the first Image */
	//this->GetClientRect(&myRect);
	myRect.left = ptheApp->cfgLeft;
	myRect.top = ptheApp->cfgTop;
	myRect.right = ptheApp->cfgRight;
	myRect.bottom = ptheApp->cfgBottom;
	dialogDC = this->GetDC();

	SetBkColor( dialogDC->m_hDC, 0x00 );

    languageCount = 0;
    infoEnabled = TRUE;


	ptheApp->PrintText( "INIT: Starting NewsThread..." );
    // Start the newsbar thread, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_TIME_CRITICAL
    newsThread = AfxBeginThread( RUNTIME_CLASS(NewsBarThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED );			//RUNTIME_CLASS or AFX_THREADPROC
	if( newsThread)
	{
		((NewsBarThread *)newsThread)->pAppData = &AppData;
		((NewsBarThread *)newsThread)->lpCRITICAL_SECTION = &playlistSection;
		((NewsBarThread *)newsThread)->ResumeThread();

		//((NewsBarThread *)newsThread)->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);		//THREAD_PRIORITY_HIGHEST,REALTIME_PRIORITY_CLASS
		ptheApp->PrintText( "INIT: Starting NewsThread... Ok" );
	} else
		ptheApp->PrintText( "INIT: Starting NewsThread... Failed!" );


	OutDebugs( "INIT: Starting main image" );
	// ==============
	ptheApp->PrintText( "INIT: Setting JPEG Picture..." );
	updateAll = TRUE;
    // setup the jpeg display timers 
    JPEG_PlayLength = 6;
	PositionWindow();
	OutDebugs( "INIT: Load First JPEG" );

	//NextJPEG();
	NextJPEGThreaded();
	updateAll = TRUE;

	//Sleep(500);

	GetCurrentImpression();
	RecordEventLog( currentImpression );		// record event of image/movie shown.

	startedTime = CTime::GetCurrentTime();

	ptheApp->PrintText( "INIT: Starting timers..." );
	StartImageTimer();
    this->SetTimer(ONEMIN_TIMER, 60*1000, NULL);

	ptheApp->PrintText( "INIT: Main OnInitDialog complete." );

	ShowCursor( FALSE );

	OutDebugs( "INIT: Init Procedure Completed." );
	return TRUE;
}









/**
 *  Initialises the Main Dialog (JPEG dialog)
 *
 *  Sets up the initial state of the dialog and starts the
 *  timer displaying the newxt JPEG Image.
 *
 */

 BOOL CWidgieDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

	shuttingDown = FALSE;

	OutDebugs( "OnInitDialog - this wnd = %08lx", this->m_hWnd );

	CString	ourip, netName;
	int netstatus;

	netstatus = CheckNetAdaptors( &ourip, &netName );

	ptheApp->m_tempHD_average =
	ptheApp->m_tempHD = QueryHDDSmartTemp(0);

	startedTime = CTime::GetCurrentTime();
	CString msg;
	msg.Format( "Application %s v%s  Restarted (%s/%s), Startup sequence begun...", AfxGetAppName(), PRODUCT_VERSION_STR, ourip, netName );
	Log_App_Event( 0, msg );
	Log_App_Debug( msg );
	Log_App_Error( msg.GetBuffer(0) );

	this->SetPicture(NULL);

	ptheApp->cfgExitWhenIdle =
	ptheApp->cfgShutdownWhenIdle =
	ptheApp->cfgRebootWhenIdle = 0;

	downloadConfigBinaries = FALSE;

	onpaintDrawing = 0;
	loadingImpression = 0;
	adminMenu = 0;
	statusInfo = 0;

	mute_status = FALSE;

    imageCount = 0;

	brightnessLevelLow = FALSE;

	AppData = NULL;
    languageCount = 0;

    alreadyDownloading =
	flashMovie.moviePlaying = FALSE;
	videoDlg.moviePlaying = FALSE;

	m_cursor = NULL;

	debugMessages = FALSE;

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ptheApp->m_mainDlg = this;

	g_logdebug = FALSE;

	if( strstr( AfxGetApp()->m_lpCmdLine,"logdebug" ) )
	{
		g_logdebug = TRUE;
	} else
	if( strstr( AfxGetApp()->m_lpCmdLine,"/p" ) )			// exit
	{
		return false;
	} else
	if( strstr( AfxGetApp()->m_lpCmdLine,"/c" ) )			// bring up config dialog
	{
		g_logdebug = TRUE;
	}

	// initialize all IO devices and start timers...
	InitApplication();

	if( CFG->cfgEnableHttpListen )
	{
		InitHttpServer( 80 );
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}






// ----------------------------------------------------          MAIN         -----------------------------------------------------
int CWidgieDlg::PositionWindow(void)
{
	// move window to top/left coord so we can see it
	this->MoveWindow(ptheApp->cfgLeft, ptheApp->cfgTop, 
				ptheApp->cfgWidth()+1, ptheApp->cfgBottom, TRUE);
	return 0;
}


// ################################################################################################################


void CWidgieDlg::ShowDialog( BOOL showLogo )
{
	OutDebugs( "CWidgieDlg - ShowDialog" );

	RECT rc = { ptheApp->cfgLeft,  ptheApp->cfgTop,
				ptheApp->cfgRight, ptheApp->cfgBottom };


	MoveWindow( ptheApp->cfgLeft,  ptheApp->cfgTop,
		        ptheApp->cfgWidth(), ptheApp->cfgHeight(), FALSE );

	ShowWindow(SW_NORMAL);

	InvalidateRect( &rc, FALSE );
}




// This gets called when we exit so we can cleanup after our mess ;)
void CWidgieDlg::ExitCleanup(void)
{
	OutDebugs( "MainDlg::ExitCleanup" );

	shuttingDown = TRUE;

	if( alreadyDownloading )
	{
		OutDebugs( "MainDlg: File is downloading, waiting till its stopped to exit" );

		int t_1 = timeGetTime();
		while( alreadyDownloading && (timeGetTime()-t_1) < 5000 )				// TIMEOUT after 5 seconds
			Sleep( 10 );
	}

	// check to see if we have any remaining events to upload
	if(  CFG->cfgIPandPort.IsEmpty() == false && m_EventLogList.GetCount() > 0 )
	{
		if( AppData->PostEventLog( &m_EventLogList ) >0 )
			m_EventLogList.RemoveAll();
	}

	// close the 'change login' details window
 	if( loginDlg.m_hWnd != NULL )
	{
		loginDlg.CloseWindow();
		loginDlg.DestroyWindow();
	}

	CString msg;
	msg.Format( "MainDlg: Application %s v%s  Shutting down", AfxGetAppName(), PRODUCT_VERSION_STR );
	Log_App_Event( 0, msg );
	Log_App_Error( msg.GetBuffer(0) );


	// stop the movies if we hit EXIT
	if( moviePlaying() )
	{
		OutDebugs("MainDlg: Stopping movie.");
		StopMoviePlayback();
//		Sleep( 2000 );
	}

	OutDebugs("MainDlg: Stopping timers.");
	/* Kill all the timers so no code runs so the OS can clean up */
    StopTimers();
	this->KillTimer(ONEMIN_TIMER);

	if( AppData )
	{
		OutDebugs("MainDlg: Closedown.");
		AppData->CloseDown();
		FreeAllLists();
	}

	if( loadingImpression )
	{
		OutDebugs( "MainDlg: Waiting for loading impression to end..." );
		int t_1 = timeGetTime();
		while( loadingImpression && (timeGetTime()-t_1) < 3000 )			// TIMEOUT after 15 seconds
			Sleep( 10 );
	}

	if( newsThread )
	{
		OutDebugs( "Main Dialog - Deleting newsbar thread" );
		((NewsBarThread *)newsThread)->Shutdown();
		delete newsThread;
		OutDebugs( "Main Dialog - Deleted newsbar" );
		newsThread = NULL;
	}
    
	if( AppData )
	{
		OutDebugs( "Main Dialog - Deleting AppData" );
		Sleep(1);
		delete AppData;
	}

	DestroyIcon( m_hIcon );

	if( dialogDC ) {
		ReleaseDC( dialogDC );
	}

	OutDebugs( "Main Dialog - ExitCleanup Done." );
}


void CWidgieDlg::OnClose()
{
	OutDebugs( "CWidgieDlg OnClose called" );
	CDialog::OnClose();
}



/**
  * Handle updating (rendering) of the JPEG images.
  *
  * Updated the section of the screen that the news bar erases.
  *
  * If the whole images needs to be rendered, the 'updateAll' 
  * member of this class needs to be set to true.
  *
  */

void CWidgieDlg::OnPaint() 
{
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
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}

	CPaintDC dc(this); // device context for painting
	CRect rect;
	GetClientRect(&rect);

	CRect updateRect;
	GetClientRect(&updateRect);
	
	int showResult;
	if( updateAll && !adminMenu  )
	{
		CDC* pContentDC;
		pContentDC = GetDC();

		if( pContentDC )
		{
			if( pPicture && pPicture->IsNull() == FALSE && pPicture->GetWidth() > 0 )
			{
		        updateAll = FALSE;
				onpaintDrawing = TRUE;

				if( brightnessLevelLow )
				{
					RECT destRC = { 0,0,DEFAULT_SCREEN_W,DEFAULT_SCREEN_H };
					destRC.bottom = CFG->cfgHeight();
					destRC.right = CFG->cfgWidth();
					pContentDC->FillSolidRect( &destRC, 0x0 );
					showResult = pPicture->AlphaBlend( pContentDC->m_hDC, destRC, destRC, 170, AC_SRC_OVER );
				} else
				{
					//showResult = pPicture->Draw( pContentDC->m_hDC, myRect );
				}
				onpaintDrawing = FALSE;
			}

			//pPicture->ShowSection(pContentDC, rect, updateRect);
			
			ReleaseDC(pContentDC);
		}
	}
	else
	{
		updateRect.top = ptheApp->cfgBottom - ptheApp->cfgMeterHeight;
	}
}


void CWidgieDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	static int showwindow = 0;

	// TODO: Add your message handler code here
    CRect myRect;
    CDC* dialogDC;

    GetClientRect(&myRect);
    dialogDC = GetDC();

    CBrush bgBrush;
    bgBrush.CreateSolidBrush(ptheApp->cfgBackgroundColor);
    dialogDC->FillRect(&myRect, &bgBrush);


	ReleaseDC(dialogDC);

//	ShowCursor( FALSE );
}




// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWidgieDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CWidgieDlg::DoScrollerUpdate(void) 
{
	internalTimerProc( 0, 0, (DWORD)&((NewsBarThread *)newsThread)->CNewsBar, 0, 0 );
}



void CWidgieDlg::OnMouseMove( UINT flags, CPoint point )
{
	static CPoint last_point;
	static DWORD lastt;

	{
		if( adminMenu && ( timeGetTime()-lastt > 10 ) )
		{
			//OutDebugs( "Y=%d, lastY=%d", point.y, last_point.y );
			if( point.y - last_point.y > 1 )
			{
				ptheApp->DoButtonCommand( BUTTONKEY_FORW );
				last_point = point;
			} else
			if( point.y - last_point.y < -1 )
			{
				ptheApp->DoButtonCommand( BUTTONKEY_BACK );
				last_point = point;
			}
		}
		lastt = timeGetTime();

	}
}


// thread to free all images in the background....
UINT CWidgieDlg::PlayBlankSoundThread(LPVOID param)
{
	AudioPlayResource( "IDR_BLANK" );
	return 1;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Our Super Timer, with lots of triggers etc...
// * image change timer
// * download news and data timers
// * shutdown/exit timers...
// * voice handeling timers...
// * status update timers
// * auto brightness changing timers
//
//
//
//
void CWidgieDlg::OnTimer(UINT nIDEvent) 
{
	this->SetFocus();

	
	//OutDebugs( "%d : %d OnTimer triggered", timeGetTime(), nIDEvent );

	// proceed if no movie is playing.... during movie we wont do anything here
	if( nIDEvent == IMAGE_TIMER &&
		(!moviePlaying()) &&
		timeGetTime() > imageNext_t
		)
	{
		static long gImageCounter = 0;

		gImageCounter++;			// total views increment

		GetCurrentImpression();
		IncImpressionViews();

		ImpList *impList = AppData->GetImpressionList();	// get current list
		ImpressionData nextImpression;

		if( impList->GetCount() > 0 )
		{
			POSITION newPos = impList->FindIndex(imageCount) ;
			nextImpression = impList->GetAt(newPos);
			CString fileToShow;
			fileToShow = ptheApp->cfgLocalBaseDir + ptheApp->cfgLocalContentDir + nextImpression.m_image_file;
			imageNext_t = timeGetTime() + (nextImpression.m_playlength * 1000);
		} else
		{
			OutDebugs( "No slides to show...  lets sleep a bit" );
			DisplayScreenMessage( "No slideshow available, trying default" );
			Sleep( 2000 );
		}

		//OutDebugs( "#### IMAGE COUNT = %d", imageCount );
		// if we are at the end of the slideshow and the flag to reset the playlist is true...
		if( imageCount == 0 && gImageCounter >= impList->GetCount() )
		{
			DisplayScreenMessage( "Slideshow has restarted now." );

			OutDebugs( "#### AT THE END of playlist" );
			if( resetImpressions )
				ResetPlaylist();

			// amount of times slideshow looped
			m_slideshowLoopCount++;
		}

		RecordEventLog( nextImpression );		// record event of image/movie shown.
		// ###################################################
		// NEXT SLIDESHOW ITEM
		// If the next slide contains a MOVIE, play it, other wise just show the still image.
		if( nextImpression.m_multimedia_file.IsEmpty() == FALSE )
		{
			imageCount++;
			GetCurrentImpression();

			OutDebugs( "%d : #### Playing next movie", timeGetTime() );
			// try playing movie, if it failed, show the image.
			int playstatus = DoShowNextMovie( currentImpression );

			// failed to play movie, lets try to force it to download.
			if( playstatus == -1 )
			{
				ResetLastDownloadTime();
				NextJPEGThreaded();			// and show the image in the mean time.
			}
		} else 
		{
			NextJPEGThreaded();
		}
		// ###################################################



		ShowCursor( FALSE );


		// flag it when we are at the end of the slideshow so we can do stuff.
		BOOL endofcycle = FALSE;
		if( (imageCount+1) == impList->GetCount() )
			endofcycle = TRUE;

		// Skin any and all internet connectivity if we are told not to connect (100% slave mode)
		if( ptheApp->cfgNoInternet == false )
		{
			// #####  DOWNLOAD TRIGGERS  #######
			CTime currentTime = CTime::GetCurrentTime();
			/* If it is time to download the next lot of XML files do it */
			if( (currentTime >= (ptheApp->cfgNewsLastDownloadTime + ptheApp->cfgNewsDownloadInterval) || endofcycle )
				&& (!alreadyDownloading))
			{
				DownloadNewsFlashContent();
			}
			/* If it is time to download the next lot of XML files do it */
			if( (currentTime >= (ptheApp->cfgXMLLastDownloadTime + ptheApp->cfgXMLDownloadInterval) || endofcycle )
				&& (!alreadyDownloading))
			{
				DownloadXMLContent();
			}
			/* If it is time to download the next lot of MEDIA files do it */
			if( (currentTime >= (ptheApp->cfgLastDownloadTime + ptheApp->cfgDownloadInterval) || endofcycle )
				&& (!alreadyDownloading))
			{
				DownloadMediaContent();
			}
		}

		// ############## SLEEP MODE CHECKS ###############
		PerformSleepModeCheck();			// check to see if we are to make the screen sleep/off


	}// if ( !flashMovie.moviePlaying )
	else
	// backup timer for scroller....
	if( nIDEvent == SCROLL_TIMER && timerActive && !moviePlaying() )
	{
		DoScrollerUpdate();
	}
	else
	if( nIDEvent == STATUS_TIMER && statusInfo )
	{
		if( !shuttingDown )
			AdminShowStatus();
	}
	else
	if( nIDEvent == ONEMIN_TIMER )				// log cell info if changed since last..., every 60seconds...
	{
		CTime currentTime = CTime::GetCurrentTime();

		// ########## GET HARDWARE STATUS
		ptheApp->m_tempHD = QueryHDDSmartTemp(0);
		if( ptheApp->m_tempHD > 0 && ptheApp->m_tempHD < 100 )		// only accept sane values
		{
			for( int lp=HD_TEMP_COUNT-1;lp >0; lp-- )
			{
				ptheApp->m_tempHD_history[lp] = ptheApp->m_tempHD_history[lp-1];
			}

			// retrieve HD temp
			ptheApp->m_tempHD_history[0] = ptheApp->m_tempHD;
		
			int temp = 0, lp = 0;
			// calc rolling average for 10mins.
			for( lp=0;lp < 10 && ptheApp->m_tempHD_history[lp]; lp++ )
			{
				temp += ptheApp->m_tempHD_history[lp];
			}
			ptheApp->m_tempHD_average = temp/(lp);

			// HD temperature warning
			if( ptheApp->m_tempHD_average > 60 )
			{
				OutDebugs( "ERROR: Warning, HD temp very high at %d'c", ptheApp->m_tempHD_average );
			}
		}

		// ####### Upload Timer Trigger CHECK
		if( CFG->cfgIPandPort.IsEmpty() == false &&
			currentTime > ptheApp->cfgLastUploadTime &&
			m_EventLogList.GetCount() > 50 )
		{
			ptheApp->cfgLastUploadTime = currentTime + ptheApp->cfgUploadInterval;
			OutDebugs( "Next update time is %s", ptheApp->cfgLastUploadTime.Format( "%m-%d %H:%M:%S" ).GetBuffer(0) );

			if( AppData->PostEventLog( &m_EventLogList ) >0 )
			{
				m_EventLogList.RemoveAll();
			}
		}


		// ####### RESTART APPLICATION CHECK
		if( ptheApp->cfgRestartTime.GetLength() >0 && !shuttingDown )
		{
			CTime t = TimeDateToCTime( ptheApp->cfgRestartTime, currentTime.Format("%Y-%m-%d") );

			if( currentTime >= t && currentTime < (t+120) )
			{
				OutDebugs( "STATUS: Auto restarting at %s", ptheApp->cfgRestartTime.GetBuffer(0) );
				ChooseAdminMenu( 21 );			// quit to shell so it restarts.
			}
		}

		// ######## CHECK for local proxy server (5min interval)
		if( CFG->cfgIPandPort.IsEmpty() == false && currentTime > m_lastProxyTime )
		{
			ptheApp->CheckLocalServer();

			if( ptheApp->cfgLocalServerReachable )
				m_lastProxyTime = currentTime + (15*60);			// check less often if we found it
			else
				m_lastProxyTime = currentTime + (5*60);				// but check more often if we havent
		}
	}

    /* Defualt OnTimer handle for Windows */
    CDialog::OnTimer(nIDEvent);
}


int CWidgieDlg::UpdateDownloadTime( int updateType )
{
	CTime currentTime = CTime::GetCurrentTime();

	if( updateType == 0 )
		ptheApp->cfgLastDownloadTime = currentTime;
	else
	if( updateType == 1 )
		ptheApp->cfgNewsLastDownloadTime = currentTime;
	else
	if( updateType == 2 )
		ptheApp->cfgXMLLastDownloadTime = currentTime;

	/* Also write the last download time and date to the ini file */
	int i = 0;
	char strCurrentDate[10] = {'\0'};
	

	CString currentTimeString = currentTime.Format( "%H:%M:%S" );
        

	i = 0;
	itoa(currentTime.GetYear(), &strCurrentDate[i], 10);
	i = strlen(&strCurrentDate[0]);
	strCurrentDate[i] = '-';
	i++;
	itoa(currentTime.GetMonth(), &strCurrentDate[i], 10);
	i = strlen(&strCurrentDate[0]);
	strCurrentDate[i] = '-';
	i++;
	itoa(currentTime.GetDay(), &strCurrentDate[i], 10);
	

	if( updateType == 0 )
	{
		WritePrivateProfileString("DOWNLOAD","LastMediaDownloadTime",	currentTimeString.GetBuffer(0), INI_FILE);
		WritePrivateProfileString("DOWNLOAD","LastMediaDownloadDate",	strCurrentDate,	INI_FILE);
	} else
	if( updateType == 1 )
	{
		WritePrivateProfileString("DOWNLOAD","LastNewsDownloadTime", currentTimeString.GetBuffer(0), INI_FILE);
		WritePrivateProfileString("DOWNLOAD","LastNewsDownloadDate", strCurrentDate, INI_FILE);
	} else
	if( updateType == 2 )
	{
		WritePrivateProfileString("DOWNLOAD","LastXMLDownloadTime", currentTimeString.GetBuffer(0), INI_FILE);
		WritePrivateProfileString("DOWNLOAD","LastXMLDownloadDate", strCurrentDate, INI_FILE);
	}

	return 0;
}




char *LoadWavIntoRam( CString fn )
{
	BOOL bResult = FALSE;
	CFileException e;
	CFile soundFile;
	char * ram = NULL;

	// try the new button files 1....4
	bResult = soundFile.Open( fn.GetBuffer(0), CFile::modeRead | CFile::typeBinary, &e);

	if( bResult )
	{
		int nSize = (int)soundFile.GetLength();
        
		ram = (char*)malloc( nSize+4 );				// removed new char[x] because it crashed, aint got time to know why

		if( ram )
		{
			if( soundFile.Read(ram, nSize) > 0 )
			{
				bResult = TRUE;
			}
		}
		soundFile.Close();
	}
	return ram;
}


// Load next image, and display on screen.
BOOL CWidgieDlg::LoadImpression()
{
	return  LoadImpressionImage();
 
}



void HalfBriteDraw( CImage *s1_image )
{
	int loop = 64; // frames

	//while( loop>0 )
	if( s1_image )
	{
		unsigned char *s1, *src1;
		long pitch = s1_image->GetPitch();


		src1 = (unsigned char*)s1_image->GetBits();

		for( int y=0; y<s1_image->GetHeight(); y++ )
		{
			s1 = src1;
			for( int x=0; x<s1_image->GetWidth(); x++ )
			{
				*s1 = (*s1 >>1) & 0x7f;	s1++;
				*s1 = (*s1 >>1) & 0x7f;	s1++;
				*s1 = (*s1 >>1) & 0x7f;	s1++;
			}
			src1 += pitch;
		}
		loop--;
	}
}



// display and control the debug admin menus
int CWidgieDlg::DisplayScreenMessage( CString message, long duration )
{
	JPEG_PlayLength = duration;

	if( message.GetLength()>0 )
	{
		CDC* pDC;

		while( onpaintDrawing )
			Sleep(1);

		pDC = GetDC();

		if( pDC && pDC->m_hDC )
		{
			RECT destRC = { 0,0,DEFAULT_SCREEN_W,DEFAULT_SCREEN_H };
			destRC.right = CFG->cfgWidth();
			destRC.bottom = CFG->cfgHeight();

			CBrush bgBrush;
			bgBrush.CreateSolidBrush( textColorBG );
			pDC->FillRect( &destRC, &bgBrush );
			pDC->SetBkColor( 0x00000000 );
			pDC->SetTextColor( 0xFFFFFF );
			pDC->SetBkMode( OPAQUE );

			HFONT
			textFont = CreateFont( 64,0, 0, 0,FW_NORMAL, FALSE, FALSE, FALSE,
			  				ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS,
							PROOF_QUALITY, DEFAULT_PITCH + FF_DONTCARE, "Arial");
	 
			HGDIOBJ old =
			SelectObject( pDC->m_hDC, textFont );

			DrawText( pDC->m_hDC, message.GetBuffer(0), message.GetLength(), &destRC, DT_CENTER );

			SelectObject( pDC->m_hDC, old );

			DeleteObject( textFont );
			ReleaseDC(pDC);
		}
	}
	this->SetFocus();
	return 0;
}



// Load next image, and display on screen.
BOOL CWidgieDlg::LoadImpressionImage()
{
    BOOL bResult = TRUE;

	if (TryEnterCriticalSection( &loadimageSection ) == TRUE )
	//if (1)
	{
		long t_1, t_jpeg = 0, t_jpeg2, t_jpeg3;
		CImage	*myImage = NULL;

		t_1 = timeGetTime();

		ImpList *impList = AppData->GetImpressionList();

		// if for some reason we have NO items to show... return
		if( impList->GetCount() == 0 )
			return FALSE;

		POSITION newPos = impList->FindIndex(imageCount) ;
		ImpressionData nextImpression = impList->GetAt(newPos);
		CString fileToShow;

		// if we are using a http url for a source image, we must instead use its hash string version to get the local version
		if( nextImpression.m_image_file.Find( "http:" ) >=0 )
		{
			fileToShow = ptheApp->cfgLocalBaseDir + ptheApp->cfgLocalContentDir;
			fileToShow.AppendFormat( "%08lx.jpg", CHashIt(nextImpression.m_image_file) );
		} else 
		{
			fileToShow = ptheApp->cfgLocalBaseDir + ptheApp->cfgLocalContentDir + nextImpression.m_image_file;
		}

		if( nextImpression.imageData )
		{
			myImage = (CImage*)nextImpression.imageData;
			OutDebugs( "Using cached image of %s", nextImpression.m_image_file.GetBuffer(0) );
		} else {
			myImage = new CImage();
			if( myImage )
			{
				if( CFG->cfgCacheSourceImages )
				{
					nextImpression.imageData = (void*)myImage;
					impList->SetAt(newPos, nextImpression);
				}
			} else
				OutDebugs( "err failed to make new CImage" );
		}

		StopTimers();

		if( myImage && myImage->IsNull() )
		{
			// try to load BMP image first before tring original JPG
			CString bmpFN = fileToShow;
			//bmpFN.Replace( ".jpg", ".bmp" );
			t_jpeg = timeGetTime();

			// ### LOAD BMP FIRST
			bResult = myImage->Load( bmpFN.GetBuffer(0) );		// if we cant load bmp version, try loading jpeg version
			t_jpeg = timeGetTime() - t_jpeg;

			OutDebugs( "Load Image slideid-%d imageid-%s (%s)", nextImpression.m_id, nextImpression.m_image_id, nextImpression.m_image_file );

			//OutDebugs( "LoadImpressionImage - Load BMP (%ldms) %s", t_jpeg, bmpFN.GetBuffer(0) );

			if( bResult != S_OK )
			// load JPEG image.... and save BMP version  , this will later be done in the downloader
			{
				//OutDebugs( "LoadImpressionImage - Load JPEG %s", fileToShow.GetBuffer(0) );
				t_jpeg2 = timeGetTime();
				bResult = myImage->Load( fileToShow.GetBuffer(0) );

				if( bResult != S_OK )
				{
					fileToShow = DecreaseFilenameVersion( fileToShow );
					bResult = myImage->Load( fileToShow );
				}

				if( bResult == S_OK )
				{
					//myImage->StretchBlt( destdc, destrect, srcrect, SRCCOPY );
					// optionally save an intermediate BMP cached file for fast re-load on slow-ass computers without jpeg decomp
					if( CFG->cfgSaveBMPCache )
					{
						OutDebugs( "LoadImpression - Save BMP %s", bmpFN.GetBuffer(0) );
						myImage->Save( bmpFN.GetBuffer(0) );
					}
					t_jpeg2 = timeGetTime() - t_jpeg2;
					//OutDebugs( "JPEG - time to myImage->Load/Save to BMP = %d ms", t_jpeg2 );
				}
				else
				{
					OutDebugs( "ERROR: Failed to load impression image %s", fileToShow.GetBuffer(0) );
					Sleep( 200 );

					// force a download if no file found.
					ResetLastDownloadTime();
				}
			}
		} else
			bResult = S_OK;

		if (bResult == S_OK && myImage->IsNull()==FALSE )
		{
			JPEG_PlayLength = nextImpression.m_playlength;
			//if ( JBuffer == NULL )
			{
				CDC* pDC;

				while( onpaintDrawing )
					Sleep(1);

				pDC = GetDC();
				t_jpeg3 = timeGetTime();

				if( pDC && pDC->m_hDC )
				{
					RECT destRC = { 0,0,DEFAULT_SCREEN_W,DEFAULT_SCREEN_H };
					destRC.bottom = CFG->cfgHeight();
					destRC.right = CFG->cfgWidth();

					RECT srcRC = { 0,0,800,600 };
					srcRC.bottom = myImage->GetHeight();
					srcRC.right = myImage->GetWidth();

					if( CFG->cfgImageBlend )
					{
						for( int lp=0; lp<150; lp+=10 )
						{
							myImage->AlphaBlend( pDC->m_hDC, destRC, srcRC, lp, AC_SRC_OVER );
							//myImage->AlphaBlend( pDC->m_hDC, destRC, destRC, lp, AC_SRC_OVER );
							Sleep( 35 );
						}
					}
					//else
					{
						//myImage->Draw( pDC->m_hDC, destRC );
						//myImage->StretchBlt( pDC->m_hDC, destRC, srcRC, SRCCOPY );
						
						//pContentDC->FillSolidRect( &destRC, 0x0 );
						//showResult = pPicture->AlphaBlend( pContentDC->m_hDC, destRC, destRC, 170, AC_SRC_OVER );

						if( myImage->GetHeight() > CFG->cfgBottom )
							myImage->AlphaBlend( pDC->m_hDC, destRC, srcRC, 255, AC_SRC_OVER );
						else
							myImage->StretchBlt( pDC->m_hDC, destRC, srcRC, SRCCOPY );

						SetPicture( myImage );
					}

					// resize smoothly to a smaller sized image.
/*					RECT sRC = { 0,0,0,0 };
					sRC.bottom = myImage->GetHeight();
					sRC.right = myImage->GetWidth();
					myImage->AlphaBlend( pDC->m_hDC, destRC, sRC, 0xff, AC_SRC_OVER );
					//myImage->Draw( pbDC->m_hDC, destRC );
					SetPicture( myImage );
*/
					OutDebugs( GetLastErrorString() );

					t_jpeg3 = timeGetTime() - t_jpeg3;
					ReleaseDC(pDC);

					if( CFG->cfgCacheSourceImages == false )
					{
						myImage->Destroy();
						delete myImage;
					}
				}

				bResult = TRUE;
			}
			//OutDebugs( "JPEG - time to myJPEG.Draw = %d ms", t_jpeg3 );

			//  Because all user input is done in this window lets make sure we have focus at all times
			this->SetFocus();
		} // if (bResult == TRUE)
		else
		{
			myImage->Destroy();
			delete myImage;
			nextImpression.imageData = NULL;

			SetPicture( NULL );
			Log_App_FileError(errMediaMissing, "Image Error", "Could not open the file: " + nextImpression.m_image_file );
			//Beep( 2DEFAULT_SCREEN_H, 60 );
			JPEG_PlayLength = 2;
		}
		DisplayStatusInfoLine();
		StartImageTimer();
		//OutDebugs( "JPEG - total time = %d ms, %d ms", t_jpeg3, timeGetTime()-t_1 );

		LeaveCriticalSection( &loadimageSection );
	} else {
		OutDebugs( "### loadimageSection busy...." );
		return 0;
	}


    return(bResult);
}



 /**
  * Retrives the next impression in the displaylist
  *
  * Copies the content of the next impression in the 
  * display list into the global data members:
  * 
  * myJPEG and movie
  *
  * NOTE: this isnt supposed to be re-entrant so only ONE instance at any time
  */

void CWidgieDlg::NextJPEG()
{
    if(	!moviePlaying() )
    {
        int numberOfImages = 0;
		ImpList *impList = AppData->GetImpressionList();

		if( impList && (unsigned int)impList != 0xfeeefeee ) {
	        numberOfImages = impList->GetCount();
		}

        if (numberOfImages > 0) // only look for and display the next image if it is actually there
        {
            if (imageCount >= numberOfImages)
            {
                imageCount = 0;
            }

			{
				BOOL imageOK;
				imageOK = LoadImpressionImage();

				if (imageOK == TRUE)
				{
					ShowWindow( SW_SHOW );
					imageCount++;
				}
				else // if there is a problem with the image file delete it from the display list and load the next one
				{
					OutDebugs( "Failed to LoadImpression, try next image..." );
					OutDebugs( "AppData->FreeImpressionImage( imageCount );" );
					SetPicture( NULL );
					AppData->FreeImpressionImage( imageCount );
					if( CFG->cfgPurgeMissingMedia )
					{
						impList->RemoveAt( impList->FindIndex(imageCount) );
					} else
						imageCount++;
				}
			}
        }
        else
        {
			DisplayScreenMessage( "No slideshow available, trying default" );

            /* Rebuild the impression list with the default loop sequence */
			OutDebugs( "Loading default local loop sequence file" );
            CString oldLoopFile = ptheApp->cfgLocalLoopName;
            ptheApp->cfgLocalLoopName = DEFAULT_LOOP;
            
			OutDebugs( "NextJPEG/EnterCriticalSection 1, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
            if( TryEnterCriticalSection(&playlistSection) )
			{
				OutDebugs( "NextJPEG/EnterCriticalSection 1, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
				OutDebugs( "Freeing All Impression Images in ram..." );
				SetPicture( NULL );
				FreeImpressionImageCache(TRUE);
				impList->RemoveAll();
				AppData->languageList.RemoveAll();
				AppData->ParseLoop( impList );
				LeaveCriticalSection(&playlistSection);
				OutDebugs( "LeaveCriticalSection 1, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
			}
            
            // Make the loop file the original so that when we get one  we can continue to use that one 
            
            ptheApp->cfgLocalLoopName = oldLoopFile;

            if (AppData->languageList.IsEmpty())
            {
                /* The loop sequence is empty. Display the logo */
                CString fileToShow = ptheApp->cfgLocalBaseDir + ptheApp->cfgLocalImagesDir + ptheApp->cfg_App_LogoFile;
				CImage myJPEG;

				if( myJPEG.Load(fileToShow) == S_OK )
				{
					//this->SetPicture(&myJPEG);
					if( CDC* pContentDC = GetDC() )
					{
						myJPEG.Draw( dialogDC->m_hDC, myRect );
						
						ReleaseDC( pContentDC );
					}

					JPEG_PlayLength = 3;
					StopTimers();
					StartImageTimer();
				} else
					OutDebugs( "ERROR: Failed to load image %s", fileToShow.GetBuffer(0) );
            }
            else
            {
                POSITION newPos = AppData->languageList.FindIndex(0);
                AppData->StrLanguage = AppData->languageList.GetAt(newPos);
            }
        }            
    } // if( !flashMovie.moviePlaying && !videoDlg.moviePlaying )          
}






/**
  * Retrives the previous impression in the displaylist
  *
  * Copies the content of the previous impression in the 
  * display list into the global data members:
  * 
  * myJPEG and movie
  *
  */

void CWidgieDlg::PreviousJPEG()
{
	// only do if there is no video playing
    if(	!moviePlaying() )
    {
        imageCount -= 2;

		ImpList *impList = AppData->GetImpressionList();

        int numberOfImages = impList->GetCount();

        if (numberOfImages > 0) // only look for and display the previous
                                // image if it is actually there
        {
            if (imageCount < 0)
            {
                imageCount = numberOfImages + imageCount;
            }
        
            LoadImpressionImage();

            imageCount++;
        }
    } //===>  if(!moviePlaying())
}


/** 
 *  Sets the pointer to the images file that is used for
 *  OnPaint Events
 *
 */

void CWidgieDlg::SetPicture(CImage* newPicture)
{
	pPicture = newPicture;
}


void CWidgieDlg::GetNextLanguage()
{

}



void CWidgieDlg::GetCurrentImpression(void)
{
	if( AppData )
	{
		ImpList *impList = AppData->GetImpressionList();

		int numberOfImages = impList->GetCount();

		if (imageCount >= numberOfImages)
		{
			imageCount = 0;
		}

		/* because the variable imageCount is incremented to the next impression
		when the impression is loaded we need to bet the previous one */

		int playingImageCount = imageCount - 1;

		if (playingImageCount < 0)
		{
			playingImageCount = numberOfImages + playingImageCount;
		}
	    
		currentImpressionPos = impList->FindIndex(playingImageCount);
	    
		if (currentImpressionPos != NULL)
		{
			currentImpression = impList->GetAt(currentImpressionPos);
		}
	}
}


// save the values from currentImpression to the list.
void CWidgieDlg::SetCurrentImpression(void)
{
    if (currentImpressionPos != NULL)
    {
		ImpList *impList = AppData->GetImpressionList();
        impList->SetAt( currentImpressionPos, currentImpression );
    }
}



void CWidgieDlg::IncImpressionViews(void)
{
    if (currentImpressionPos != NULL)
	{
		GetCurrentImpression();
		currentImpression.m_viewCount++;
		SetCurrentImpression();
	}
}




void CWidgieDlg::OnMButtonDown(UINT nChar, CPoint p) 
{
	if( !adminMenu  )
	{
		ptheApp->DoButtonCommand( 'M' );
	}
}


void CWidgieDlg::OnRButtonDown(UINT nChar, CPoint p) 
{
	if( !adminMenu )
	{
		DownloadXMLContent();
	}
}


void CWidgieDlg::OnLButtonDown(UINT nChar, CPoint p) 
{
	if( !adminMenu )
	{
		PlayButtonSoundThread( NULL );

		if( p.x < 10 && p.y < 10 )
		{
			ptheApp->appReturnCode = 1;
			PrintText( "Exiting now...." );
			SendMessage(WM_COMMAND, (WPARAM) IDCANCEL, 0);
		}
		else
		if( p.x > 790 && p.y < 10 )
		{
			ptheApp->appReturnCode = 0;
			PrintText( "Exiting now...." );
			SendMessage(WM_COMMAND, (WPARAM) IDCANCEL, 0);
		}
		else
		{
			DownloadMediaContent();
		}
	}
}




#define	DEC(x) --x; if (x<0) {x = 0;}
#define	INC(x) ++x; if (x>255) {x = 255;}







// thread to free all images in the background....
UINT CWidgieDlg::PlayButtonSoundThread(LPVOID param)
{
	if( param )
	{
		sndPlaySound( NULL, SND_NODEFAULT );
		if( *(long*)param )
			sndPlaySound( (LPCSTR)param, SND_ASYNC|SND_MEMORY );			//ptheApp->m_mainDlg->buttonSoundPtr[n]
		else
			AudioPlayResource( "IDR_BUTTON" );
		//PlaySound( KEYPRESS_SOUND, NULL, SND_FILENAME );
	}
	return 1;
}



void CWidgieDlg::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if( nChar == 13 )
		SendMessage( WM_KEYDOWN, nChar, 0 );			// simulate 'y' key for right button
}



void CWidgieDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if( CFG->cfgScreenSaverMode == FALSE )
	{
		GetCurrentImpression();

		switch( nChar )

		{
			// -----------------  button & keyboard commands ----------------
			case BUTTONKEY_FORW:
				OutDebugs( "Button Next Selected" );
				if( mute_status == FALSE )
					DoShowNext();
				break;

			case BUTTONKEY_BACK:
				OutDebugs( "Button Previous Selected" );
				if( mute_status == FALSE )
					DoShowPrevious();
				break;


			// -----------------  keyboard commands ----------------
			case 'C':
				InputLoginDetails();
				break;

			case 'Q':
				ChooseAdminMenu( 20 );
				break;

			case 'R':
				RestartImages();
				BringWindowToTop();
				break;

			case 'V':
				{
					CString vstr;
					vstr.Format( "Version %s , ID %s", PRODUCT_VERSION_STR, CFG->cfgID );
					PrintText( vstr.GetBuffer(0) );
				}
				break;

			case 'D':
				PrintText( "Downloading New Media Content..." );
				DownloadMediaContent();
				break;

			case 'U':
				PrintText( "Update stats data on server..." );
				if( AppData->PostEventLog( &m_EventLogList ) >0 )
				{
					m_EventLogList.RemoveAll();
				}
				break;

			case 'N':
				PrintText( "Downloading Latest News XML Content..." );
				DownloadNewsFlashContent();
				break;


			case 'X':
				OutDebugs( "Cancle Animation" );
				if( moviePlaying() )
				{
					OutDebugs( "Exiting movie playback" );
					StopMoviePlayback();

					//RestartImages();
					//NextJPEGThreaded();

					((NewsBarThread *)newsThread)->CNewsBar.ShowDialog( TRUE );
				} else 
				{
					PrintText( "Downloading Latest XML Content..." );
					ptheApp->cfgConfigVersion = "";
					DownloadXMLContent();
				}
				break;

			case 'P':
				debugMessages ^= 1;
				break;

			case 0xbe:
				ptheApp->cfgStepSpeed--; break;
			case 0xbc:
				ptheApp->cfgStepSpeed++; break;

			// Show Status
			case 'S':
				statusInfo ^= 1;
				ShowCursor( FALSE );
				break;

			// Show Slideshow List
			case 'L':
				AdminShowSlideshow();
				break;

			// Pause
			case ' ':
				PauseSystem();
				break;

			case 'H':
				{
					char txt[] = "\n\n\n\nKeys\n----\nSpace - Pause Image Cycling\n\n"
									//"M -  Admin Menu\n"
									"D - Download Missing Media content\n"
									"U - Update statistics data on server\n"
									"N - Download News XML content\n"
									"P - Debug Status on/off\n"
									"L - List slideshow on screen\n"
									"S - Status Info Display\n"
									"V - Version Number\n"
									"C - Change Login Details Dialog Box \n"
									"Q - Quit to Windows\n"
									"< > ... Scroller Speed Control\n";
					PrintText( txt );
				}
				break;


			case 'Z':
				{
					PrintText( "Reloading all XML/Image files..." );
					ResetPlaylist();
					GetCurrentImpression();
				}
				break;
			default:
#ifdef _DEBUG
				if( !adminMenu && debugMessages )
				{
					char debug_msg[256];
					sprintf( debug_msg, "                                  Key '%c' (0x%02x)    .", nChar, nChar );
					// Write the newsflash to the intermediate Image 
					PrintDebugText( debug_msg );
				}
#endif
				break;
		}
	} // else

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}


LRESULT CWidgieDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    if(message==WM_COMMAND)
    {
        if(wParam==IDOK)
			return 1;
		else
        if(wParam==IDCANCEL)
        {
			ExitCleanup();
        }
    }
   
	return CDialog::WindowProc(message, wParam, lParam);
}


void CWidgieDlg::StartNewsAndMeter()
{
	if( newsThread )
	{
	    ((NewsBarThread *)newsThread)->CNewsBar.ShowDialog();
		//OutDebugs( "MainDialog - StartNewsAndMeter" );
	}
}

void CWidgieDlg::StartImageTimer()
{
	imageNext_t = timeGetTime() + (JPEG_PlayLength*ONE_SECOND);		// set the time when to trigger next change
    this->SetTimer(IMAGE_TIMER, 200, NULL);
    this->SetTimer(STATUS_TIMER, 500, NULL);

	timerActive = TRUE;
}

void CWidgieDlg::StopTimers(void)
{
	timerActive = FALSE;
	this->KillTimer(IMAGE_TIMER);
	this->KillTimer(SCROLL_TIMER);
}






















// #####################################     DOWNLOAD STUFF     #######################################



// determine if we have a net connection and what type it is, then return appropriate value...
int CWidgieDlg::CheckNetConnection(void)
{
	// call global one that isnt inside a class ....
	return globalCheckNetConnection();

	int result;
	CString netip, netname;

	result = CheckNetAdaptors( &netip, &netname );

	if( ptheApp->cfgIgnoreEthernet && result == NETIS_LAN )
	{
		netname = "none";
		result = NETIS_NONE;
	}

	return result;
}



// ----------------------------------------------------
// Download the new multimedia content from which ever network source...
//
//
UINT CWidgieDlg::GetNewMediaContent(LPVOID pWidgieDialog)
{
	CWidgieDlg *dlg = (CWidgieDlg *)pWidgieDialog;
	CWidgieApp *cfg = dlg->ptheApp;

    UINT result = 0;

	// setup GPRS connection if there is NO WIFI or NO LAN internet
	int net_status = dlg->CheckNetConnection();

	// hopefully WIFI will auto select/start, so we either have wifi or lan automatically, otherwise
	// we just manually setup gprs (or adhoc P2P in future, that is T2T ;] )
	if( net_status == NETIS_NONE )
	{
		OutDebugs( "Scan for wifi now or ethernet..." );
		net_status = dlg->CheckNetConnection();
	}

	// only attempt to download if we have a net connection...
	if( net_status )
	{
		// ## only do this if we have a true ethernet or Wifi connection.
		if( net_status == NETIS_WIFI || net_status == NETIS_LAN )
		{
			// OBSELETE FOR THIS APPLICATION
			/*
			if( cfg->cfgManifestFileName.IsEmpty() == FALSE )
			{
				downloadsize =	dlg->AppData->DownloadXMLManifest();
				if( downloadsize >0 )
				{
					// ok now lets parse the download'ed manifest to decide what content to download...
					dlg->AppData->ParseManifest();
				} else
					OutDebugs( "ERROR: failed to download manifest file - (%s)", cfg->cfgManifestFileName );
				// --
			}
			*/

			// download new logos/news images... if a server is defined
			if( CFG->cfgIPandPort.IsEmpty() == false )
			{
				if( dlg->AppData->DownloadSupportImages( "newslogo" ) )
				{		
					((NewsBarThread *)dlg->newsThread)->CNewsBar.ReloadLogo();
					dlg->downloadConfigNewsLogo = FALSE;
				}

				if( dlg->AppData->DownloadSupportImages( "scrolllogo" ) )
				{
					dlg->downloadConfigNewsScrollLogo = FALSE;
				}

				//if( dlg->downloadConfigExe )
				// always check if to download or not.
				{
					if( dlg->AppData->DownloadNewExecuteable() > 0 )			// download new exe
						dlg->downloadConfigExe = FALSE;

					// perhaps restart if a new exe is downloaded.
				}
			}

			// download the image/multimedia data thats defined in the manifest, this must be done after parsing the manifest xml file
			// download new media binaries from the manifest list
			//dlg->AppData->DownloadContent();		

			long filesDone, filesTotal, filesSize;

			filesSize = dlg->AppData->DownloadMissingContent( &filesTotal, &filesDone );

			// download content that is missing from the internal slideshow
			if( filesSize >0 )
			{
				OutDebugs( "STATUS: Downloaded %d bytes of new media content, %d/%d files", filesSize, filesDone, filesTotal );
				// reset our playing loop sequence to include new impressions with new media....
				// this will also restart our hti counters.....
				dlg->ResetPlaylist();
			}


			//
			// delete content/ stuff that IS NOT in the manifest list, but only do this on a monday
			//CTime nowT = CTime::GetCurrentTime();
			//if( nowT.GetDayOfWeek() == 1 )
			//	dlg->AppData->DeleteUnusedContent();

			// update system ini file which contains the last download times...
			dlg->UpdateDownloadTime();
		} else
		if( net_status == NETIS_GPRS )
			OutDebugs( "Cannot download media content over GPRS connection" );
	} else
		OutDebugs( "Download Media failure, we have no network connection" );

	// finito
	dlg->alreadyDownloading = FALSE;
    return result;
}


// ----------------------------------------------------
// Download the new news content from which ever network source...
//
//
UINT CWidgieDlg::GetLatestNewsContent(LPVOID pWidgieDialog)
{
	CWidgieDlg *dlg = (CWidgieDlg *)pWidgieDialog;
	CWidgieApp *cfg = dlg->ptheApp;
	UINT result = 0;

	OutDebugs( "Downloading latest news content now..." );

	// setup GPRS connection if there is NO WIFI or NO LAN internet
	int net_status = dlg->CheckNetConnection();

	if( net_status == NETIS_NONE )
	{
		net_status = dlg->CheckNetConnection();
	}

	// only attempt to download if we have a net connection...
	if( net_status )
	{
		// download any xml news files from the server
		int downloadsize = 
		dlg->AppData->DownloadXMLNews();

		if( downloadsize > 0 )
		{
			// reparse the xml news files and load latest news into ram...
			dlg->ResetNewsflashlist();
		}

		if( dlg->alreadyDownloading != 2 )
			dlg->UpdateDownloadTime( 1 );

		OutDebugs( "Downloading latest news done." );
	} else
	{
		Log_App_Event( 0, "Downloading latest news didnt download, no network available." );
	}

	// ok we are done...
	dlg->alreadyDownloading = FALSE;

    return result;
}

// ----------------------------------------------------
// Download the new xml content from which ever network source...
//
//
UINT CWidgieDlg::GetNewXMLContent(LPVOID pWidgieDialog)
{
	CWidgieDlg *dlg = (CWidgieDlg *)pWidgieDialog;
	CWidgieApp *cfg = dlg->ptheApp;
    UINT result = 0;

	// setup GPRS connection if there is NO WIFI or NO LAN internet
	int net_status = dlg->CheckNetConnection();

	if( net_status == NETIS_NONE )
	{
		net_status = dlg->CheckNetConnection();
	}

	// only attempt to download if we have a net connection...
	if( net_status )
	{
		OutDebugs( "Downloading new xml content now..." );

		CString currentVersion = cfg->cfgConfigVersion;
		// Download all/new XML content.... (probly about 50-70kn max raw, though should be compressed either
		// in .xml.gz format, or all the files in ONE .xml.zip package.
		// check http://www.15seconds.com/issue/020314.htm , and http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=7164&lngWId=4
		// Data is downloaded in the following order ; XMLNews, LoopSequence, Config Settings, Manifest Download List...
		int downloadsize =
		dlg->AppData->DownloadXMLLoop();			// just get the slideshow, used to be dlg->AppData->DownloadXMLAll();

		// we need some extra checking in future to check if the new slideshow is
		// really changed, otherwise no need to reprocess it if its identical.
		// either an xml element detailing the last update time, or md5 sum of the file.

		if( downloadsize >0 )
		{
			// set a flag to signal it to reset the list at the end of the cycle, not now.
			dlg->resetImpressions = TRUE;

			// parse all xml data that was downloaded
			dlg->ResetPlaylist();			//dlg->ResetAllLists();

			// force a download of media content now since we have a new slideshow.
			dlg->ResetLastDownloadTime();
		}


		// Attempt to download a new config.xml file, the downloader will decide if it needs
		// to or not based on updated date/times.
		// OBSELETE : if( currentVersion.CompareNoCase( cfg->cfgConfigVersion ) )
		{
			// download a new config xml file if it has been updated...
			if( dlg->AppData->DownloadXMLConfig() >0 )
			{
				// parse our config settings, hopefully its all valid...
				dlg->AppData->ParseConfigDefaults();
				dlg->downloadConfigBinaries = TRUE;
			}
		}

		// update system ini file which contains the last download times...
		dlg->UpdateDownloadTime( 2 );
	} else
		OutDebugs( "ERROR: Download XML Files, we have no network connection" );

	// finito
	dlg->alreadyDownloading = FALSE;
    return result;
}











// ############## DOWNLOAD TRIGGERS

// download new xml/news content, and new media content too.
int CWidgieDlg::DownloadMediaContent(void)
{
	// if we are downloading, do nothing....
	if( alreadyDownloading == FALSE )
	{
		{
			alreadyDownloading = TRUE;

			OutDebugs( "Starting GetNewMediaContent thread...." );
			downloadThread = AfxBeginThread((AFX_THREADPROC)GetNewMediaContent, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL,0,0 );
		}
	}
	return 0;
}

// download new xml/news content, and new media content too.
int CWidgieDlg::DownloadXMLContent(void)
{
	// if we are downloading, do nothing....
	if( alreadyDownloading == FALSE )
	{
		if( CFG->cfgIPandPort.IsEmpty() == false )
		{
			alreadyDownloading = TRUE;

			OutDebugs( "Starting GetNewXMLContent thread...." );
			downloadThread = AfxBeginThread((AFX_THREADPROC)GetNewXMLContent, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL,0,0);
		}
	}
	return 0;
}

// download only new news content, nothing else....
int CWidgieDlg::DownloadNewsFlashContent( BOOL dontupdateTime )
{
	// if we are downloading, do nothing....
	if( alreadyDownloading == FALSE )
	{
		{
			if( dontupdateTime )
				alreadyDownloading = 2;
			else
				alreadyDownloading = 1;
	
			OutDebugs( "Starting GetLatestNewsContent thread...." );
			downloadThread = AfxBeginThread((AFX_THREADPROC)GetLatestNewsContent, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL,0,0 );
		}
	}
	return 0;
}

























// ############################### MOVIE MEDIA CONTROL


int CWidgieDlg::InitFlashMovie(void)
{
	if( flashMovie.m_hWnd == NULL )
	{
		flashMovie.Create(IDD_SHOCKWAVE_DIALOG, NULL);
		flashMovie.Hide();
		movieVisible = FALSE;
		return TRUE;
	} else
		return FALSE;
}


int CWidgieDlg::InitHttpWindow(void)
{
	if( HtmlWindow.m_hWnd == NULL )
	{
		HtmlWindow.Hide();
		return TRUE;
	} else
		return FALSE;
}

int CWidgieDlg::SetupMovie(void)
{
	/* load the movie/flash/shockwave media,  in case they want to watch it */
    CString movieToShow = ptheApp->cfgLocalBaseDir + ptheApp->cfgLocalContentDir + currentImpression.m_multimedia_file;

    /* test if the movie file exists */
    CFile mmfile;
    BOOL fileExists = mmfile.Open(movieToShow, CFile::modeRead);

	// if we cant find the media, then try pervious version of the file...
	if( fileExists == FALSE )
	{
		movieToShow = DecreaseFilenameVersion( movieToShow );			// decrement file version string
		fileExists = mmfile.Open(movieToShow, CFile::modeRead);			// open it again

		if( fileExists == FALSE )
		{
			movieToShow.Replace( ".swf", ".mpg" );
			fileExists = mmfile.Open(movieToShow, CFile::modeRead);			// open it again
		}
	}

	// ok its ready to load....
    if (fileExists == TRUE)
    {
		unsigned char HeaderBuffer[4] = {0};
		unsigned char ValidFlashHeader[4] = {'F','W','S',0};
		unsigned char ValidShockHeader[4] = {'C','W','S',0};
		unsigned char ValidMPEGHeader[4] = {0,0,0x01,0xBA};			// 00000000  00 00 01 BA 21 00 01 00 19 80 09 D5 00 00 01 BB    ...º!....€.Õ...»
		unsigned char ValidM2THeader[4] = {0x47,0x40,0x00,0x30};	// 00000000  47 40 00 30 A6 00 FF FF FF FF FF FF FF FF FF FF    G@.0¦.ÿÿÿÿÿÿÿÿÿÿ
		unsigned char ValidAVIHeader[4] = {'R','I','F','F'};		// RIFF
		unsigned char ValidASFHeader[4] = { 0x30,0x26,0xB2,0x75};	// ASF and WMV files.
		unsigned char ValidHTMLHeader[4] = { '<' };					// HTML Files
		

		mmfile.Read(&HeaderBuffer[0], 3);
        mmfile.Close();
        
        /* check if the file is indeed a flash file */
        if (*HeaderBuffer == *ValidFlashHeader || *HeaderBuffer == *ValidShockHeader )
        {
			movieType = MOVIE_FLASH;
			InitFlashMovie();
            flashMovie.movie.SetMovie(movieToShow);
			flashMovie.movie.SetQuality( 1 );
			flashMovie.movie.SetQuality2( "Medium" );
            flashMovie.movie.Rewind();
			flashMovie.PlayMovie();
			return TRUE;
        } else
        if (HeaderBuffer[0] == ValidHTMLHeader[0] && movieToShow.Find(".htm") >0 )
        {
			movieType = MOVIE_HTML;
			HtmlWindow.ShowPageURL( movieToShow );
			return TRUE;
        } else
        if (*HeaderBuffer == *ValidMPEGHeader ||
			*HeaderBuffer == *ValidAVIHeader ||
			*HeaderBuffer == *ValidM2THeader ||
			*HeaderBuffer == *ValidASFHeader ||
			movieToShow.Find( ".mpg" ) >0 ||
			movieToShow.Find( ".mp4" ) >0 ||
			movieToShow.Find( ".mkv" ) >0 ||
			movieToShow.Find( ".mpeg" ) >0 ||
			movieToShow.Find( ".m2v" ) >0 ||
			movieToShow.Find( ".vob" ) >0
			)
        {
			movieType = MOVIE_MPEGAVI;
			videoDlg.m_Path = movieToShow;
			videoDlg.OnPlay();
			return TRUE;
		}
		else
        {
			movieType = MOVIE_NONE;
			CString msg;
			msg.Format( "Invalid Multimedia file: %s (Header=%02x:%02x:%02x:%02x)", currentImpression.m_multimedia_file, HeaderBuffer[0], HeaderBuffer[1], HeaderBuffer[2], HeaderBuffer[3] );
            Log_App_FileError(errBadMedia, "Multimedia Error", msg );
			return FALSE;
        }
    } // if (fileExists == TRUE)
    else
    {
        Log_App_FileError(errMediaMissing, "Multimedia Error", "Could not open the file: " + currentImpression.m_multimedia_file);
		return -1;
    }
	return FALSE;
}



void CWidgieDlg::StopMoviePlayback(void)
{
	//OutDebugs( "Stopping Movie media playback" );

	if( moviePlaying() )
	{
		switch( movieType )
		{
			case MOVIE_FLASH:
				flashMovie.Hide();
				movieVisible = flashMovie.StopMovie();
				flashMovie.moviePlaying = FALSE;
				break;

			case MOVIE_MPEGAVI:
				BringWindowToTop();
				videoDlg.Hide();			// try to hide the main video dialog so its gone
				videoDlg.OnStop();			// then when we cant see it, stop it
				ShowNewsDialog();			// show our newsbar again
				break;

			case MOVIE_HTML:
				BringWindowToTop();
				HtmlWindow.Hide();
				HtmlWindow.Blank();
				ShowNewsDialog();			// show our newsbar again
				break;
		}
	}
	OutDebugs( "%d : Stopping Movie media playback - Completed", timeGetTime() );

	// go to next image before we see the old one...
	//NextJPEGThreaded();

	// make sure our news bar scrolls now.
	StartNewsAndMeter();


	updateAll = TRUE;
}


static BOOL pause_status = FALSE;

void CWidgieDlg::PauseMoviePlayback(void)
{
	if( moviePlaying() )
	{
		if( pause_status )
		{
			UnPauseMoviePlayback();
			pause_status = FALSE;
		} else
		{
			pause_status = TRUE;

			switch( movieType )
			{
				case MOVIE_FLASH:
					flashMovie.PauseMovie();
					break;
				case MOVIE_MPEGAVI:
					videoDlg.OnPause();
					break;
			}
		}
	}
}



void CWidgieDlg::UnPauseMoviePlayback(void)
{
	if( moviePlaying() )
	{
		if( pause_status )
		{
			pause_status = FALSE;

			switch( movieType )
			{
				case MOVIE_FLASH:
					flashMovie.PauseMovie();
					break;
				case MOVIE_MPEGAVI:
					videoDlg.OnPause();
					break;
			}
		}
	}
}



void CWidgieDlg::ShowNewsDialog(void)
{
	if( newsThread )
		((NewsBarThread *)newsThread)->CNewsBar.ShowDialog( FALSE ); 

}


int CWidgieDlg::HideNewsLogo(void)
{
	if( newsThread )
		((NewsBarThread *)newsThread)->CNewsBar.HideLogo();

	return 1;
}



/*
 * ######    DO INFO
 *
 * Show current impression on screen
 *
 *
 */

int CWidgieDlg::DoShowNextMovie( ImpressionData impression )
{
	CTime nowT = CTime::GetCurrentTime();
    
	// #############  BACK TO IMAGE CYCLING   ############
    if( moviePlaying() && (impression.m_voice_status != "1") )
    {
		if( debugMessages )
		{
			OutDebugs( "Exiting movie playback" );
			RestartImages();
			BringWindowToTop();

			StopMoviePlayback();
			StopTimers();
			((NewsBarThread *)newsThread)->CNewsBar.ShowDialog();
			//NextJPEGThreaded();
			StartNewsAndMeter();
		}
		return 0;
	} 
	else
	// ############  SHOW MOVIE ON SCREEN  ##########
    if( (!moviePlaying()) )
    {
		OutDebugs( "Showing Movie slideid-%d imageid-%s (%s)", impression.m_id, impression.m_image_id, impression.m_multimedia_file );

//		StopTimers();
//		((NewsBarThread *)newsThread)->CNewsBar.PauseScroller( 120 );

		int result = SetupMovie();

		if( result == -1 )
			return -1;
		
		m_infoTime = CTime::GetCurrentTime();		// now ignore info to call, with in 2/3 seconds of this time.

		if( result )
		{
			movieVisible = TRUE;
			((NewsBarThread *)newsThread)->CNewsBar.BringWindowToTop();
			//currentImpression.m_hitCount++;
			//SetCurrentImpression();
		} else // if movie fails, cont as normal, but make sure the newsbar is running
		{
			OutDebugs( "SetupMovie() failed to show, so lets start the scroller again..." );
			StartNewsAndMeter();
			return 0;
		}

		return 1;
    } // if ((infoEnabled) && (!flashMovie.moviePlaying ))
	return 0;
}

















int CWidgieDlg::DoShowNext(void)
{
    /* For statistical reasons */

	if( moviePlaying() )
	{
		if( debugMessages )
			StopMoviePlayback();		// only stop the movie if in debug mode when pressing buttons
		else
			return 0;
	} else {
		StopTimers();
		IncImpressionViews();
		NextJPEGThreaded();
	}

	return 0;
}

int CWidgieDlg::DoShowPrevious(void)
{
    /* For statistical reasons */

	if( moviePlaying() )
	{
		if( debugMessages )
			StopMoviePlayback();
		else
			return 0;
	} else {
		StopTimers();
		IncImpressionViews();
	    PreviousJPEGThreaded();
	}
	return 0;
}




BOOL CWidgieDlg::IsNetworkAvailable( int type ) 
{
	switch( type )
	{
		case NETWORK_LAN:
			return FALSE;
			break;

		case NETWORK_WIFI:
			return FALSE;
			break;

		default:
			return FALSE;
	}
}




void CWidgieDlg::DisplayMessageLine( CString message, int pausetime ) 
{
	(( NewsBarThread *)newsThread )->CNewsBar.displayCentered( message, pausetime );
}





// ######### JPEG IMAGE CHANGER CALLS


// thread that does the next/prev jpeg change....
UINT CWidgieDlg::JPEGImageThread(LPVOID param)
{
	if( AFXDIALOG->loadingImpression ) 
		OutDebugs( "JpegThread - busy... waiting until its free" );

	// prevent dual running here, so wait until the last call was completed
	while( AFXDIALOG->loadingImpression ) 
		Sleep(100);

	//OutDebugs( "JpegThread - begin" );

	// make sure the the playlist is not being accessed by any of the other	threads before showing the next JPEG
	if (TryEnterCriticalSection( &AFXDIALOG->playlistSection ) == TRUE )
	{
//		OutDebugs( "JpegThread - Inside CriticalSection, LockCount=%d, RecursionCount=%d", AFXDIALOG->playlistSection.LockCount, AFXDIALOG->playlistSection.RecursionCount );

		AFXDIALOG->loadingImpression = TRUE;

		int option = (int)param;
		if( option == 0 )
			AFXDIALOG->NextJPEG();
		else
		if( option == 1 )
			AFXDIALOG->PreviousJPEG();

		AFXDIALOG->loadingImpression = FALSE;
		LeaveCriticalSection( &AFXDIALOG->playlistSection );
		//ptheApp->SetAppPriority(THREAD_PRIORITY_ABOVE_NORMAL);		//THREAD_PRIORITY_BELOW_NORMAL
//		OutDebugs( "JpegThread - LeaveCriticalSection, LockCount=%d, RecursionCount=%d", AFXDIALOG->playlistSection.LockCount, AFXDIALOG->playlistSection.RecursionCount );
	} else
		OutDebugs( "JpegThread - Failed CriticalSection, LockCount=%d, RecursionCount=%d", AFXDIALOG->playlistSection.LockCount, AFXDIALOG->playlistSection.RecursionCount );

	//OutDebugs( "JpegThread - completed" );

	return 1;
}

// perform a next jpeg inside a thread.
int CWidgieDlg::NextJPEGThreaded(void)
{
//	while( loadingImpression )	{
//		Sleep(10);		OutDebugs( "loadingImpression = 1, sleeping..." );
//	}

	//if( loadingImpression == FALSE )
	{
//		OutDebugs( "Starting JPEGImageThread thread...." );
		jpegThread = AfxBeginThread( AFX_THREADPROC(JPEGImageThread), (LPVOID)0, THREAD_PRIORITY_BELOW_NORMAL, 0, 0 );
	}

	return 1;
}

// perform a previous jpeg inside a thread.
int CWidgieDlg::PreviousJPEGThreaded(void)
{
//	while( loadingImpression )	{
//		Sleep(10);		OutDebugs( "loadingImpression = 1, sleeping..." );
//	}
	//if( loadingImpression == FALSE )
	{
//		OutDebugs( "Starting JPEGImageThread thread...." );
		jpegThread = AfxBeginThread( AFX_THREADPROC(JPEGImageThread), (LPVOID)1, THREAD_PRIORITY_BELOW_NORMAL, 0, 0 );
	}
	return 1;
}

// -------------------- Image Refresh

int CWidgieDlg::RestartImages(void)
{
	OutDebugs( "Restart Images." );
	StopTimers();

	updateAll = TRUE;

	BringWindowToTop();

	//OnPaint();

	StartNewsAndMeter();
    StartImageTimer();

	if( newsThread )
		((NewsBarThread *)newsThread)->CNewsBar.ShowDialog();

//	if( newsThread )
//		((NewsBarThread *)newsThread)->CNewsBar.ShowLogo();


	return 0;
}

// -------------------- FREE IMAGE CACHE

// thread to free all images in the background....
UINT CWidgieDlg::FreeImageThread(LPVOID param)
{
	CWidgieApp *ptheApp = CFG;
	ptheApp->m_mainDlg->freeingCache_f = TRUE;

	//CImage **cacheP = (CImage **)param;
	CArray<CImage*> *cacheList = (CArray<CImage*> *)param;
	CImage *image;

	for( int i=0; i<cacheList->GetCount();i++ )
	{
		image = cacheList->GetAt(i);
		if( image )
		{
			//OutDebugs( "FreeImageThread... freeing image %d 0x%08lx", i, image );
			image->Destroy();

			delete image;
		}
	}
	//AppData->FreeImpressionImage( lp );

	cacheList->RemoveAll();

	delete cacheList;

	//OutDebugs( "Doing FreeImageThread...Done" );
	ptheApp->m_mainDlg->freeingCache_f = FALSE;
	return 1;
}








int CWidgieDlg::FreeImpressionImageCache( BOOL sync )
{
	ImpList *impList = AppData->GetImpressionList();

	if( impList && impList->GetCount() )
	{
		OutDebugs( "Freeing All Impression Images in ram (total=%d)...", impList->GetCount() );
		CArray<CImage*> *cacheList = new CArray<CImage*>;

		// place all cimages into the array
		for( int lp=0; lp<=impList->GetCount();lp++)
		{
			cacheList->Add( AppData->GetImpressionImage( lp ) );
		}

		OutDebugs( "Starting FreeImageThread thread..." );
		freeingCache_f = TRUE;
		freecacheThread = AfxBeginThread( AFX_THREADPROC(FreeImageThread), (LPVOID)cacheList, THREAD_PRIORITY_LOWEST, 0, 0  );

		if( sync )
		{
			while( freeingCache_f )			// wait till the thread is done...
				Sleep(1);
		}
	}

	return 0;
}





// post download, reset/reload all xml lists from disk to ram, and reparse everything like from the begining
int CWidgieDlg::FreeAllLists(void)
{
	if( TryEnterCriticalSection(&playlistSection) )
	{
		OutDebugs( "FreeAllLists/EnterCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

		// free all lists...
		for( int lang=0; lang < AppData->languageList.GetCount(); lang++ )
		{
			ImpList *impList = AppData->GetImpressionList(lang);
			FreeImpressionImageCache(TRUE);
			impList->RemoveAll();
		}

		AppData->newsflashList.RemoveAll();			//AppData->newsflashList.GetCount()
	    AppData->fileList.RemoveAll();
	    AppData->languageList.RemoveAll();

		LeaveCriticalSection(&playlistSection);        

		OutDebugs( "FreeAllLists/LeaveCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
	} else
		return 1;

	return 0;
}



// post download, reset/reload all xml lists from disk to ram, and reparse everything like from the begining
int CWidgieDlg::ResetAllLists(void)
{
    // re-build the display list for the current Language
	OutDebugs( "ResetAllLists/EnterCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

	if( TryEnterCriticalSection(&playlistSection) )
	{
		OutDebugs( "ResetAllLists/EnterCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

		// free/delete all impressions for all langs in memory
		{
			ImpList *impList = AppData->GetImpressionList(0);
			FreeImpressionImageCache(TRUE);
			impList->RemoveAll();
		}

		AppData->newsflashList.RemoveAll();
	    AppData->fileList.RemoveAll();

		// reload all xmls now...
		AppData->ParseAll();

		resetImpressions = FALSE;					// reset the flag to done - zero

		languageCount = 0;
		AppData->SetImpressionList( languageCount );

		LeaveCriticalSection(&playlistSection);        

		OutDebugs( "ResetAllLists/LeaveCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
	} else
		return 1;

	return 0;
}



// reset/reload only the play list data ,keep everything else untouched.
int CWidgieDlg::ResetPlaylist(void)
{
    // re-build the display list for the current Language
	OutDebugs( "ResetPlaylist/EnterCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

	if( TryEnterCriticalSection(&playlistSection) )
	{
		OutDebugs( "ResetPlaylist/EnterCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

		FreeImpressionImageCache(TRUE);
		ImpList *impList = AppData->GetImpressionList();
		if( impList )
		{
			impList->RemoveAll();
			AppData->ParseLoop( impList );
			resetImpressions = FALSE;				// reset the flag to done - zero
		}
		LeaveCriticalSection(&playlistSection);        

		OutDebugs( "ResetPlaylist/LeaveCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
	} else
		return 1;

	return 0;
}



// reset/reload only the news xml lists
int CWidgieDlg::ResetNewsflashlist(void)
{
    // re-build the display list for the current Language
	OutDebugs( "ResetNewsflashlist/EnterCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

	if( TryEnterCriticalSection(&playlistSection) )
	{
		OutDebugs( "ResetNewsflashlist/EnterCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

		AppData->newsflashList.RemoveAll();			//AppData->newsflashList.GetCount()
		AppData->ParseNews();
		LeaveCriticalSection(&playlistSection);        

		OutDebugs( "ResetNewsflashlist/LeaveCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
	} else
		return 1;

	return 0;
}





// ##################### ADMIN MENU  SYSTEM ###############################3


int CWidgieDlg::PrintText(char *text, int color )
{
	CDC* textDC  = this->GetDC();
	if( textDC && textDC->m_hDC )
	{
		RECT rc = { 00,00, DEFAULT_SCREEN_W, DEFAULT_SCREEN_H-45 };
		rc.bottom = CFG->cfgHeight();
		rc.right = CFG->cfgWidth();

		textDC->SetBkColor( textColorBG );
		if( color == -1 )
			textDC->SetTextColor( textColor );
		else
			textDC->SetTextColor( color );

		CBrush bgBrush;
		bgBrush.CreateSolidBrush( textColorBG );
//		textDC->FillRect(&rc, &bgBrush);

//		rc.bottom = 16*3;
		DrawText( textDC->m_hDC, text, strlen(text ), &rc, DT_LEFT );
		if( strstr( text, "ERROR:" ) )
		{
			StopTimers();
			StartImageTimer();
		}
		ReleaseDC( textDC );
	}
	return 0;
}


int CWidgieDlg::PrintDebugText(char *text,int color)
{
	if( (adminMenu || debugMessages) )
		PrintText( text,color );
	return 0;
}



int CWidgieDlg::PrintXY(int x, int y, char * text, ...)
{
	CDC* textDC  = GetDC();
	if( textDC && textDC->m_hDC )
	{
		RECT rc = { 00,00, DEFAULT_SCREEN_W-8, DEFAULT_SCREEN_H-40 };
		rc.left = x;
		rc.top = y;
		rc.bottom = CFG->cfgHeight()-8;
		rc.right = CFG->cfgWidth()-40;

		textDC->SetBkColor( textColorBG );
		textDC->SetTextColor( textColor );

		{
			int lines = 1;
			CBrush bgBrush;

			char *p = text;
			while( p && *p )
			{
				if( *p++ == '\n' )
					lines++;
			}
			bgBrush.CreateSolidBrush( textColorBG );
			RECT rc2 = { x,y, DEFAULT_SCREEN_W-8,y+(17*lines) };
			textDC->FillRect( &rc2, &bgBrush );
		}

		char lineout[4000];

		if ( text ){
			va_list		args;
			va_start( args, text);
			vsprintf( lineout, text, args );
			va_end( args );
//			strcat( lineout, "                                             " );
			DrawText( textDC->m_hDC, lineout, strlen(lineout), &rc, DT_LEFT );
			//TabbedTextOut( textDC->m_hDC, x, y, text, strlen(text ), 0, NULL, 0 );
		}
		ReleaseDC( textDC );
	}
	return 0;
}

int CWidgieDlg::PrintXYC(int x, int y, int col, char * text, ...)
{
	CDC* textDC  = GetDC();
	if( textDC && textDC->m_hDC )
	{
		RECT rc = { 00,00, DEFAULT_SCREEN_W-8, DEFAULT_SCREEN_H-40 };
		rc.left = x;
		rc.top = y;
		rc.bottom = CFG->cfgHeight()-8;
		rc.right = CFG->cfgWidth()-40;
		textDC->SetBkColor( textColorBG );
		textDC->SetTextColor( col );

		{
			CBrush bgBrush;
			bgBrush.CreateSolidBrush( textColorBG );
			RECT rc2 = { x,y, DEFAULT_SCREEN_W-8,y+17 };
			textDC->FillRect( &rc2, &bgBrush );
		}

		char lineout[4000];

		if ( text ){
			va_list		args;
			va_start( args, text);
			vsprintf( lineout, text, args );
			va_end( args );
//			strcat( lineout, "                                             " );
			DrawText( textDC->m_hDC, lineout, strlen(lineout), &rc, DT_LEFT );
			//TabbedTextOut( textDC->m_hDC, x, y, text, strlen(text ), 0, NULL, 0 );
		}
		ReleaseDC( textDC );
	}
	return 0;
}





/*
	textFont = CreateFont(ptheApp->cfgFontHeight,0, 0, 0,FW_NORMAL, FALSE, FALSE, FALSE,
			  		    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS,
					    PROOF_QUALITY, DEFAULT_PITCH + FF_DONTCARE, ptheApp->cfgFontFace);
 
    SelectObject(tempImage, newsFont);
*/


int CWidgieDlg::ChooseAdminMenu(int menuSelected)
{
	static int s_micstatus = 0;
	switch( menuSelected )
	{
		case 1: 	FreeImpressionImageCache(TRUE); break;
		case 2: 	ResetPlaylist(); break;
		case 3: 	DownloadNewsFlashContent();
					DownloadXMLContent();
					DownloadMediaContent();
					break;
		case 4: 	DownloadXMLContent(); break;
		case 5:		DownloadNewsFlashContent(); break;
		case 6:		AppData->DownloadMissingContent(NULL,NULL); break;
		case 7:		AppData->DeleteUnusedContent(); break;

		case 8: 	debugMessages ^= 1; 
					PrintDebugText( debugMessages ? "Debug is now ON        " : "Debug is now OFF       " ); 
					break;
		case 9:		debugMessages = 1; 
					//ptheApp->cfgUpdateExe = "Widgie.exe";
					DeleteFile( "Media\\widgie.exe" );
					AppData->DownloadNewExecuteable();			// download new exe, then flag it to quit/restart....
					debugMessages = 0;
					break;
		case 10:	ptheApp->cfgStepSpeed++;	ptheApp->cfgStepSpeed&=0x3; 
					break;

		case 11:	ptheApp->DoButtonCommand( 'X' );
					break;
		case 12:	ptheApp->DoButtonCommand( ' ' );
					break;

		case 17:	ptheApp->cfgUseRSSNews = TRUE; DownloadNewsFlashContent(); break;
		case 18:	PrintDebugText( "Renewing adaptors     " ); 
					system( "ipconfig /renew" ); 
					PrintDebugText( "Renewing adaptors completed.       " );
					break;
		case 19:	PrintDebugText( "Starting VNC Server     " ); 
					if( system( "C:\\Program Files\\TridiaVNC\\win32\\WinVNC.exe" ) == -1 ) 
					{
						if( system( "C:\\Program Files\\RealVNC\\WinVNC\\WinVNC.exe" ) == -1 )
							PrintDebugText( "VNC failed to Launched       " );
						else
							PrintDebugText( "VNC Launched       " );
					} else
						PrintDebugText( "VNC Launched       " );
					break;
		case 20: 	ptheApp->appReturnCode = 1;	SendMessage(WM_COMMAND, (WPARAM) IDCANCEL, 0); break;
		case 21: 	ptheApp->appReturnCode = 0;	SendMessage(WM_COMMAND, (WPARAM) IDCANCEL, 0); break;
		case 22: 	ptheApp->appReturnCode = 1;
					RebootNow();
					SendMessage(WM_COMMAND, (WPARAM) IDCANCEL, 0); 
					break;
		case 23:	Log_App_Event( 0, "Shutting down via admin control" );
					break;
		case 25:	return 1; break;
		case 99: 	ptheApp->appReturnCode = 3;	SendMessage(WM_COMMAND, (WPARAM) IDCANCEL, 0); break;
	}
	return 0;
}




// display and control the debug admin menus
int CWidgieDlg::DisplayAdminMenu(int menupart, int command)
{
	return 0;
}




#define	ADDTEXT	statusText += "\n"; statusText += txt; lines++;

CString CWidgieDlg::GetStatusText( int impnumber )
{
	CString statusText, txt;
	CString	ourip, netName, gateway;

	int netstatus;
	int lines = 0;

	if( shuttingDown == FALSE )
	{
		netstatus = CheckNetAdaptors( &ourip, &netName );

	
		CTime nowTime = CTime::GetCurrentTime();

		txt.Format( "Started on : %s, Clock=%s", startedTime.Format( "%Y-%m-%d %H:%M:%S" ), nowTime.Format( "%Y-%m-%d %H:%M:%S" ) );
		ADDTEXT

		txt.Format( "Last Media Download Time : %s", ptheApp->cfgLastDownloadTime.Format( "%Y-%m-%d %H:%M:%S" ) );
		ADDTEXT
		CTime next = (ptheApp->cfgLastDownloadTime + ptheApp->cfgDownloadInterval);
		txt.Format( "Next Media Download Time : %s", next.Format( "%Y-%m-%d %H:%M:%S" ) );
		ADDTEXT

		txt.Format( "Last XML Download Time   : %s", ptheApp->cfgXMLLastDownloadTime.Format( "%Y-%m-%d %H:%M:%S" ) );
		ADDTEXT
		next = (ptheApp->cfgXMLLastDownloadTime + ptheApp->cfgXMLDownloadInterval);
		txt.Format( "Next XML Download Time   : %s", next.Format( "%Y-%m-%d %H:%M:%S" ) );
		ADDTEXT

		txt.Format( "Last News Download Time  : %s", ptheApp->cfgNewsLastDownloadTime.Format( "%Y-%m-%d %H:%M:%S" ) );
		ADDTEXT
		next = (ptheApp->cfgNewsLastDownloadTime + ptheApp->cfgNewsDownloadInterval);
		txt.Format( "Next News Download Time  : %s", next.Format( "%Y-%m-%d %H:%M:%S" ) );
		ADDTEXT

		txt.Format( "Next Stats Upload Time   : %s (every %s)", ptheApp->cfgLastUploadTime.Format( "%Y-%m-%d %H:%M:%S" ),
			ptheApp->cfgUploadInterval.Format( "%H:%M:%S" ) );
		ADDTEXT



		txt.Format( "Last MDID Login Time     : %s", AppData->m_sessionStartTime.Format( "%Y-%m-%d %H:%M:%S" ) );
		ADDTEXT
		txt.Format( "Last Config Mod Time     : %s", AppData->m_configDateStamp );
		ADDTEXT


			

		if( alreadyDownloading && !AppData->currentDownloadFile.IsEmpty() ) 
		{
			txt.Format( "Downloading - %s, %d bytes", alreadyDownloading ? "YES" : "NO", AppData->m_currentDownloaded );
			ADDTEXT
			txt.Format( "  %s", AppData->currentDownloadFile );
			ADDTEXT
		} else
		if( !AppData->currentDownloadFile.IsEmpty() ) 
		{
			txt.Format( "Last Download - '%s'", AppData->currentDownloadFile );
			ADDTEXT
		}


		txt.Format( "Download Count : News=%d, Login=%d, Config=%d, Slideshow=%d, Media=%d ",
			AppData->m_downloadNewsCount, AppData->m_downloadLoginCount, AppData->m_downloadConfigCount,
			AppData->m_downloadSlideshowCount, AppData->m_downloadMediaCount );
		ADDTEXT

		FindNetworkAdaptors( "*", NULL, &gateway, NULL, NULL );
		txt.Format( "Network        : %s  (%s GW=%s)", netName.GetBuffer(0), ourip.GetBuffer(0), gateway.GetBuffer(0) );
		ADDTEXT

		txt.Format( "Local Name=%s, Server=(%s), Role=%s", ptheApp->cfgLocalName, ptheApp->cfgIPandPort,
			ptheApp->cfgLocalServer.IsEmpty()==TRUE ? "Local Proxy":"Client" );
		ADDTEXT

		txt.Format( "Local Server=(%s), %s",
			ptheApp->cfgLocalServer.IsEmpty() ? "None" : ptheApp->cfgLocalServer, 
			ptheApp->cfgLocalServerReachable==TRUE ? "Active":"unreachable" );
		ADDTEXT

		txt.Format( "Hard Disk Temp : %d'c (10min Avg=%d'c)", ptheApp->m_tempHD, ptheApp->m_tempHD_average );
		ADDTEXT

			txt.Format( "Scroller Time : %d ms", CFG->m_timetaken_ms );
		ADDTEXT

		ImpList *impList = NULL;
		if( AppData )
		{
			impList = AppData->GetImpressionList();

			txt.Format( "Slideshow - ID=%d , Last Modified = %s", AppData->m_slideshowID, AppData->m_slideshowDate );
			ADDTEXT
			txt.Format( "Slideshow - LockCount=%d RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
			ADDTEXT
			txt.Format( "Total Slides = %d, News Items = %d", impList->GetCount(), AppData->newsflashList.GetCount() );
			ADDTEXT
		}




		if( AppData )
		{
			ImpressionData impression;
			if( impnumber == -1 )			// if we ask for -1, we get the current impression, else we get what we asked for
			{
				GetCurrentImpression();
				impression = currentImpression;
			} else {
				POSITION newPos = impList->FindIndex(impnumber);
				if( newPos )
					impression = impList->GetAt(newPos);
			}

			txt.Format( "This Slide - #%d id=%d. %s  Views=%d", imageCount+1, impression.m_id, impression.m_title, impression.m_viewCount );
			ADDTEXT
			if( impression.m_multimedia_file.GetLength()>0 )
				txt.Format( " %s", impression.m_multimedia_file );
			else
				txt.Format( " %s", impression.m_image_file );
			ADDTEXT
		}

		while( lines++ < 28 )
		{
			statusText += "\n";
		}
	}

	return statusText;
}



//
//
// Display the whole slideshow in a nice list.
//
//
CString CWidgieDlg::Status_GetSlideShowsList( void )
{
	CString statusText, txt;
	int lines = 0;

	POSITION newPos;

	int index = 0;
	// go through all impressions, and check to see if any files are missing, if so , then download them
	while( newPos = AppData->m_slideshowList.FindIndex(index++) )
	{
		CString name = AppData->m_slideshowList.GetAt( newPos );

		txt.Format( "%d. %s", index, name );

		ADDTEXT
	}
	return statusText;
}



//
//
// Display the whole slideshow in a nice list.
//
//
CString CWidgieDlg::Status_GetEventList( void )
{
	CString statusText, txt;
	int lines = 0;

	POSITION newPos;

	int index = 0;
	// go through all impressions, and check to see if any files are missing, if so , then download them
	while( newPos = m_EventLogList.FindIndex(index++) )
	{
		CString name = m_EventLogList.GetAt( newPos );

		txt.Format( "%d. %s", index, name );

		ADDTEXT
	}
	return statusText;
}




//
//
// Display the whole slideshow in a nice list.
//
//
CString CWidgieDlg::Status_GetSlideshow( void )
{
	CString statusText, txt;
	int lines = 0;

	POSITION newPos;
	ImpList *impList = AppData->GetImpressionList();

	int index = 0;
	// go through all impressions, and check to see if any files are missing, if so , then download them
	while( newPos = impList->FindIndex(index++) )
	{
	    ImpressionData nextImpression = impList->GetAt(newPos);

		if( currentImpression.m_id == nextImpression.m_id )
			txt.Format( "*%d. (%d) %s : ",
				index,
				nextImpression.m_id,
				nextImpression.m_title );
		else
			txt.Format( "%d. (%d) %s : ",
				index,
				nextImpression.m_id,
				nextImpression.m_title );

		if( nextImpression.m_multimedia_file.GetLength() >0 )
			txt.Append( nextImpression.m_multimedia_file );
		else
			txt.Append( nextImpression.m_image_file );

		ADDTEXT
	}
	return statusText;
}


int CWidgieDlg::AdminShowSlideshow( void )
{
	int x = 12;
	int y = 8+(FONT_HEIGHT*3);

	if (TryEnterCriticalSection( &statusSection ) == TRUE )
	{
		CString statusMessage = Status_GetSlideshow();
		PrintXY( x, y+=FONT_HEIGHT, statusMessage.GetBuffer(0) );
		LeaveCriticalSection( &statusSection );
		return 0;
	}
	return 1;
}


int CWidgieDlg::AdminShowStatus( int impnumber )
{
	int x = 12;
	int y = 8+(FONT_HEIGHT*3);

	if (TryEnterCriticalSection( &statusSection ) == TRUE )
	{
		CString statusMessage = GetStatusText( impnumber );
		PrintXY( x, y+=FONT_HEIGHT, statusMessage.GetBuffer(0) );
		LeaveCriticalSection( &statusSection );
		return 0;
	}
	return 1;
}

int CWidgieDlg::DisplayStatusInfoLine(void)
{
    
	if( debugMessages )
	{
		char txt[256];
		sprintf( txt, "\nSTATUS: Downloading=%s , Printing=%s , Wifi=%s, playlistSection.Count=%d", 
			alreadyDownloading ? "YES" : "NO",
			printingDocket  ? "YES" : "NO",
			wifiInternet_f  ? "YES" : "NO",
			playlistSection.LockCount 
			);
		PrintDebugText( txt );
	}
	return 0;
}




// Ask user to enter new login details to MDID (plasmanet)
void CWidgieDlg::InputLoginDetails(void)
{
	if( loginDlg.m_hWnd == NULL )
	{
		ShowCursor( TRUE );
		ShowCursor( TRUE );

		PauseSystem();
		DisplayMessageLine( "Enter new details....", 5*60 );

		ShowCursor( TRUE );
		ShowCursor( TRUE );
		ShowCursor( TRUE );
		
		// display dialog for inputs
		if( loginDlg.DoModal() == IDOK )
		{
			// if OK hit, then redownload config.xml because we changed users or slideshow IDs
			AppData->m_sessionToken.Empty();		// clear the login session ID so it relogs in
			DownloadXMLContent();
		}

		((NewsBarThread *)newsThread)->CNewsBar.PauseOff();

		// unpause system.
		PauseSystem();

		SetFocus();
		ShowCursor( FALSE );
	} else
	{
		ShowCursor( TRUE );
		PauseSystem();

		loginDlg.BringWindowToTop();
		loginDlg.RedrawWindow();
		loginDlg.SetFocus();
	}
}

// Pause all screen action, ie  video and image cycling
int CWidgieDlg::PauseSystem(void)
{
	if( moviePlaying() )
	{
		PauseMoviePlayback();
	} else
	{
		if( timerActive )
		{
			StopTimers();
			DisplayMessageLine( "Pausing Images for 3mins", 3*60 );
			PrintText( "PAUSED" );
		} else {
			PrintText( "PLAY" );
			DisplayMessageLine( "Pausing stopped, PLAY BALL", 3 );
			StartImageTimer();
			((NewsBarThread *)newsThread)->CNewsBar.PauseOff();
		}
	}
	return 0;
}


// perform sleep mode
// stop animations/movies
// blank screen/monitor
void CWidgieDlg::ActivateSleepMode(void)
{
	if( timerActive )
		PauseSystem();

	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, 0);
}

// restart all actions.
void CWidgieDlg::StopSleepMode(void)
{
	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, 0, 0);

	PauseSystem();
}




bool CWidgieDlg::PerformSleepModeCheck(void)
{
	CTime currentTime = CTime::GetCurrentTime();
	CString timeString = currentTime.Format("%H:%M");
	CTimeSpan nowHours = TimeDateToCTimeSpan( timeString, 0 );
	CTimeSpan downHours = TimeDateToCTimeSpan( CFG->cfgSleepStartTime, 0 );		// 11pm time
	CTimeSpan upHours = TimeDateToCTimeSpan( CFG->cfgSleepStopTime, 0 );		// 6am time

	// if its late at night, lets make it dark
	if( (nowHours > downHours ||		// is it after 11pm ????  or
		nowHours < upHours) &&			// is it before 6am ???
		sleep_active == FALSE )
	{
		OutDebugs( "STATUS: Changing to sleepmode automatically down at set time %s", CFG->cfgSleepStartTime );
		sleep_active = TRUE;
		ActivateSleepMode();
	} else
	if( (nowHours > upHours &&			// is it after 6am and 
		nowHours < downHours) &&		// is it before 11pm ???
		sleep_active == TRUE )
	{
		OutDebugs( "STATUS: Changing to sleepmode automatically up at set time %s", CFG->cfgSleepStopTime );
		StopSleepMode();
		sleep_active = FALSE;
	}

	return (bool)sleep_active;
}

//
// using the current impression (slide), record its basic details id/name/time for uploading later
// the format of the string is
// DATE . TIME . Collection_ID . Image_ID
//
int CWidgieDlg::RecordEventLog( ImpressionData imp )
{
	CString event;

	CTime current_t = CTime::GetCurrentTime();


	event.Format( "%s.%s.%s.%s", current_t.Format( "%Y-%m-%d.%H:%M:%S" ), 
		imp.m_image_cid, imp.m_image_id, 
		imp.m_title );

	OutDebugs( "Event Log - %s", event.GetBuffer(0) );

	m_EventLogList.AddTail( event );

	return 0;
}



