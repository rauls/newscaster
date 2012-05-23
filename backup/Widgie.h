// Widgie.h : main header file for the WIDGIE application
//

#if !defined(AFX_WIDGIE_H__31E48102_6B2B_4FFB_BBAE_8A43A3C50426__INCLUDED_)
#define AFX_WIDGIE_H__31E48102_6B2B_4FFB_BBAE_8A43A3C50426__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif
#include <afxstr.h>
#include <atlimage.h> 
#include <afxtempl.h> 

#include "resource.h"		// main symbols
//#include "WidgieDlg.h"

#include "Splash.h"
#include "Picture.h"

#ifndef BUFFER_LENGTH
#define BUFFER_LENGTH       256
#endif

#define ONE_SECOND          	1000
#define INI_FILE				CFG->ini_file
#define DEFUALT_LOOP        	"default\\loop.xml"     // gotta put this in .ini file

#define	DEFAULT_LOGOFILE		"SplashLogo.jpg"
#define	SCROLLNEWSLOGO_FN		"news_logo_scroll.bmp"
#define	NEWSLOGO_FN				"news_logo.bmp"

#define DEFPATH_CONTENT			"Media\\"
#define DEFPATH_IMAGES			"Images\\"

#define	DEFURL_ARTWORK			"/content/"

#define	DEFURL_NEWS				"news.xml"
#define	DEFURL_MANIFEST			"manifest.xml"
#define	DEFURL_LOOPSEQ			"loop.xml"
#define	DEFURL_CONFIG			"config.xml"

#define	DEFURL_NATIONALNEWS		"http://ticker.seven.com.au/xml/national.xml"
#define	DEFURL_YAHOOTOPNEWS		"http://rss.news.yahoo.com/rss/topstories"
#define	DEFURL_YAHOOUSANEWS		"http://rss.news.yahoo.com/rss/us"
#define	DEFURL_YAHOOWORLDNEWS	"http://rss.news.yahoo.com/rss/world"
#define	DEFURL_BBCWORLDNEWS		"http://news.bbc.co.uk/rss/newsonline_world_edition/front_page/rss091.xml"

#define	XMLFN_NEWS				"/widgie/newsflash.xml"
#define	XMLFN_MANIFEST			"/widgie/manifest.xml"
#define	XMLFN_LOOPSEQ			"/widgie/loopsequence.xml"
#define	XMLFN_CONFIG			"/widgie/config.xml"

#define XMLFN_NATIONALNEWS		"national.xml"
#define XMLFN_YAHOOWORLDNEWS	"yahootopnews.xml"
#define XMLFN_BBCWORLDNEWS		"bbcnews.xml"


#define	CFG				( (CWidgieApp *)AfxGetApp( ) )



void PrintDebugText( char *txt, int color=-1 );

/////////////////////////////////////////////////////////////////////////////
// CWidgieApp:
// See Widgie.cpp for the implementation of this class
//
#ifndef SM_CMONITORS
#define	SM_CMONITORS 80
#endif


class CWidgieApp : public CWinApp
{
public:
	CWidgieApp();
    CSplash m_splash;

    /* System Globals */
    CString cfgLanguage;

	CString cfgIniFilename;
	char ini_file[BUFFER_LENGTH];

	int running;
	int	cfgRuns;

    /* Graphics Globals */
    int cfgTop;                 // Top of MCCA screen (y co-ordinate)
    int cfgLeft;                // Left of MCCA screen (x co-ordinate)
    int cfgBottom;              // Bottom of MCCA screen (y co-ordinate)
    int cfgRight;               // Right of MCCA screen (x co-ordinate)
	virtual cfgWidth()	{ return cfgRight-cfgLeft; };

    int cfgMeterHeight;         // Height of the Taxi meter used when rendering 
                                // overlaped regions when the newsbar moves away
    
    int cfgBarHeight;           // Height of the news bar 

    int cfgTempBarHeight;       // When a new bar height is received it is stroed here
                                // because the graphics for the newsflash crashes if
                                // cfgBarHeight is modified while the newsflash thread
                                // is oprating. WidgieDlg handles the stopping and
                                // starting of the thread and the copying of this
                                // variable to cfgBarHeight.
                            

    int cfgNewsBarDelay;        // Delay used to animate rising and falling of
                                // newsbar. Delay in between moving the dialog by
                                // 1 pixel.
    
    int cfgNewsInterval;        // time that the newsbar stays hidden after it hides
    int cfgScrollSpeed;         // time interval that news scrolling is updated

    int cfgBackgroundColor;     // The colour to paint the background


	int cfgTextColor;           // newsbar text colour

    CString cfgFontFace;        // newsbar font face any True Type Font will work
		
    int cfgFontHeight;          // height of the newsflash font
		
	int cfgStepSpeed;           // home many pixels to advance the caption when scrolling  (ie its speed)


	// ------- NEWS TICKERS
	BOOL cfgSaveBMPCache;
	BOOL cfgPreloadAllImages;
	BOOL cfgIgnoreEthernet;		// do use the ethernet as a download medium

	BOOL cfgScreenSaverMode;	// be a screen saver - do not talk to server hosts.
	BOOL cfgImageBlend;			// blend smoothly between two images

	BOOL cfgEnableHttpListen;	// enable the internal http listening server for debug/communications monitoring and media sharing

	BOOL cfgPurgeMissingMedia;	// remove missing media from play list if not found.
	BOOL cfgEnableLogs;			// enable debug/operations logs

	CString	cfgConfigVersion;	// version of XML config data being used , to decide if to download a new config.xml or not.


    /* Local Machine paths */  
    CString cfgLocalBaseDir;    // Directory where all directories required
                                // by the MCCA can be found
    
    CString	cfgLocalContentDir;		// Directory relative to cfgLocalBaseDir where all  JPEG and Sockwave content can be found

	CString	cfgLocalImagesDir;		// Directory relative to cfgLocalBaseDir where all applications static default images are kept

    CString cfgLocalXML_Dir;        // Directory relative to cfgLocalBaseDir where   all XML files can be found

    CString	cfg_App_LogoFile;


    CString cfgLocalNewsName;       // name of file containing newsflash XML
    CString cfgLocalLoopName;       // name of file containing Loopsequence XML
    CString cfgLocalManifestName;   // name of file containing manifest XML

	/* Web Server Information */
	CString cfgIPandPort;           // Main Content Server IP/PORT
	CString	cfgUsername;			// Username that is used to login to MDID2
	CString cfgUserpass;			// Password used to login to MDID2

	/* Server Content paths */
    CString cfgServerContentPath;   // path on server, relative to URL, that contains
                                    // all JPEG and Shockwave Content
	CString	cfgServerSubmitURL;		// submitted url to send hit data.
    
	CString	cfgConfigFileName;		// name of file containing config data in XML
    CString cfgNewsFileName;        // name of file containing newsflash XML or  script that returns the file
	CString cfgManifestFileName;    // name of file containing manifest XML or  script that returns the file
	CString cfgLoopsequenceFileName;// name of file containing loopsequence XML or  script that returns the file

	CString	cfgMainLogoFileName;
	CString	cfgNewsLogoFileName;
	CString	cfgScrollLogoFileName;

	CString	cfgUpdateExe;

	int		cfgDefaultPlayLength;
    /* download configuration 
		we have a download interval for *ALL* data, which downloads everything if its a fast net connection, or ONLY
		the XML content if its GPRS modem connection.
		we also have a download interval for NEWS only downloads, which is more frequent than general downloads.
	
	*/
    CString		cfgDownloadIntervalTime;
    int			cfgDownloadIntervalDay;

	// main media times/intervals
	CTime		cfgLastDownloadTime;
    CTimeSpan	cfgDownloadInterval;

	// news download intervals/counters
	CTime		cfgNewsLastDownloadTime;		// last time we got the news.
	CTimeSpan	cfgNewsDownloadInterval;		// probly 3hrs or something.

	// specific news download timers... for "today", if used, += 24hrs
	BOOL 		cfgUseSevenTicker;				// flag to decide if to use the 7 news ticker directly, or not.
	BOOL 		cfgUseYahooTicker;				// flag to decide if to use the yahoo news ticker directly, or not.
	BOOL 		cfgUseBBCTicker;				// flag to decide if to use the yahoo news ticker directly, or not.
	CTime		cfgNewsNextDownloadTime1;		// last time we got the news.
	CTime		cfgNewsNextDownloadTime2;		// last time we got the news.
	CTime		cfgNewsNextDownloadTime3;		// last time we got the news.
	CString		cfgNewsDownloadTime1;
	CString		cfgNewsDownloadTime2;
	CString		cfgNewsDownloadTime3;


	CList<CString,CString> cfgNewsRSSFeeds;		// a list of rss news feeds definable besides the hardcoded above ones.

	// xml downloa periods...
	CTime		cfgXMLLastDownloadTime;			// last time we got the xml
	CTimeSpan	cfgXMLDownloadInterval;			// probly 12hrs or something.

	// upload intervals/counters
	CTime		cfgLastUploadTime;		// last time we got the news.
	CTimeSpan	cfgUploadInterval;		// 1 week, 24*7

	// Auto generated based on last 6 digits of MAC ADDR
	CString		cfgID;

	CString		cfgBrightnessDown;
	CString		cfgBrightnessUp;
	int			cfgBrightnessDownValue;
	int			cfgBrightnessUpValue;

	BOOL		cfgShutdownWhenIdle;
	BOOL		cfgRebootWhenIdle;
	BOOL		cfgExitWhenIdle;			// exit app and restart via start.bat
	BOOL		cfgExitAt3am;				// exit app at 3am and restart via start.bat

	BOOL		cfgNeverShutdown;
	BOOL		cfgShutdownActive;

	BOOL		cfgShowScrollLogo;

	// ---------------------------------------------------------------------------------------------------------------------


	class CWidgieDlg	*m_mainDlg;
	int			appReturnCode;

	void		ChangeIVAStatus( int x );
	int			m_ivaStatus;
	int			m_ivaStatusLast;
	int			m_ivaVersion;
	int			m_CallCount;
	int			m_CallBusyCount;
	int			m_CallNoAnswerCount;
	int			m_CallNoDialToneCount;

#define	HD_TEMP_COUNT	60

	int			m_tempHD;					// temperature of HD
	int			m_tempHD_average;			// moving average of last hour
	int			m_tempHD_history[HD_TEMP_COUNT];		// 60 items worth (1 every 5mins)

	CString		m_TestingID;				// ID/name of the person testing the hardware.

	BOOL		m_logdebug;					// log all OutDebugs messages to debug.log

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWidgieApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWidgieApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    CImage	SplashLogo;
    

public:
	void	DoButtonCommand( unsigned char key );
	int		ChooseAdminMenu(int menuSelected);
	void	SetAppPriority( int pri );
	CString	BuildStatusString();
	int		PrintText(char *text);
	void	MuteCheck( int x );
	void	MuteHoldCheck( int secs );

	virtual const char *GetExeName() { return m_pszExeName; }
};



void RebootNow( void );
void ShutdownNow( void );
void DisplayMessageLine( CString message );
int globalCheckNetConnection(void);
int globalCheckNetAdaptors( CString *netip, CString *netname );
unsigned long HashIt(char *string, unsigned short len);
#define CHashIt( str )	HashIt( str.GetBuffer(0), str.GetLength() )

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WIDGIE_H__31E48102_6B2B_4FFB_BBAE_8A43A3C50426__INCLUDED_)
