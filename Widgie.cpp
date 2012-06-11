// Widgie.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <afxsock.h>

#include <shlobj.h>
#include <strsafe.h>

#include "Widgie.h"
#include "WidgieThread.h"
#include "WidgieDlg.h"
#include "ConfigEdit.h"
#include "Picture.h"
#include "utils.h"
#include "winnet_io.h"
#include "ping.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWidgieApp

BEGIN_MESSAGE_MAP(CWidgieApp, CWinApp)
	//{{AFX_MSG_MAP(CWidgieApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWidgieApp construction

CWidgieApp::CWidgieApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	SetDialogBkColor( 0L, 0L );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWidgieApp object

CWidgieApp theApp;



#define GETGRAPHIC_INT(name, def)		GetPrivateProfileInt("GRAPHICS", name, def, ini_file )

#define GETINI_INT(label,name, def)			GetPrivateProfileInt( label, name, def, ini_file )

#define GETINI_STRING( cfgString, label, tagname, defaultname ) lpReturnedString = cfgString.GetBuffer(BUFFER_LENGTH); \
    iniResult = GetPrivateProfileString( label, tagname, defaultname, lpReturnedString, BUFFER_LENGTH, ini_file ); \
	if( cfgString == "" ) cfgString = defaultname; \
	cfgString.ReleaseBuffer();

#define	VALIDATE_RANGE(var,min,max)			    if (var < min) { var = min; } else if (var > max) { var = max; }


/////////////////////////////////////////////////////////////////////////////
// CWidgieApp initialization
BOOL CWidgieApp::InitInstance()
{
	struct stat sbuf;

	if (!AfxSocketInit())
	{
		OutDebugs( "Failed to init sockets" );
//		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

    if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        OutDebugs( "CoInitialize Failed!" );
        exit(3);
    }

	OutDebugs( "MainApp: InitInstance" );

	cfgIniFilename = GetExeName();
	cfgIniFilename.Append( ".ini" );
	SetDialogBkColor( 0 , 0x00ff00 );

	cfgNoInternet = false;

	cfgRuns =
	appReturnCode = 0;

	cfgShutdownActive = 0;

	m_tempHD_average = 0;
	memset( m_tempHD_history, 0, sizeof(int)*HD_TEMP_COUNT );

	cfgNewsNextDownloadTime1 =
	cfgNewsNextDownloadTime2 =
	cfgNewsNextDownloadTime3 = 0;
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

//  SetAppPriority(THREAD_PRIORITY_BELOW_NORMAL);		//THREAD_PRIORITY_BELOW_NORMAL
//	SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );

    // Get System Settings from the initialisation file and validate them

    HDC theScreen = GetDC(NULL);   // handle to the screen;

    int maxHeight = GetDeviceCaps(theScreen, VERTRES);
    int maxWidth = GetDeviceCaps(theScreen, HORZRES);

#ifdef _DEBUGHALF
    maxHeight /= 2;
    maxWidth /= 2;
#endif


	SHGetFolderPath( NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,(LPSTR)ini_file );
	PathAppend( ini_file, GetExeName() );
	strcat( ini_file, "\\" );
	cfgLocalBaseDir = ini_file;

	// check if base dir exists, other wise default to current dir...
	if( stat( cfgLocalBaseDir, &sbuf ) )
	{
		GetCurrentDirectory( BUFFER_LENGTH, ini_file );
		strcat( ini_file, "\\" );
		cfgLocalBaseDir = ini_file;
	}

	PathAppend( ini_file, cfgIniFilename );


	// =========================== [GRAPHIC] AREA ============================
	// Screen Dimensions
    cfgTop = GETGRAPHIC_INT( "Top",0 );
	VALIDATE_RANGE( cfgTop, 0, maxHeight )

	cfgLeft = GETGRAPHIC_INT( "Left",0 );
	VALIDATE_RANGE( cfgLeft, 0, maxWidth )

    cfgBottom = GETGRAPHIC_INT( "Bottom",maxHeight );
	VALIDATE_RANGE( cfgBottom, 0, maxHeight )

    cfgRight = GETGRAPHIC_INT( "Right",maxWidth );
	VALIDATE_RANGE( cfgRight, 0, maxWidth )


	// Scroller Dimensions
    cfgBarHeight = GETGRAPHIC_INT( "BarHeight",45 );
	VALIDATE_RANGE( cfgBarHeight, 0, maxHeight )
    cfgTempBarHeight = cfgBarHeight;

	cfgFontHeight = GETGRAPHIC_INT( "FontHeight",37 );
	VALIDATE_RANGE( cfgFontHeight, 10, 200 )

#ifdef _DEBUGHALF
	if( cfgFontHeight>0 )	cfgFontHeight /= 2;
	if( cfgBarHeight>0 )	cfgBarHeight /= 2;
#endif

	// calculate automatic sizing to be 7%
	if( cfgBarHeight == 0 )
	{
		cfgBarHeight = (int)(cfgBottom * 0.07);
	}
	if( cfgFontHeight == 0 )
	{
		cfgFontHeight = cfgBarHeight - 5;
	}


	// these 3 are crap.....
    cfgScrollSpeed = GETGRAPHIC_INT( "ScrollSpeed",10 );
	VALIDATE_RANGE( cfgScrollSpeed, 10, 50 )
    cfgNewsBarDelay = GETGRAPHIC_INT( "NewsBarDelay",10 );
	VALIDATE_RANGE( cfgNewsBarDelay, 1, 50 )
    cfgNewsInterval = GETGRAPHIC_INT( "NewsInterval",4 );
	VALIDATE_RANGE( cfgNewsInterval, 1, 20 )

    cfgBackgroundColor = GETGRAPHIC_INT( "BackgroundColor",0x00000000 );
	VALIDATE_RANGE( cfgBackgroundColor, 0x00000000, 0x00FFFFFF )

    cfgTextColor = GETGRAPHIC_INT( "TextColor",0x00FFFFFF );
	VALIDATE_RANGE( cfgTextColor, 0x111111, 0xFFFFFF )
    
	cfgStepSpeed = GETGRAPHIC_INT( "StepSpeed",2 );			// true scroll speed
	VALIDATE_RANGE( cfgStepSpeed, 1, 15 )

	cfgDefaultPlayLength = 	GETGRAPHIC_INT( "DefaultPlayLen",3 );

    DWORD iniResult;
    LPTSTR lpReturnedString;

	GETINI_STRING( cfgFontFace, "GRAPHICS", "FontFace", "Verdana" );

	cfgImageBlend = GETINI_INT( "GRAPHICS", "ImageBlend", 0 );
	cfgSaveBMPCache = GETINI_INT( "GRAPHICS", "SaveBMPCache", 0 );
	cfgCacheSourceImages = GETINI_INT( "GRAPHICS", "CacheLoad", 0 );

	// =========================== [LOCAL MACHINE] AREA ============================
	cfgUseRSSNews = GETINI_INT( "LOCAL MACHINE", "RSSNews", 1 );

	for( int i=0; i<10; i++)
	{
		CString rss, val;
		val.Format( "Item_%d", i );
		GETINI_STRING( rss, "RSSNEWS", val, "" );
		if( rss.IsEmpty() == false )
		{
			cfgNewsRSSFeeds.AddTail( rss );
		}
	}

	cfgPreloadAllImages = GETINI_INT( "LOCAL MACHINE", "PreloadAllImages", 0 );
	cfgScreenSaverMode = GETINI_INT( "LOCAL MACHINE", "ScreenSaver", 0 );
	cfgEnableHttpListen = GETINI_INT( "LOCAL MACHINE", "EnableHttpListen", 0 );
	cfgPurgeMissingMedia = GETINI_INT( "LOCAL MACHINE", "PurgeMissingMedia", 0 );
	cfgShowScrollLogo = GETINI_INT( "LOCAL MACHINE", "ShowScrollLogo", 1 );
	cfgEnableLogs = GETINI_INT( "LOCAL MACHINE", "EnableLogs", 0 );

	GETINI_STRING( cfgRestartTime, "LOCAL MACHINE", "RestartTime", "06:00:00" );

	GETINI_STRING( cfgLanguage, "LOCAL MACHINE", "Language", "english" );
	GETINI_STRING( cfgLocalBaseDir, "LOCAL MACHINE", "LocalBaseDir", cfgLocalBaseDir );
	// check if base dir doesnt exist, default to current dir...
	if( stat( cfgLocalBaseDir, &sbuf ) )
	{
		char tString[256];
		GetCurrentDirectory( BUFFER_LENGTH, tString );
		cfgLocalBaseDir = tString;
		cfgLocalBaseDir += "\\";
	}

	GETINI_STRING( cfgLocalContentDir, "LOCAL MACHINE", "LocalContentDir", DEFPATH_CONTENT );
	GETINI_STRING( cfgLocalImagesDir, "LOCAL MACHINE", "LocalImagesDir", DEFPATH_IMAGES );
	GETINI_STRING( cfg_App_LogoFile, "LOCAL MACHINE", "LogoFile", DEFAULT_LOGOFILE );
    
	GETINI_STRING( cfgSleepStartTime, "LOCAL MACHINE", "SleepStartTime", "21:00" );
	GETINI_STRING( cfgSleepStopTime, "LOCAL MACHINE", "SleepStopTime", "07:00" );

	// ---- process last transfer times from C:\WINDOWS\widgie.ini
	{
		CString cfgLastDownloadTimeStr,cfgLastDownloadDateStr;

		GETINI_STRING( cfgLastDownloadTimeStr, "DOWNLOAD", "LastXMLDownloadTime", "00:00:00" );
		GETINI_STRING( cfgLastDownloadDateStr, "DOWNLOAD", "LastXMLDownloadDate", "2004-04-04" );
		cfgXMLLastDownloadTime = TimeDateToCTime(cfgLastDownloadTimeStr, cfgLastDownloadDateStr);

		GETINI_STRING( cfgLastDownloadTimeStr, "DOWNLOAD", "LastNewsDownloadTime", "00:00:00" );
		GETINI_STRING( cfgLastDownloadDateStr, "DOWNLOAD", "LastNewsDownloadDate", "2004-04-04" );
		cfgNewsLastDownloadTime = TimeDateToCTime(cfgLastDownloadTimeStr, cfgLastDownloadDateStr);

		GETINI_STRING( cfgLastDownloadTimeStr, "DOWNLOAD", "LastMediaDownloadTime", "00:00:00" );
		GETINI_STRING( cfgLastDownloadDateStr, "DOWNLOAD", "LastMediaDownloadDate", "2004-04-04" );
		cfgLastDownloadTime = TimeDateToCTime(cfgLastDownloadTimeStr, cfgLastDownloadDateStr);
		cfgDownloadInterval = 60*60;

		GETINI_STRING( cfgLastDownloadTimeStr, "DOWNLOAD", "LastUploadTime", "00:00:00" );
		GETINI_STRING( cfgLastDownloadDateStr, "DOWNLOAD", "LastUploadDate", "2005-01-01" );
		cfgLastUploadTime = TimeDateToCTime(cfgLastDownloadTimeStr, cfgLastDownloadDateStr);

		// config version of the last config.xml file downloaded to check against to decide if to download it again or not....
		GETINI_STRING( cfgConfigVersion, "LOCAL MACHINE", "ConfigVersion", "1.0" );

		cfgRuns = GETINI_INT( "LOCAL MACHINE", "Runs", 0 );

	}

	// ---- process download interval values....
	GETINI_STRING( cfgDownloadIntervalTime, "DOWNLOAD", "XMLDownloadIntervalTime", "00:20:00" );
	cfgDownloadIntervalDay = GETINI_INT( "DOWNLOAD", "XMLDownloadIntervalDays", 0 );
	cfgXMLDownloadInterval = TimeDateToCTimeSpan( cfgDownloadIntervalTime, cfgDownloadIntervalDay );
	VALIDATE_RANGE( cfgXMLDownloadInterval, 60*60, 88640*31 )

	GETINI_STRING( cfgDownloadIntervalTime, "DOWNLOAD", "NewsDownloadIntervalTime", "01:00:00" );
	cfgDownloadIntervalDay = GetPrivateProfileInt( "DOWNLOAD", "NewsDownloadIntervalDays", 0, ini_file );
	cfgNewsDownloadInterval = TimeDateToCTimeSpan( cfgDownloadIntervalTime, cfgDownloadIntervalDay );
	VALIDATE_RANGE( cfgNewsDownloadInterval, 60*60, 88640*31 )

	GETINI_STRING( cfgDownloadIntervalTime, "DOWNLOAD", "MediaDownloadIntervalTime", "00:45:00" );
	cfgDownloadIntervalDay = GetPrivateProfileInt( "DOWNLOAD", "MediaDownloadIntervalDays", 0, ini_file );
	cfgDownloadInterval = TimeDateToCTimeSpan( cfgDownloadIntervalTime, cfgDownloadIntervalDay );
	VALIDATE_RANGE( cfgDownloadInterval, 60*60, 88640*31 )

	GETINI_STRING( cfgDownloadIntervalTime, "DOWNLOAD", "UploadIntervalTime", "01:00:00" );
	cfgDownloadIntervalDay = GetPrivateProfileInt( "DOWNLOAD", "UploadIntervalDays", 0, ini_file );
	cfgUploadInterval = TimeDateToCTimeSpan( cfgDownloadIntervalTime, cfgDownloadIntervalDay );
	VALIDATE_RANGE( cfgUploadInterval, 10*60, 88640*31 )

	// --- LOCAL MACHINE
	GETINI_STRING( cfgLocalXML_Dir, "LOCAL MACHINE", "LocalXML_Dir", "Config\\" );
	GETINI_STRING( cfgLocalNewsName, "LOCAL MACHINE", "LocalNewsName", DEFURL_NEWS );
   	GETINI_STRING( cfgLocalLoopName, "LOCAL MACHINE", "LocalLoopName", DEFURL_LOOPSEQ );
	GETINI_STRING( cfgLocalManifestName, "LOCAL MACHINE", "LocalManifestName", DEFURL_MANIFEST );

	// --- SERVER
	GETINI_STRING( cfgIPandPort, "SERVER", "IPandPort", "server" );
	GETINI_STRING( cfgLocalServer, "SERVER", "LocalServer", "localserver" );
	if( cfgLocalServer.CompareNoCase( "none" ) == 0 )
		cfgLocalServer.Empty();


	cfgScrollLogoFileName = SCROLLNEWSLOGO_FN;
	cfgNewsLogoFileName = NEWSLOGO_FN;

	// ######## SCREEN SAVER MODE
	//if( strstr( m_pszAppName, ".scr" ) )
	{
		bool showConfig = false;
		// handle command args.
		if( strstr( m_lpCmdLine, "/s" ) )
		{
			CFG->cfgScreenSaverMode = true;
			showConfig = false;
		} else
		if( strstr( m_lpCmdLine, "/c" ) )
		{
			showConfig = true;
		}
		if( strstr( m_lpCmdLine, "/monitor2" ) )
		{
			CFG->cfgUse2ndMonitor = true;
		}
		if( strstr( m_lpCmdLine, "/nointernet" ) )
		{
			CFG->cfgNoInternet = true;
		}

		if( showConfig )
		{
			CConfigEdit editor;
			editor.DoModal();

			OutDebugs( "Exit Main Dialog." );
			// Finished with COM
			CoUninitialize();
			// Since the dialog has been closed, return FALSE so that we exit the
			//  application, rather than start the application's message pump.
			exit( appReturnCode );
		}
	}
	// ###########################

	// #########  DISPLAY SPLASH WINDOW at this point.
	OpenSplashLogo();

	if( cfgIPandPort.IsEmpty() == false )
	{
		// Special case for 'waiting until a network is started'
		// we will typically only have a max wait time of 2minutes
		if( CFG->cfgScreenSaverMode == false )
		{
			CString	ourip, netName;
			int timeout = 0;
			int waitfornet = GETINI_INT( "SERVER", "waitfornet", 30 );		// determines how many seconds to wait for network to appear

			if( waitfornet >0 && CheckNetAdaptors( &ourip, &netName ) == FALSE )
			{
				PrintTextLarge( "  Waiting for network  " );
				Sleep( 500 );

				while( CheckNetAdaptors( &ourip, &netName ) == FALSE &&
					timeout< waitfornet )
				{

					Sleep( 1000 );
					timeout++;

					CString msg;
					msg.Format( "Waiting _________ %d", timeout );
					PrintTextLarge( msg.GetBuffer(0) );

				}

				if( CheckNetAdaptors( &ourip, &netName ) == FALSE )
					PrintTextLarge( "No network active  " );
			}
		}


		// work out if our server is locally located so that we can
		// use the 192. local address instead of its real internet address
		// that is not reachable inside the network. This internet address
		// is defined in config.xml and will be used once loaded if this
		// local address is not reachable, else we will just use this local
		// address directly because we assume we are inside the 'lab network'
		if( cfgIPandPort.Find( "192." ) >=0 || cfgIPandPort.Find( "10." ) >=0 )
		{
			if( Ping( cfgIPandPort.GetBuffer(0) ) >=0 )
			{
				cfgServerIsLocal = TRUE;
				OutDebugs( "STATUS: We are located internally near the server" );
			}
		}
		cfgOriginalServer = cfgIPandPort;
		OutDebugs( "Data server=%s, Local Proxy server=%s", cfgIPandPort, cfgLocalServer );
	}


	// default to 'sign1' as the local server
	// todo/
	// if sign1 cannot be found, then scan for all clients and assign one that has the
	// lowest IP to the local server.
	// if this value is blank, then we talk to the remote main server.
	if( cfgLocalServer.GetLength() >0 )
	{
		char buff[256]; long size=255;
		GetComputerName( buff, (LPDWORD)&size );
		cfgLocalName.SetString( buff );

		CString server = URLGetPort( cfgLocalName, NULL );
		// if we are the local server, then null it to avoid talking to our selves , DUH!!!!!
		if( server.CompareNoCase( buff ) == 0 )
		{
			OutDebugs( "We are the local 'proxy' server!!" );
			cfgLocalServer.Empty();
		}
		//else
		//	CheckLocalServer();
	}


	GETINI_STRING( cfgServerContentPath, "SERVER", "ServerContentPath", DEFURL_ARTWORK );

	// MDID login settings.
	GETINI_STRING( cfgUsername, "SERVER", "Username", "" );
	GETINI_STRING( cfgUserpass, "SERVER", "Userpass", "" );
	GETINI_STRING( cfgID, "SERVER", "SlideshowID","" );

	// Lets find our ID and use that if we are still in demo mode.
	// because we can overide the ID to become any one, any time.
	if( cfgID.IsEmpty() == TRUE || 
		cfgID.CompareNoCase( "default" ) == 0 ||
		cfgID.CompareNoCase( "automatic" ) == 0 
		)
	{
		CString macAddr, ourip, netName;
		FindNetworkAdaptors( "*", NULL,NULL,NULL, &macAddr);

		// if we do have a mac address, lets use its last 6 digits.
		if( macAddr.IsEmpty() == FALSE )
		{
			cfgID = "Display_";
			cfgID.Append( macAddr.Right( 6 ) );
		} else
			cfgID = "Display_Demo";
	}



	GETINI_STRING( cfgServerImagePath, "SERVER", "ServerImagePath", "/content/personal/full/" );
	GETINI_STRING( cfgServerVideoPath, "SERVER", "ServerVideoPath", "/content/video" );

	// remote server url files to access to get data down.
	GETINI_STRING( cfgNewsFileName, "SERVER", "NewsFileName", XMLFN_NEWS );
	GETINI_STRING( cfgManifestFileName, "SERVER", "ManifestFileName", XMLFN_MANIFEST );
	GETINI_STRING( cfgLoopsequenceFileName, "SERVER", "LoopsequenceFileName", XMLFN_LOOPSEQ );
	GETINI_STRING( cfgGetImageFileName, "SERVER", "GetImageFileName", XMLFN_GETIMAGE );
	GETINI_STRING( cfgConfigFileName, "SERVER", "ConfigFileName", XMLFN_CONFIG );

	// only download media files if they dont exist locally...
	// or always try to download them even if they exist, but stop
	// when they are the same size.
	cfgVideoDownloadAlways = FALSE;
	cfgImagesDownloadAlways = FALSE;

    /* Initialise the meter hight with a valid value unitl it is updated by the taxi meter code */
    cfgMeterHeight = cfgBottom;
    
	CString msg;
	msg.Format( "ID = %s", cfgID );
	PrintTextLarge( msg.GetBuffer(0) );

	// ################## SETUP MAIN WINDOW #######################
	OutDebugs( "Start Main Dialog..." );
    CWidgieDlg dlg;
	m_pMainWnd = &dlg;

	//SetAppPriority(THREAD_PRIORITY_BELOW_NORMAL);		//THREAD_PRIORITY_BELOW_NORMAL  THREAD_PRIORITY_ABOVE_NORMAL

	/* Display the main dialog */
    /* The dialog that is used to display the JPEG advetisments */
	dlg.DoModal();

	OutDebugs( "MainDlg ended, Exiting Main ..." );

    // Finished with COM
    CoUninitialize();

    // Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	exit( appReturnCode );
}

BOOL CWidgieApp::OpenSplashLogo( void )
{
	OutDebugs( "Filling in white the Splash window..." );
	OutDebugs( "Creating Splash window..." );
    /* Display the splash logo Image */
    CRect myRect;
    CDC* dialogDC;
  	/* Display the Slash Dialog */
	m_splash.Create(IDD_SPLASH, NULL);
	m_splash.GetClientRect(&myRect);
    dialogDC = m_splash.GetDC();

#ifdef _DEBUG
	cfgUse2ndMonitor = true;
#endif
   
	if( GetSystemMetrics(SM_CMONITORS) > 1 && cfgUse2ndMonitor )
	{
		int w = GetSystemMetrics(SM_CXFULLSCREEN);
		int y = GetSystemMetrics(SM_CYFULLSCREEN);
		int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int vy = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		int x2 = vw - w;
		int y2 = vy - y;

		cfgLeft += w;
		cfgRight += x2;
		cfgBottom = vy;
	}

//	Sleep( 10 );		// wait a bit for the window to be ready...
    m_splash.MoveWindow(cfgLeft, cfgTop, cfgRight-cfgLeft, cfgBottom-cfgTop, TRUE);

	// Make the background White
    CBrush bkSplashBrush;
    bkSplashBrush.CreateSolidBrush(0x000000);
    dialogDC->FillRect(&myRect, &bkSplashBrush);

    // load the logo file
    CString fileToShow = cfgLocalBaseDir + cfgLocalImagesDir + cfg_App_LogoFile;
	OutDebugs( "Loading Splash Image - %s", fileToShow.GetBuffer(0) );
	m_splash.ShowWindow( SW_SHOW );

	CImage	SplashLogo;
	if( SplashLogo.Load(fileToShow) != S_OK )
	{
		SplashLogo.LoadFromResource( AfxGetInstanceHandle(), IDB_MAINIMAGE );
	}

	RECT logoRect = { 1,0, 801,600 };
	logoRect.right = cfgRight;
	logoRect.bottom = cfgBottom;
	int showResult;
		
	if( SplashLogo.IsNull() == FALSE )
	{
		OutDebugs( "Showing splash screen..." );
		showResult = SplashLogo.Draw(dialogDC->m_hDC, logoRect);
	}

	PrintTextLarge( "Starting up...." );

	return TRUE;
}


#include <winbase.h>

__int64 DiskFree( CString disk )
{
	__int64 avail, free, total;
	BOOL success = GetDiskFreeSpaceEx( disk.GetBuffer(0), (PULARGE_INTEGER) &avail,
	(PULARGE_INTEGER) &total, (PULARGE_INTEGER) &free);
	if (success)
		return free;
	else
		return 0;
}


//
// Check the local server to see if its reachable.
// If it is not, then clear out the record for it so nothing will use it.
// Future Addon: Scan for new local servers periodically, or reset them with
// new config.xml details using <localserver> element.
//
//
BOOL CWidgieApp::CheckLocalServer( void )
{
	// ok we are just a normal client, so lets try to ping the local proxy to see if its up.
	if( cfgLocalServer.IsEmpty() == FALSE )
	{
		// #### Scan our subnet for all IPS that are alive, and add them to a list
		//CList <CString,CString> machines;
		//int macs = ScanSubnetForDisplays( &machines );

		CString server = URLGetPort( cfgLocalServer, NULL );
		// see if we can ping it
		int ms = Ping( server ), download=0;

		// see if we can connect to its web port and that its another sign proxy
		if( ms >= 0 && m_mainDlg && m_mainDlg->AppData )
		{
			download = m_mainDlg->AppData->DownloadFile( cfgLocalServer, "/ver" );
		}

		// if it is indeed a sign proxy, lets use it.
		if(  ms >= 0 && download >0 )
		{
			if( cfgLocalServerReachable == FALSE )
				OutDebugs( "STATUS: Local Server '%s' is now reachable", server );

			cfgLocalServerReachable = TRUE;
			m_LocalServerFoundTime = CTime::GetCurrentTime();
			return TRUE;
		} else
		{
			if( cfgLocalServerReachable == TRUE )
				OutDebugs( "STATUS: Local Server '%s' cannot be reached", server );
			cfgLocalServerReachable = FALSE;
			return FALSE;
		}
	}
	return FALSE;
}



void CWidgieApp::DoButtonCommand( unsigned char key )
{
//	((CWidgieDlg *)m_pMainWnd)->OnKeyDown( key, 0, 0 );

	//MuteCheck( key );

	((CWidgieDlg *)m_pMainWnd)->SendMessage( WM_KEYDOWN, key&0xff, 0 );
}

int CWidgieApp::ChooseAdminMenu(int menuSelected)
{
	return ((CWidgieDlg *)m_pMainWnd)->ChooseAdminMenu( menuSelected );
}


void CWidgieApp::SetAppPriority( int pri )
{
    SetThreadPriority(pri);
}


int CWidgieApp::PrintText(char *text)
{
	if( cfgEnableLogs )
	{
		CDC* textDC  = m_splash.GetDC();
		if( textDC && textDC->m_hDC )
		{
			OutDebugs( text );


			RECT rc = { 80,0, cfgRight, 16*2 };
			textDC->SetBkColor( 0x00ffffff );
			textDC->SetTextColor( 0x000000 );

			CBrush bgBrush;
			bgBrush.CreateSolidBrush( 0x00ffffff );
	//		textDC->FillRect(&rc, &bgBrush);

			DrawText( textDC->m_hDC, text, strlen(text ), &rc, DT_LEFT );
			m_splash.ReleaseDC( textDC );
		}
	}
	return 0;
}



int CWidgieApp::PrintTextLarge(char *text)
{
	if( cfgEnableLogs )
	{
		CDC* textDC  = m_splash.GetDC();
		if( textDC && textDC->m_hDC )
		{
			int fontsize = 80;
#ifdef _DEBUG
			fontsize /= 2;
#endif
			OutDebugs( text );

			RECT rc = { 0,150, cfgRight-cfgLeft, cfgBottom };
			rc.top = cfgBottom - 20 - fontsize;

			textDC->SetBkColor( 0x00000000 );
			textDC->SetTextColor( 0x0000FF );
			textDC->SetBkMode( OPAQUE );

			CBrush bgBrush;
			bgBrush.CreateSolidBrush( 0x00 );

			HFONT
			textFont = CreateFont( fontsize,0, 0, 0,FW_NORMAL, FALSE, FALSE, FALSE,
			  				ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS,
							PROOF_QUALITY, DEFAULT_PITCH + FF_DONTCARE, "Times");
	 
			SelectObject( textDC->m_hDC, textFont );

			DrawText( textDC->m_hDC, text, strlen(text ), &rc, DT_CENTER );

			m_splash.ReleaseDC( textDC );
			DeleteObject( textFont );
		}
	}
	return 0;
}





// ######### GLOBAL FUNCS #########


void RebootNow( void )
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 
		
	// Get a token for this process. 
	if (OpenProcessToken(GetCurrentProcess(), 	TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		// Get the LUID for the shutdown privilege. 
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
			
		tkp.PrivilegeCount = 1;  // one privilege to set    
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
			
		// Get the shutdown privilege for this process. 
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 	(PTOKEN_PRIVILEGES)NULL, 0); 
			
		// Cannot test the return value of AdjustTokenPrivileges. 
		if (GetLastError() == ERROR_SUCCESS) 
		{
			CFG->appReturnCode = 2;	
			if( ExitWindowsEx(EWX_FORCE|EWX_REBOOT, 0) == 0 )
				OutDebugs( "shutdown failed - %s", GetLastErrorString() );
		}
	}
}






void ShutdownNow( void )
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 
		
	// Get a token for this process. 
	if (OpenProcessToken(GetCurrentProcess(), 	TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		// Get the LUID for the shutdown privilege. 
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
			
		tkp.PrivilegeCount = 1;  // one privilege to set    
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
			
		// Get the shutdown privilege for this process. 
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 	(PTOKEN_PRIVILEGES)NULL, 0); 
			
		// Cannot test the return value of AdjustTokenPrivileges. 
		if (GetLastError() == ERROR_SUCCESS) 
		{
			OutDebugs( "DEBUG: ShutdownNow()/Shutting down now...");

			CFG->cfgShutdownActive = TRUE;	

			CFG->appReturnCode = 2;	

			if( ExitWindowsEx(EWX_FORCE|EWX_SHUTDOWN, 0) == 0 )
				OutDebugs( "shutdown failed - %s", GetLastErrorString() );
		}
	}
}






#include "winnet_io.h"
#include ".\widgie.h"
int globalCheckNetConnection(void)
{
	int result;
	CString netip, netname;

	try
	{
		result = CheckNetAdaptors( &netip, &netname );
	}
	catch (...) {
		OutDebugs( "ERROR: CheckNetAdaptors() has failed..." );
	}


	if( CFG->cfgIgnoreEthernet && result == NETIS_LAN )
	{
		result = NETIS_NONE;
	}
	return result;
}


int globalCheckNetAdaptors( CString *netip, CString *netname )
{
	int result;

	try
	{
		result = CheckNetAdaptors( netip, netname );
	}
	catch (...) {
		OutDebugs( "ERROR: CheckNetAdaptors() has failed..." );
	}


	if( CFG->cfgIgnoreEthernet && result == NETIS_LAN )
	{
		result = NETIS_NONE;
	}
	return result;
}



int ScanSubnetForDisplays( CList <CString,CString> *machinesP )
{
	int count=0;

	CString localip, localname, partip;

	globalCheckNetAdaptors( &localip, &localname );
	
	if ( int pos = localip.ReverseFind( '.' ) )
	{
		partip = localip.Mid( 0, pos );		
	}


	for( int index=1; index<254; index++ )
	{
		CString ip;
		ip.Format( "%s.%d", partip, index );
		if( Ping( ip ) >= 0 )
		{
			OutDebugs( "Found machine %s", ip );
			machinesP->AddTail( ip );
			count++;
		}
	}
	return count;
}



void PrintDebugText( char *txt, int color )
{
	if( CFG->m_mainDlg )
		CFG->m_mainDlg->PrintDebugText( txt,color );
}




void DisplayMessageLine( CString message ) 
{
	if( CFG->m_mainDlg )
		CFG->m_mainDlg->DisplayMessageLine( message );
}


//
// save login variables and slideshow id to the widgie.ini file stored in the application folder
// not the Windows root folder.
void CWidgieApp::SaveLoginDetails(void)
{
//	char ini_file[BUFFER_LENGTH];
//	GetCurrentDirectory( BUFFER_LENGTH, ini_file );
//	strcat( ini_file, "\\" );
//	strcat( ini_file, INI_FILE );

	WritePrivateProfileString("SERVER", "Username", cfgUsername, ini_file );
	WritePrivateProfileString("SERVER", "Userpass", cfgUserpass, ini_file );

	// only if we have a nonestandard displayid do we save it.
	// we do not want to save an automatic ID to the variable, because
	// it then wont ever be automatic again, it must be either blank or
	// not defined to be automatic, or be defined as the word 'automatic'
	if( cfgID.Find( "Display_" ) == -1 )
		WritePrivateProfileString("SERVER", "slideshowid", cfgID, ini_file );
}

