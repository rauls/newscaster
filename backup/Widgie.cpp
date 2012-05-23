// Widgie.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include <afxsock.h>

#include "Widgie.h"
#include "WidgieThread.h"
#include "WidgieDlg.h"
#include "Picture.h"
#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>



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
    if (!AfxSocketInit())
	{
		OutDebugs( "Failed to init sockets" );
//		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	cfgIniFilename = GetExeName();
	cfgIniFilename.Append( ".ini" );

	SetDialogBkColor( 0 , 0x00ff00 );

	cfgRuns =
	appReturnCode = 0;

	cfgShutdownActive = 0;
	cfgDefaultPlayLength = 3;


	m_tempHD_average = 0;
	memset( m_tempHD_history, 0, sizeof(int)*HD_TEMP_COUNT );

	AfxEnableControlContainer();

    if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        OutDebugs( "CoInitialize Failed!" );
        exit(3);
    }

	cfgNewsNextDownloadTime1 =
	cfgNewsNextDownloadTime2 =
	cfgNewsNextDownloadTime3 = 0;
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

    SetAppPriority(THREAD_PRIORITY_BELOW_NORMAL);		//THREAD_PRIORITY_BELOW_NORMAL
//	SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS );


    // Get System Settings from the initialisation file and validate them

    HDC theScreen = GetDC(NULL);   // handle to the screen;

#ifdef _DEBUG2
    int maxHeight = 600;
    int maxWidth = 800;
#else
    int maxHeight = GetDeviceCaps(theScreen, VERTRES);
    int maxWidth = GetDeviceCaps(theScreen, HORZRES);
#endif
	// check if base dir exists, other wise default to current dir...

	GetCurrentDirectory( BUFFER_LENGTH, ini_file );
	strcat( ini_file, "\\" );
	strcat( ini_file, cfgIniFilename );


	// =========================== [GRAPHIC] AREA ============================

    cfgTop = GETGRAPHIC_INT( "Top",0 );
	VALIDATE_RANGE( cfgTop, 0, maxHeight )

    cfgLeft = GETGRAPHIC_INT( "Left",0 );
	VALIDATE_RANGE( cfgLeft, 0, maxWidth )


    cfgBottom = GETGRAPHIC_INT( "Bottom",maxHeight );
	VALIDATE_RANGE( cfgBottom, 0, maxHeight )

    cfgRight = GETGRAPHIC_INT( "Right",maxWidth );
	VALIDATE_RANGE( cfgRight, 0, maxWidth )


    cfgBarHeight = GETGRAPHIC_INT( "BarHeight",45 );
	VALIDATE_RANGE( cfgBarHeight, 0, maxHeight )
    
    cfgTempBarHeight = cfgBarHeight;

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
    
    cfgFontHeight = GETGRAPHIC_INT( "FontHeight",37 );
	VALIDATE_RANGE( cfgFontHeight, 10, 200 )


	cfgStepSpeed = GETGRAPHIC_INT( "StepSpeed",2 );
	VALIDATE_RANGE( cfgStepSpeed, 1, 15 )

    DWORD iniResult;
    LPTSTR lpReturnedString;

	GETINI_STRING( cfgFontFace, "GRAPHICS", "FontFace", "Verdana" );

	cfgImageBlend = GETINI_INT( "GRAPHICS", "ImageBlend", 0 );
	cfgSaveBMPCache = GETINI_INT( "GRAPHICS", "SaveBMPCache", 0 );

	// =========================== [LOCAL MACHINE] AREA ============================

	cfgUseSevenTicker = GETINI_INT( "LOCAL MACHINE", "SevenTickerNews", 0 );
	cfgUseYahooTicker = GETINI_INT( "LOCAL MACHINE", "YahooTickerNews", 0 );
	cfgPreloadAllImages = GETINI_INT( "LOCAL MACHINE", "PreloadAllImages", 0 );

	cfgScreenSaverMode = GETINI_INT( "LOCAL MACHINE", "ScreenSaver", 0 );
	cfgEnableHttpListen = GETINI_INT( "LOCAL MACHINE", "EnableHttpListen", 0 );
	cfgPurgeMissingMedia = GETINI_INT( "LOCAL MACHINE", "PurgeMissingMedia", 0 );

	cfgShowScrollLogo = GETINI_INT( "LOCAL MACHINE", "ShowScrollLogo", 1 );

	cfgEnableLogs = GETINI_INT( "LOCAL MACHINE", "EnableLogs", 0 );


	GETINI_STRING( cfgLanguage, "LOCAL MACHINE", "Language", "english" );
	GETINI_STRING( cfgLocalBaseDir, "LOCAL MACHINE", "LocalBaseDir", "C:\\Plasma.Application" );
	// check if base dir exists, other wise default to current dir...
	{
		struct stat sbuf;
		if( stat( cfgLocalBaseDir, &sbuf ) )
		{
			char tString[256];
			GetCurrentDirectory( BUFFER_LENGTH, tString );
			cfgLocalBaseDir = tString;
			cfgLocalBaseDir += "\\";
		}
	}

	GETINI_STRING( cfgLocalContentDir, "LOCAL MACHINE", "LocalContentDir", DEFPATH_CONTENT );
	GETINI_STRING( cfgLocalImagesDir, "LOCAL MACHINE", "LocalImagesDir", DEFPATH_IMAGES );
	GETINI_STRING( cfg_App_LogoFile, "LOCAL MACHINE", "LogoFile", DEFAULT_LOGOFILE );
    

	// ---- process last transfer times from C:\WINDOWS\widgie.ini
	{
		CString cfgLastDownloadTimeStr,cfgLastDownloadDateStr;
		//char *ini_file = cfgIniFilename.GetBuffer(0);
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
		GETINI_STRING( cfgLastDownloadDateStr, "DOWNLOAD", "LastUploadDate", "2004-04-04" );
		cfgLastUploadTime = TimeDateToCTime(cfgLastDownloadTimeStr, cfgLastDownloadDateStr);

		// config version of the last config.xml file downloaded to check against to decide if to download it again or not....
		GETINI_STRING( cfgConfigVersion, "LOCAL MACHINE", "ConfigVersion", "1.0" );

		cfgRuns = GETINI_INT( "LOCAL MACHINE", "Runs", 0 );

	}

	// ---- process download interval values....
	GETINI_STRING( cfgDownloadIntervalTime, "DOWNLOAD", "XMLDownloadIntervalTime", "12:00:00" );
	cfgDownloadIntervalDay = GetPrivateProfileInt( "DOWNLOAD", "XMLDownloadIntervalDays", 0, ini_file );
	cfgXMLDownloadInterval = TimeDateToCTimeSpan( cfgDownloadIntervalTime, cfgDownloadIntervalDay );
	VALIDATE_RANGE( cfgXMLDownloadInterval, 60*60, 88640*31 )

	GETINI_STRING( cfgDownloadIntervalTime, "DOWNLOAD", "NewsDownloadIntervalTime", "03:00:00" );
	cfgDownloadIntervalDay = GetPrivateProfileInt( "DOWNLOAD", "NewsDownloadIntervalDays", 0, ini_file );
	cfgNewsDownloadInterval = TimeDateToCTimeSpan( cfgDownloadIntervalTime, cfgDownloadIntervalDay );
	VALIDATE_RANGE( cfgNewsDownloadInterval, 60*60, 88640*31 )

	GETINI_STRING( cfgDownloadIntervalTime, "DOWNLOAD", "MediaDownloadIntervalTime", "06:00:00" );
	cfgDownloadIntervalDay = GetPrivateProfileInt( "DOWNLOAD", "MediaDownloadIntervalDays", 0, ini_file );
	cfgDownloadInterval = TimeDateToCTimeSpan( cfgDownloadIntervalTime, cfgDownloadIntervalDay );
	VALIDATE_RANGE( cfgDownloadInterval, 60*60, 88640*31 )

	GETINI_STRING( cfgDownloadIntervalTime, "DOWNLOAD", "UploadIntervalTime", "00:00:00" );
	cfgDownloadIntervalDay = GetPrivateProfileInt( "DOWNLOAD", "UploadIntervalDays", 7, ini_file );
	cfgUploadInterval = TimeDateToCTimeSpan( cfgDownloadIntervalTime, cfgDownloadIntervalDay );
	VALIDATE_RANGE( cfgUploadInterval, 23*60*60, 88640*31 )


	// --- LOCAL MACHINE


	GETINI_STRING( cfgLocalXML_Dir, "LOCAL MACHINE", "LocalXML_Dir", "Config\\" );

	GETINI_STRING( cfgLocalNewsName, "LOCAL MACHINE", "LocalNewsName", DEFURL_NEWS );
    
	GETINI_STRING( cfgLocalLoopName, "LOCAL MACHINE", "LocalLoopName", DEFURL_LOOPSEQ );
    
	GETINI_STRING( cfgLocalManifestName, "LOCAL MACHINE", "LocalManifestName", DEFURL_MANIFEST );


	// --- SERVER
	GETINI_STRING( cfgIPandPort, "SERVER", "IPandPort", "server" );
	GETINI_STRING( cfgServerContentPath, "SERVER", "ServerContentPath", DEFURL_ARTWORK );

	GETINI_STRING( cfgUsername, "SERVER", "Username", "Client" );
	GETINI_STRING( cfgUserpass, "SERVER", "Userpass", "Client" );
    
	GETINI_STRING( cfgNewsFileName, "SERVER", "NewsFileName", XMLFN_NEWS );
	GETINI_STRING( cfgManifestFileName, "SERVER", "ManifestFileName", XMLFN_MANIFEST );
	GETINI_STRING( cfgLoopsequenceFileName, "SERVER", "LoopsequenceFileName", XMLFN_LOOPSEQ );
	GETINI_STRING( cfgConfigFileName, "SERVER", "ConfigFileName", XMLFN_CONFIG );

    
    /* Initialise the meter hight with a valid value unitl it is updated by the taxi meter code */
    cfgMeterHeight = cfgBottom;
    
	OutDebugs( "Filling in white the  Splash window..." );
    /* Display the splash logo Image */
    CRect myRect;
    CDC* dialogDC;

	OutDebugs( "Creating Splash window..." );
  	/* Display the Slash Dialog */
	m_splash.Create(IDD_SPLASH, NULL);
	m_splash.GetClientRect(&myRect);
    dialogDC = m_splash.GetDC();

	if( GetSystemMetrics(SM_CMONITORS) > 1 )
	{
		cfgLeft += GetSystemMetrics(SM_CXFULLSCREEN);
		cfgRight += GetSystemMetrics(SM_CXFULLSCREEN);
	}

	Sleep( 10 );		// wait a bit for the window to be ready...
    m_splash.MoveWindow(cfgLeft, cfgTop, cfgRight, cfgBottom, TRUE);


    // Make the background White
    CBrush bkSplashBrush;
    bkSplashBrush.CreateSolidBrush(0x00FFFFFF);
    dialogDC->FillRect(&myRect, &bkSplashBrush);

    // load the logo file
    CString fileToShow = cfgLocalBaseDir + cfgLocalImagesDir + cfg_App_LogoFile;
	OutDebugs( "Loading Logo %s...", fileToShow.GetBuffer(0) );
    SplashLogo.Load(fileToShow);
	m_splash.ShowWindow( SW_SHOW );

	RECT logoRect = { 1,0, 801,600 };
	logoRect.right = cfgRight;
	logoRect.bottom = cfgBottom;
	OutDebugs( "Showing opening screen..." );
	int showResult;
		
	if( SplashLogo.IsNull() == FALSE )
		showResult = SplashLogo.Draw(dialogDC->m_hDC, logoRect);


	OutDebugs( "Start Main Dialog..." );
    CWidgieDlg dlg;
	m_pMainWnd = &dlg;

    SetAppPriority(THREAD_PRIORITY_ABOVE_NORMAL);		//THREAD_PRIORITY_BELOW_NORMAL

	dlg.newsThread = NULL;

	dlg.m_lastStatusTime = 0;


	/* Display ht emain dialog */
    /* The dialog that is used to display the JPEG advetisments */
	dlg.DoModal();


	OutDebugs( "Exit Main Dialog." );

    // Finished with COM
    CoUninitialize();
    // Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	exit( appReturnCode );
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


			RECT rc = { 80,0, 800, 16*2 };
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


unsigned long crctab[256] = {
  0x0,
  0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
  0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6,
  0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
  0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
  0x5BD4B01B, 0x569796C2, 0x52568B75, 0x6A1936C8, 0x6ED82B7F,
  0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A,
  0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
  0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58,
  0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033,
  0xA4AD16EA, 0xA06C0B5D, 0xD4326D90, 0xD0F37027, 0xDDB056FE,
  0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4,
  0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
  0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5,
  0x2AC12072, 0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
  0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 0x7897AB07,
  0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C,
  0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1,
  0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
  0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B,
  0xBB60ADFC, 0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698,
  0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D,
  0x94EA7B2A, 0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
  0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 0xC6BCF05F,
  0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
  0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80,
  0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
  0x58C1663D, 0x558240E4, 0x51435D53, 0x251D3B9E, 0x21DC2629,
  0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C,
  0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E,
  0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65,
  0xEBA91BBC, 0xEF68060B, 0xD727BBB6, 0xD3E6A601, 0xDEA580D8,
  0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2,
  0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
  0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74,
  0x857130C3, 0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
  0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 0x7B827D21,
  0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A,
  0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E, 0x18197087,
  0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
  0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D,
  0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE,
  0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB,
  0xDBEE767C, 0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
  0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 0x89B8FD09,
  0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
  0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF,
  0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

/* convert a string to a magic number */
unsigned long HashIt(char *string, unsigned short len)
{          
	register unsigned long answer=0,count;
	register unsigned char b0,c0;
	register char *p;

	p=string;
	for (count=0;count<len;count++) {
		c0 = *p++;
		b0 = (unsigned char)(answer >> 24);
		answer = answer << 8;
		answer |= b0;
		answer ^= (crctab[(b0^c0)&0xff]);
	}

	return(answer);
}

