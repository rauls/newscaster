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
    
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	ptheApp = (CWidgieApp *)AfxGetApp();

	buttonSoundPtr[0] = NULL;
	buttonSoundPtr[1] = NULL;
	buttonSoundPtr[2] = NULL;
	buttonSoundPtr[3] = NULL;
	buttonSoundPtr[4] = NULL;
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

	CString vstr; vstr.Format( "%s Version %s", ptheApp->GetExeName(),  PRODUCT_VERSION_STR );

	ptheApp->PrintText( vstr.GetBuffer(0) );

	ptheApp->PrintText( "Decoding XML files..." );
    AppData  = new CWidgieXML(ptheApp->cfgLanguage);
	AppData->m_currentDownloaded = 0;
	AppData->SetImpressionList( 0 );
    AppData->ParseAll();
	ptheApp->PrintText( "Decoding XML files Done" );
	    
    /* Later when the list is downloaded and parsed again or when the
       list is accessed this critical section will have to be entered
       before anything else can be done */
    InitializeCriticalSection(&playlistSection);
    InitializeCriticalSection(&statusSection);
    InitializeCriticalSection(&loadimageSection);
	OutDebugs( "INIT CRITICAL SECTION, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );

    imageCount = 0;

	ptheApp->PrintText( "Initing video dialogs..." );
	// create  movie dialog... but hide it 
	if( videoDlg.Create( IDD_VIDEO_DIALOG, NULL ) )
		ptheApp->PrintText( "Initing video dialogs... Ok" );
	else
		ptheApp->PrintText( "Initing video dialogs... Failed" );
	
	/* Display the first Image */
	//this->GetClientRect(&myRect);
	myRect.left = ptheApp->cfgLeft;
	myRect.top = 0;
	myRect.right = ptheApp->cfgRight;
	myRect.bottom = ptheApp->cfgBottom;
	dialogDC = this->GetDC();

	SetBkColor( dialogDC->m_hDC, 0x00 );

    languageCount = 0;
    infoEnabled = TRUE;


	ptheApp->PrintText( "Starting NewsThread..." );
    // Start the newsbar thread, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_TIME_CRITICAL
    newsThread = AfxBeginThread( RUNTIME_CLASS(NewsBarThread), THREAD_PRIORITY_HIGHEST, 0, CREATE_SUSPENDED );			//RUNTIME_CLASS or AFX_THREADPROC
	if( newsThread)
	{
		((NewsBarThread *)newsThread)->pAppData = &AppData;
		((NewsBarThread *)newsThread)->lpCRITICAL_SECTION = &playlistSection;
		((NewsBarThread *)newsThread)->ResumeThread();

		//((NewsBarThread *)newsThread)->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);		//THREAD_PRIORITY_HIGHEST,REALTIME_PRIORITY_CLASS
		ptheApp->PrintText( "Starting NewsThread... Ok" );
		Sleep( 1000 );
	} else
		ptheApp->PrintText( "Starting NewsThread... Failed!" );


	OutDebugs( "Starting main image" );
	// ==============
	ptheApp->PrintText( "Setting JPEG Picture..." );
	updateAll = TRUE;
    // setup the jpeg display timers 
    JPEG_PlayLength = 6;
	PositionWindow();
	OutDebugs( "Load next JPEG" );
	NextJPEGThreaded();
	updateAll = TRUE;

	startedTime = CTime::GetCurrentTime();

	ptheApp->PrintText( "Starting timers..." );
	OutDebugs( "Starting timers" );
	StartImageTimer();
    this->SetTimer(ONEMIN_TIMER, 60*1000, NULL);

	ptheApp->PrintText( "Main OnInitDialog complete." );

	ShowCursor( FALSE );

	OutDebugs( "Init Procedure Completed." );
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

	shuttingdDown = FALSE;

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
				ptheApp->cfgRight+1, ptheApp->cfgBottom, TRUE);
	return 0;
}


// ################################################################################################################


void CWidgieDlg::ShowDialog( BOOL showLogo )
{
	OutDebugs( "ShowDialog" );

	RECT rc = { ptheApp->cfgLeft,  ptheApp->cfgTop,
				ptheApp->cfgRight, ptheApp->cfgBottom };


	MoveWindow( ptheApp->cfgLeft,  ptheApp->cfgTop,
		        ptheApp->cfgRight, ptheApp->cfgBottom, FALSE );

	ShowWindow(SW_NORMAL);

	InvalidateRect( &rc, FALSE );
}




// This gets called when we exit so we can cleanup after our mess ;)
void CWidgieDlg::ExitCleanup(void)
{
	OutDebugs( "ExitCleanup" );

	shuttingdDown = TRUE;

	if( alreadyDownloading )
	{
		OutDebugs( "DEBUG: File is downloading, waiting till its stopped to exit" );

		int t_1 = timeGetTime();
		while( alreadyDownloading && (timeGetTime()-t_1) < 5000 )				// TIMEOUT after 5 seconds
			Sleep( 10 );
	}


	CString msg;
	msg.Format( "Application %s v%s  Shutting down", AfxGetAppName(), PRODUCT_VERSION_STR );

	Log_App_Event( 0, msg );
	Log_App_Error( msg.GetBuffer(0) );


	// stop the movies if we hit EXIT
	if( moviePlaying() )
	{
		StopMoviePlayback();
	}

	/* Kill all the timers so no code runs so the OS can clean up */
    StopTimers();
	this->KillTimer(ONEMIN_TIMER);

	if( AppData )
	{
		AppData->CloseDown();

		FreeAllLists();
	}

	if( loadingImpression )
	{
		OutDebugs( "Waiting for loading impression to end..." );
		int t_1 = timeGetTime();
		while( loadingImpression && (timeGetTime()-t_1) < 3000 )			// TIMEOUT after 15 seconds
			Sleep( 10 );
	}

	if( newsThread )
	{
		OutDebugs( "Main Dialog - Deleting newsbar thread" );
		((NewsBarThread *)newsThread)->CNewsBar.logoDlg.CloseWindow();
		((NewsBarThread *)newsThread)->CNewsBar.EndCleanup();
		//((NewsBarThread *)newsThread)->CNewsBar.DestroyWindow();
		//((NewsBarThread *)newsThread)->Delete();
		((NewsBarThread *)newsThread)->Die();
		OutDebugs( "Main Dialog - Deleted newsbar" );
		newsThread = NULL;
	}
    
	if( AppData )
	{
		OutDebugs( "Deleting AppData" );
		delete AppData;
	}

	OutDebugs( "Main Dialog - ExitCleanup Done." );
}


void CWidgieDlg::OnClose()
{
	OutDebugs( "Dialog OnClose called" );
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
					RECT destRC = { 0,0,800,600 };
					destRC.bottom = CFG->cfgBottom;
					destRC.right = CFG->cfgRight;
					pContentDC->FillSolidRect( &destRC, 0x0 );
					showResult = pPicture->AlphaBlend( pContentDC->m_hDC, destRC, destRC, 170, AC_SRC_OVER );
				} else
				{
					showResult = pPicture->Draw( pContentDC->m_hDC, myRect );
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

	ShowCursor( FALSE );
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
		ShowCursor( TRUE );

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

		ShowCursor( FALSE );
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

	ShowCursor( FALSE );
		
	// proceed if no movie is playing.... during movie we wont do anything here
	if( nIDEvent == IMAGE_TIMER &&
		(!moviePlaying())
		)
	{
		static gImageCounter = 0;

		if( gImageCounter%10 )
		{
			AudioPlayResource( "IDR_BLANK" );
			//playButtonSoundThread = AfxBeginThread( AFX_THREADPROC(PlayButtonSoundThread), (LPVOID)buttonSoundPtr[0], THREAD_PRIORITY_NORMAL, 0, 0  );
			//		sndPlaySound( NULL, SND_NODEFAULT );
		}

		gImageCounter++;

		GetCurrentImpression();
		IncImpressionViews();

		CString fileToShow;
		ImpressionData nextImpression;

		ImpList *impList = AppData->GetImpressionList();
		if( impList->GetCount() >0 )
		{
			POSITION newPos = impList->FindIndex(imageCount) ;
			nextImpression = impList->GetAt(newPos);
			fileToShow = ptheApp->cfgLocalBaseDir + ptheApp->cfgLocalContentDir + nextImpression.m_image_file;

			if( nextImpression.m_image_file.IsEmpty() )
			{
				imageCount++;
				GetCurrentImpression();
				DoShowNextMovie( currentImpression );
			} else 
				NextJPEGThreaded();
		}

		CTime currentTime = CTime::GetCurrentTime();

		/* If it is time to download the next lot of XML files do it */
		if ((currentTime >= (ptheApp->cfgNewsLastDownloadTime + ptheApp->cfgNewsDownloadInterval))
			&& (!alreadyDownloading))
		{
			DownloadNewsFlashContent();
		}
		else
		/* If it is time to download the next lot of MEDIA files do it */
		if ((currentTime >= (ptheApp->cfgLastDownloadTime + ptheApp->cfgDownloadInterval))
			&& (!alreadyDownloading))
		{
			DownloadMediaContent();
		}
		else
		/* If it is time to download the next lot of XML files do it */
		if ((currentTime >= (ptheApp->cfgXMLLastDownloadTime + ptheApp->cfgXMLDownloadInterval))
			&& (!alreadyDownloading))
		{
			DownloadXMLContent();
		}
		ShowCursor( FALSE );

	}// if ( !movie.moviePlaying )
	else
	// backup timer for scroller....
	if( nIDEvent == SCROLL_TIMER && timerActive && !moviePlaying() )
	{
		DoScrollerUpdate();
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
		
			int temp = 0;
			// calc rolling average for 10mins.
			for( int lp=0;lp < 10 && ptheApp->m_tempHD_history[lp]; lp++ )
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
		unsigned char *s1, *s2, *src1, *src2, *d;
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


// Load next image, and display on screen.
BOOL CWidgieDlg::LoadImpressionImage()
{
    BOOL bResult = TRUE;

	if (TryEnterCriticalSection( &loadimageSection ) == TRUE )
//	if (1)
	{
		long t_1, t_jpeg = 0, t_jpeg2, t_jpeg3;
		CImage	*myImage = NULL;
		CString fileToShow;

		t_1 = timeGetTime();

		ImpList *impList = AppData->GetImpressionList();
		POSITION newPos = impList->FindIndex(imageCount) ;
		ImpressionData nextImpression = impList->GetAt(newPos);

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
			//OutDebugs( "Using cached image of %s", nextImpression.m_image_file.GetBuffer(0) );
		} else {
			myImage = new CImage();
			if( myImage )
			{
				nextImpression.imageData = (void*)myImage;
				impList->SetAt(newPos, nextImpression);
			} else
				OutDebugs( "err failed to make new CImage" );
		}

		StopTimers();

		if( myImage && myImage->IsNull() )
		{
			// try to load BMP image first before tring original JPG
			CString bmpFN = fileToShow;
			bmpFN.Replace( ".jpg", ".bmp" );
			t_jpeg = timeGetTime();

			bResult = myImage->Load( bmpFN.GetBuffer(0) );		// if we cant load bmp version, try loading jpeg version
			t_jpeg = timeGetTime() - t_jpeg;
			OutDebugs( "LoadImpressionImage - Load BMP (%ldms) %s", t_jpeg, bmpFN.GetBuffer(0) );
			if( bResult != S_OK )
			// load JPEG image.... and save BMP version  , this will later be done in the downloader
			{
				OutDebugs( "LoadImpressionImage - Load JPEG %s", fileToShow.GetBuffer(0) );
				t_jpeg2 = timeGetTime();
				bResult = myImage->Load( fileToShow.GetBuffer(0) );

				if( bResult != S_OK )
				{
					fileToShow = DecreaseFilenameVersion( fileToShow );
					bResult = myImage->Load( fileToShow );
				}

				if( bResult == S_OK )
				{
					// optionally save an intermediate BMP cached file for fast re-load on slow-ass computers without jpeg decomp
					if( CFG->cfgSaveBMPCache )
					{
						//myImage->StretchBlt( destdc, destrect, srcrect, SRCCOPY );
						OutDebugs( "LoadImpression - Save BMP %s", bmpFN.GetBuffer(0) );
						myImage->Save( bmpFN.GetBuffer(0) );
					}

					t_jpeg2 = timeGetTime() - t_jpeg2;
					OutDebugs( "JPEG - time to myImage->Load/Save to BMP = %d ms", t_jpeg2 );
				}
				else
				{
					OutDebugs( "ERROR: Failed to load impression image %s", fileToShow.GetBuffer(0) );
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
					RECT destRC = { 0,0,800,600 };
					destRC.bottom = CFG->cfgBottom;
					destRC.right = CFG->cfgRight;

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
					else
					{
						myImage->Draw( pDC->m_hDC, destRC );
						SetPicture( myImage );
					}
					t_jpeg3 = timeGetTime() - t_jpeg3;

					ReleaseDC(pDC);
				}

				bResult = TRUE;
			}
			OutDebugs( "JPEG - time to myJPEG.Draw = %d ms", t_jpeg3 );

			//  Because all user input is done in this window lets make sure we have focus at all times
			this->SetFocus();
		} // if (bResult == TRUE)
		else
		{
			myImage->Destroy();
			nextImpression.imageData = NULL;

			SetPicture( NULL );
			Log_App_FileError(errMediaMissing, "Image Error", "Could not open the file: " + nextImpression.m_image_file );
			//Beep( 2600, 60 );
			JPEG_PlayLength = 2;
		}
		DisplayStatusInfoLine();
		StartImageTimer();
		OutDebugs( "JPEG - total time = %d ms", timeGetTime()-t_1 );

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
		ImpList *impList = AppData->GetImpressionList();

        int numberOfImages = impList->GetCount();
        
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
					OutDebugs( "Failed to LoadImpression, try next image %d...", imageCount );
					OutDebugs( "AppData->FreeImpressionImage( %d );", imageCount );
					SetPicture( NULL );
					AppData->FreeImpressionImage( imageCount );
					if( CFG->cfgPurgeMissingMedia )
					{
						impList->RemoveAt( impList->FindIndex(imageCount) );
					} else
						imageCount++;
					//NextJPEG();
					// Using this method - if all images are missing or invalid then the all the impressions will be removed and the logo will up
				}
			}
        }
        else
        {
            /* Rebuild the impression list with the default loop sequence */
			OutDebugs( "Loading default local loop sequence file" );
            CString oldLoopFile = ptheApp->cfgLocalLoopName;
            ptheApp->cfgLocalLoopName = DEFUALT_LOOP;
            
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
    } // if( !movie.moviePlaying && !videoDlg.moviePlaying )          
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
    } // if( !movie.moviePlaying )
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





void CWidgieDlg::OnRButtonDown(UINT nChar, CPoint p) 
{

	if( !adminMenu )
	{
		PlayButtonSoundThread( NULL );
	 
		GetCurrentImpression();
		OutDebugs( "Button Previous Selected" );
		DoShowPrevious();
	}
}


void CWidgieDlg::OnMButtonDown(UINT nChar, CPoint p) 
{
	if( !adminMenu  )
	{
		ptheApp->DoButtonCommand( 'M' );
	}
}


void CWidgieDlg::OnLButtonDown(UINT nChar, CPoint p) 
{
	if( adminMenu )
	{
		ptheApp->DoButtonCommand( 'I' );
	}
	else
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
		if( p.x > 20 && p.x <780 && p.y < 10 )
		{
			keybd_event( 'M', 0x45, 0, 0 );
		}
		else
		{
			GetCurrentImpression();
			OutDebugs( "Button Next Selected" );
			DoShowNext();
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
	ShowCursor( FALSE );

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
			case 'Q':
				SendMessage(WM_COMMAND, (WPARAM)IDCANCEL, 0);
				break;

			case 'R':
				RestartImages();
				BringWindowToTop();
				break;

			case 'V':
				{
					CString vstr;
					vstr.Format( "%s Version %s", AfxGetAppName(), PRODUCT_VERSION_STR );
					PrintText( vstr.GetBuffer(0) );
				}
				break;

			case 'D':
				PrintText( "Downloading New XML/Media Content..." );
				DownloadMediaContent();
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

					RestartImages();

					//NextJPEGThreaded();

					((NewsBarThread *)newsThread)->CNewsBar.ShowDialog( TRUE );
				}  else {
					PrintText( "Downloading Latest XML Content..." );
					ptheApp->cfgConfigVersion = "";
					DownloadXMLContent();
				}
				break;


			case 0xbe:
				ptheApp->cfgStepSpeed--; break;
			case 0xbc:
				ptheApp->cfgStepSpeed++; break;

			// Show Status
			case 'S':
				statusInfo ^= 1;
				if( statusInfo )
					AdminShowStatus();
				else
					DoShowNext();
				break;

			// Pause
			case ' ':
				if( moviePlaying() )
				{
					PauseMoviePlayback();
				} else
				{
					if( timerActive )
					{
						StopTimers();
						DisplayMessageLine( "Pausing Images for 60seconds", 60 );
						PrintText( "PAUSED" );
					} else {
						PrintText( "PLAY" );
						DisplayMessageLine( "Pausing stopped, PLAY BALL", 3 );
						StartImageTimer();
						((NewsBarThread *)newsThread)->CNewsBar.PauseOff();
					}
				}
				break;

			case 'H':
				{
					char txt[] = "\n\n\n\nKeys\n----\nF , B  - Forward/Backword\n[ ] - Brightness control\nI - Show Flash Anim\nSpace - Pause Image Cycling\n\n"
									"M -  Admin Menu\n"
									"D -  Download XML/Media content\n"
									"N -  Download News XML content\n"
									"P - Print Docket\n"
									"R - Refresh Impression Image\n"
									"S - Status Info Display\n"
									"V - Version Number\n"
									"+- Keypad ... Volume Control\n"
									"[ ] ... Brightness Control\n"
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
	OutDebugs( "MainDialog - StartNewsAndMeter" );
	if( newsThread )
	    ((NewsBarThread *)newsThread)->CNewsBar.ShowDialog();

	ShowCursor( FALSE );
}

void CWidgieDlg::StartImageTimer()
{
    this->SetTimer(IMAGE_TIMER, JPEG_PlayLength * ONE_SECOND, NULL);

    this->SetTimer(STATUS_TIMER, 300, NULL);

	
    //this->SetTimer(SCROLL_TIMER, 1, NULL);
	
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
			Log_App_Event( dlg->currentImpression.m_id, "Downloading new media content now..." );

			// only if a server is defined, try to download from it
			if( CFG->cfgIPandPort.IsEmpty() == false )
			{
				int downloadsize = dlg->AppData->DownloadXMLManifest();
				if( downloadsize >0 )
				{
					// ok now lets parse the download'ed manifest to decide what content to download...
					dlg->AppData->ParseManifest();
				} else
					OutDebugs( "ERROR: failed to download manifest file - (%s)", cfg->cfgManifestFileName );

				if( dlg->downloadConfigNewsLogo )
				{
					if( dlg->AppData->DownloadSupportImages( "newslogo" ) ){			// download new logos/news images...
						((NewsBarThread *)dlg->newsThread)->CNewsBar.ReloadLogo();
					}
					dlg->downloadConfigNewsLogo = FALSE;
				}
				if( dlg->downloadConfigNewsScrollLogo )
				{
					dlg->AppData->DownloadSupportImages( "scrolllogo" );
					dlg->downloadConfigNewsScrollLogo = FALSE;
				}
				if( dlg->downloadConfigExe )
				{
					if( dlg->AppData->DownloadNewExecuteable() > 0 )			// download new exe
						dlg->downloadConfigExe = FALSE;
				}

				// download the image/multimedia data thats defined in the manifest, this must be done after parsing the manifest xml file
				dlg->AppData->DownloadContent();		// download new media binaries from the manifest list
			}


			// download the image/multimedia data that in the loop but doesnt exist locally.
			// the loop sequence may contain direct urls, so try to download them regardless if the server is defined
			dlg->AppData->DownloadMissingContent();

			// reset our playing loop sequence to include new impressions with new media....
			// this will also restart our hit counters.....
			dlg->ResetPlaylist();

			// delete content/ stuff that IS NOT in the manifest list, but only do this on a monday
			CTime nowT = CTime::GetCurrentTime();
			if( nowT.GetDayOfWeek() == 1 )
				dlg->AppData->DeleteUnusedContent();

			// update system ini file which contains the last download times...
			dlg->UpdateDownloadTime();

			// get some news while we have some net connection any way.... :)
			GetLatestNewsContent( pWidgieDialog );
		} else
		if( net_status == NETIS_GPRS )
			OutDebugs( "Cannot download media content over GPRS DATA" );
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

	Log_App_Event( 0, "Downloading latest news content now..." );

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
		} else
			OutDebugs( "ERROR: failed to download news file - (%s)", cfg->cfgNewsFileName );

		if( dlg->alreadyDownloading != 2 )
			dlg->UpdateDownloadTime( 1 );

		Log_App_Event( 0, "Downloading latest news done." );
	} else
	{
		OutDebugs( "Download News, we have no network connection" );
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
		Log_App_Event( dlg->currentImpression.m_id, "Downloading new xml content now..." );

		CString currentVersion = cfg->cfgConfigVersion;
		// Download all/new XML content.... (probly about 50-70kn max raw, though should be compressed either
		// in .xml.gz format, or all the files in ONE .xml.zip package.
		// check http://www.15seconds.com/issue/020314.htm , and http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=7164&lngWId=4
		// Data is downloaded in the following order ; XMLNews, LoopSequence, Config Settings, Manifest Download List...
		int downloadsize =
		dlg->AppData->DownloadXMLAll();

		// ok now lets parse the download'ed manifest to decide what content to download...
		//dlg->AppData->ParseManifest();

		if( downloadsize >0 )
		{
			// parse all xml data that was downloaded
			dlg->ResetAllLists();

		} else
			OutDebugs( "ERROR: failed to download xml files for some reason when tried" );


		// after parsing everything, lets check to see if theres a new config xml to download....
		if( currentVersion.CompareNoCase( cfg->cfgConfigVersion ) )
		{
			// download a new config xml file
			dlg->AppData->DownloadXMLConfig();
			// parse our config settings, hopefully its all valid...
			dlg->AppData->ParseConfigDefaults();

			dlg->downloadConfigBinaries = TRUE;
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


int CWidgieDlg::InitMovie(void)
{
	return TRUE;
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
		unsigned char ValidMPEGHeader[4] = {0,0,0x01,0xBA};			//00000000  00 00 01 BA 21 00 01 00 19 80 09 D5 00 00 01 BB    ...º!....€.Õ...»
		unsigned char ValidAVIHeader[4] = {'R','I','F','F'};			//RIFF

		mmfile.Read(&HeaderBuffer[0], 3);
        mmfile.Close();
        
        /* check if the file is indeed a flash file */
        if (*HeaderBuffer == *ValidFlashHeader || *HeaderBuffer == *ValidShockHeader )
        {
			movieType = MOVIE_FLASH;
			return TRUE;
        } else
        if (*HeaderBuffer == *ValidMPEGHeader || *HeaderBuffer == *ValidAVIHeader )
        {
			movieType = MOVIE_MPEGAVI;
			videoDlg.m_Path = movieToShow;
			videoDlg.OnPlay();
			return TRUE;
        } else
        {
			movieType = MOVIE_NONE;
            Log_App_FileError(errBadMedia, "Multimedia Error", "Invalid Multimedia file: " + currentImpression.m_multimedia_file);
			return FALSE;
        }
    } // if (fileExists == TRUE)
    else
    {
        Log_App_FileError(errMediaMissing, "Multimedia Error", "Could not open the file: " + currentImpression.m_multimedia_file);
    }
	return FALSE;
}



void CWidgieDlg::StopMoviePlayback(void)
{
	OutDebugs( "Stopping Movie media playback" );

	if( moviePlaying() )
	{
		switch( movieType )
		{
			case MOVIE_FLASH:
				break;

			case MOVIE_MPEGAVI:
				BringWindowToTop();
				videoDlg.Hide();			// try to hide the main video dialog so its gone
				videoDlg.OnStop();			// then when we cant see it, stop it
				ShowNewsDialog();			// show our newsbar again
				break;
		}
	}

	// go to next image before we see the old one...
	//NextJPEGThreaded();

	// make sure our news bar scrolls now.
	StartNewsAndMeter();

	updateAll = TRUE;
	OutDebugs( "Stopping Movie media playback - Completed" );
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
	ShowCursor( FALSE );
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
		OutDebugs( "Showing Movie %d (%s)", impression.m_id, impression.m_multimedia_file );

//		StopTimers();
//		((NewsBarThread *)newsThread)->CNewsBar.PauseScroller( 120 );

		movieVisible = SetupMovie();
		
		m_infoTime = CTime::GetCurrentTime();		// now ignore info to call, with in 2/3 seconds of this time.

		ShowCursor( FALSE );

		if( movieVisible )
		{
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
		//this->SetFocus();
    } // if ((infoEnabled) && (!movie.moviePlaying ))
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

	OutDebugs( "JpegThread - begin" );

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

	OutDebugs( "JpegThread - completed" );

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

	ShowCursor( FALSE );

	updateAll = TRUE;

	BringWindowToTop();

	//OnPaint();

	StartNewsAndMeter();
    StartImageTimer();

	if( newsThread )
		((NewsBarThread *)newsThread)->CNewsBar.ShowDialog();

	ShowCursor( FALSE );

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
	} else {
		OutDebugs( "FreeAllLists/LeaveCriticalSection, LockCount=%d, RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
		return 1;
	}

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
		if( impList ){
			impList->RemoveAll();
			AppData->ParseLoop( impList );
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
		RECT rc = { 00,00, 800, 600-45 };

		rc.bottom = CFG->cfgBottom-CFG->cfgBarHeight;
		rc.right = CFG->cfgRight;

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
		RECT rc = { 00,00, 800-8, 600-40 };
		rc.left = x;
		rc.top = y;
		rc.bottom = CFG->cfgBottom-8;
		rc.right = CFG->cfgRight-40;


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
			RECT rc2 = { x,y, 800-8,y+(17*lines) };
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
		RECT rc = { 00,00, 800-8, 600-40 };
		rc.left = x;
		rc.top = y;
		rc.bottom = CFG->cfgBottom-8;
		rc.right = CFG->cfgRight-40;
		textDC->SetBkColor( textColorBG );
		textDC->SetTextColor( col );

		{
			CBrush bgBrush;
			bgBrush.CreateSolidBrush( textColorBG );
			RECT rc2 = { x,y, 800-8,y+17 };
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
		case 3: 	DownloadMediaContent();	break;
		case 4: 	DownloadXMLContent(); break;
		case 5:		DownloadNewsFlashContent(); break;
		case 6:		AppData->DownloadMissingContent(); break;
		case 7:		AppData->DeleteUnusedContent(); break;

		case 8: 	debugMessages ^= 1; 
					PrintDebugText( debugMessages ? "Debug is now ON        " : "Debug is now OFF       " ); 
					break;
		case 10:	ptheApp->cfgStepSpeed++;	ptheApp->cfgStepSpeed&=0x3; 
			break;

		case 17:	ptheApp->cfgUseYahooTicker = ptheApp->cfgUseSevenTicker = TRUE; DownloadNewsFlashContent(); break;
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
		case 24:	debugMessages = 1; 
					//ptheApp->cfgUpdateExe = "Widgie.exe";
					DeleteFile( "Media\\widgie.exe" );
					AppData->DownloadNewExecuteable();			// download new exe, then flag it to quit/restart....
					debugMessages = 0;
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
	CString	ourip, netName;
	int netstatus;
	int lines = 0;

	netstatus = CheckNetAdaptors( &ourip, &netName );
	
	CTime nowTime = CTime::GetCurrentTime();

	txt.Format( "Started on : %s,Clock=%s", startedTime.Format( "%Y-%m-%d %H:%M:%S" ), nowTime.Format( "%Y-%m-%d %H:%M:%S" ) );
	ADDTEXT

	txt.Format( "Last Media Download Time : %s", ptheApp->cfgLastDownloadTime.Format( "%Y-%m-%d %H:%M:%S" ) );
	ADDTEXT
	txt.Format( "Last XML Download Time : %s", ptheApp->cfgXMLLastDownloadTime.Format( "%Y-%m-%d %H:%M:%S" ) );
	ADDTEXT
	CTime next = (ptheApp->cfgXMLLastDownloadTime + ptheApp->cfgXMLDownloadInterval);
	txt.Format(  "Next XML Download Time : %s", next.Format( "%Y-%m-%d %H:%M:%S" ) );
	ADDTEXT

	txt.Format( "Last News Download Time : %s", ptheApp->cfgNewsLastDownloadTime.Format( "%Y-%m-%d %H:%M:%S" ) );
	ADDTEXT
	next = (ptheApp->cfgNewsLastDownloadTime + ptheApp->cfgNewsDownloadInterval);
	txt.Format( "Next News Download Time : %s", next.Format( "%Y-%m-%d %H:%M:%S" ) );
	ADDTEXT

	if( alreadyDownloading && !AppData->currentDownloadFile.IsEmpty() ) 
	{
		txt.Format( "Downloading - %s, %d bytes", alreadyDownloading ? "YES" : "NO", AppData->m_currentDownloaded );
		ADDTEXT
		txt.Format( "  %s", AppData->currentDownloadFile );
		ADDTEXT
	}

	txt.Format( "Network        : %s  (%s)", netName.GetBuffer(0), ourip.GetBuffer(0) );
	ADDTEXT

	txt.Format( "Hard Disk Temp : %d'c (10min Avg=%d'c)", ptheApp->m_tempHD, ptheApp->m_tempHD_average );
	ADDTEXT

	ImpList *impList = NULL;
	if( AppData )
	{
		impList = AppData->GetImpressionList();

		txt.Format( "Playlist   - LockCount=%d RecursionCount=%d", playlistSection.LockCount, playlistSection.RecursionCount );
		ADDTEXT
		txt.Format( "Impressions = %d, NewsFlash = %d, Manifest = %d", impList->GetCount(), AppData->newsflashList.GetCount(), AppData->fileList.GetCount() );
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

		txt.Format( "This Impression - id=%d, Hits=%d, Views=%d", impression.m_id, impression.m_hitCount, impression.m_viewCount );
		ADDTEXT
	}

	while( lines++ < 28 )
	{
		statusText += "\n";
	}

	return statusText;
}




int CWidgieDlg::AdminShowStatus( int impnumber )
{
	int x = 400;
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

//  ###############################################################################################################


