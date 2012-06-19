// WidgieDlg.h : header file
//

#if !defined(AFX_WIDGIEDLG_H__C53F83F8_D773_44D5_852F_1877658B730B__INCLUDED_)
#define AFX_WIDGIEDLG_H__C53F83F8_D773_44D5_852F_1877658B730B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "NewsDialog.h"
#include <afxstr.h>
#include <atlimage.h> 

#include "Picture.h"
#include "AVI PlayerDlg.h"
#include "swFlashDialog.h"
#include "HtmlWindow.h"
#include "LoginEdit.h"

#include "WidgieXML.h"
#include "loopseqinfo.h"


#define IMAGE_TIMER	         1
#define SCROLL_TIMER         2
#define STATUS_TIMER         3
#define VOICECALL_TIMER      4
#define	ONEMIN_TIMER		 5
#define	MUTE_TIMER			 6
#define	TENMIN_TIMER		 7


#define	AFXDIALOG		( (CWidgieApp *)AfxGetApp( ) )->m_mainDlg

extern void ShutdownNow( void );

#define		DEFAULT_SCREEN_W	800
#define		DEFAULT_SCREEN_H	600


/////////////////////////////////////////////////////////////////////////////
// CWidgieDlg dialog

class CWidgieDlg : public CDialog
{
// Construction
public:
	CWidgieDlg(CWnd* pParent = NULL);	// standard constructor
	~CWidgieDlg();						// standard deconstructor

    CWidgieXML*			AppData;        		// Reference to a strucure that contains all 
                                		// internal playlists for the MCCA
	class CWidgieApp*	ptheApp;
	
    CWinThread* 		newsThread;     		// thread that will contain the news dialog
    CWinThread* 		downloadThread; 		// thread that will do all downloading and parsing
    CWinThread* 		freecacheThread;  		// thread that will perform the freeing of cached images
    CWinThread* 		jpegThread;				// thread that will perform the dialing to advertiser

    CRITICAL_SECTION	playlistSection;		// the critical section that all threads must
												// used before reading or midifying the play lists
    CRITICAL_SECTION	statusSection;			// the critical section that all threads must
    CRITICAL_SECTION	loadimageSection;		// the critical section that all threads must

	CMovieDialog		flashMovie;        		// dialog that will display shockwave media
	CAVIPlayerDlg		videoDlg;
	CHtmlWindow			HtmlWindow;		
	CLoginEditDlg		loginDlg;



	// // set after OnKeyDown event...
	ImpressionData		currentImpression;
	POSITION			currentImpressionPos;

	
	int imageCount;             // index of current image in JPEG playlist
    BOOL updateAll;             // flag that is set if the entire JPEG is to
                                // be re-rendered the next time the OnPaint 
                                // event is fired for this dialog
    int languageCount;
    int JPEG_PlayLength;        // how long the current images is to be displayed
    int alreadyDownloading;     // flag that signifies if the download thread is running, 1 = yes, 2 = yes but dont update time

    


    // function that is called as a new thread to download new data
    static UINT GetNewMediaContent(LPVOID pWidgieDialog);
    static UINT GetNewXMLContent(LPVOID pWidgieDialog);
    static UINT GetLatestNewsContent(LPVOID pWidgieDialog);
	
	static UINT JPEGImageThread(LPVOID param);
	static UINT FreeImageThread(LPVOID param);
	static UINT PlayButtonSoundThread(LPVOID param);

	BOOL 	printingDocket;
	BOOL 	gprsData_f;
	BOOL 	gprsVoice_f;
	BOOL 	wifiInternet_f;

	BOOL 	loadingImpression;
	BOOL 	timerActive;
	BOOL 	freeingCache_f;
	BOOL	mute_status;

	BOOL 	adminMenu;
	BOOL 	statusInfo;
	BOOL 	debugMessages;

	BOOL	resetImpressions;

	BOOL	downloadConfigBinaries;		// when next downloading manifest, also get some config binaries too.
	BOOL	downloadConfigExe;
	BOOL	downloadConfigNewsLogo;
	BOOL	downloadConfigNewsScrollLogo;

	BOOL	shuttingDown;

	BOOL	brightnessLevelLow;			// true if the brightness level is low.
	BOOL	sleep_active;				// default false

	int		textColor;					// status/debug test colors...
	int		textColorBG;

	CTime	startedTime;
	CTime	shutdownTime;				// time when to shutdown automatically later...

	CTime	m_lastStatusTime;			// time of when we last recieved a status result.
	CTime	m_lastProxyTime;

	CList<CString,CString> m_EventLogList;		// this contains a log of each diplayed sign in the slideshow and is uploaded to the server every one hour


    void 	StartNewsAndMeter();
    void 	StartImageTimer();
    
	BOOL	moviePlaying();
	void	ShowNewsDialog(void);
	int		DoShowNextMovie(ImpressionData impression);


// Dialog Data
	//{{AFX_DATA(CWidgieDlg)
	enum { IDD = IDD_WIDGIE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWidgieDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	HCURSOR m_cursor;

	// Generated message map functions
	//{{AFX_MSG(CWidgieDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonDown(UINT nChar, CPoint p);
	afx_msg void OnLButtonDown(UINT nChar, CPoint p);
	afx_msg void OnMButtonDown(UINT nChar, CPoint p);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMouseMove( UINT flags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	
	//CPicture myJPEG;				// object that loads and displays all JPEG files
	CImage	*pPicture;				// reference to myJPEG

	char	*buttonSoundPtr[4];		// button wave file in memory.

	CRect	myRect;					// rectangle used to pass the dialogs bounds  to display functions
	CDC*	dialogDC;				// a handle to the device context that is assigned
									// the address if this diaslog and used passed to display functions

	CBrush	bgBrush;
	HFONT	textFont;


	enum { MOVIE_NONE=0, MOVIE_FLASH, MOVIE_MPEGAVI, MOVIE_HTML };
	int		movieType;
	BOOL	movieVisible;          // flag that is set to true when the shockwave window is displayed

	BOOL	infoEnabled;

	BOOL	onpaintDrawing;

	CString	m_lastCell, m_lastCell2;			// Store cell tower info here.... (hex str)
	CString	m_netip;
	CString m_netname;

	CTime	m_infoTime;							// time of when the info button is pressed
	long	m_slideshowLoopCount;
	// time counter, the time of when to allow a screen image change.
	unsigned long imageNext_t;

    void SetPicture(CImage* newPicture);		// See C++ implementation
    BOOL LoadImpression();
    BOOL LoadImpressionImage();
    void NextJPEG();							// See C++ implementation
    void PreviousJPEG();						// See C++ implementation
    void GetNextLanguage();

public:
	virtual void ResetLastDownloadTime()
	{
		ptheApp->cfgLastDownloadTime = (CTime::GetCurrentTime() - ptheApp->cfgDownloadInterval);
	}


	void DoScrollerUpdate(void);
	void StopTimers(void);
	void StopMoviePlayback(void);
	int  DoShowInformation(void);
	int  DoShowNext(void);
	int  DoShowPrevious(void);
	void GetCurrentImpression(void);
	void SetCurrentImpression(void);
	void IncImpressionViews(void);

	BOOL InitApplication(void);


	enum NetworkType { NETWORK_NONE, NETWORK_LAN, NETWORK_WIFI };
	BOOL IsNetworkAvailable( int type );

	int  InitFlashMovie(void);		
	int	 InitHttpWindow(int playLength);
	int  SetupMovie(void);
	void PauseMoviePlayback(void);
	void UnPauseMoviePlayback(void);
	void ExitCleanup(void);
	void ShowDialog( BOOL showLogo );

	int CheckNetConnection(void);
	int DownloadMediaContent(void);
	int DownloadXMLContent(void);
	int DownloadNewsFlashContent( BOOL dontupdateTime=0 );
	int UpdateDownloadTime( int updateType = 0 );
	int PrintText(char *text,int color=-1);
	int PrintDebugText(char *text,int color=-1);
	void DisplayMessageLine( CString message, int pausetime = 200 );
	int NextJPEGThreaded(void);
	int PreviousJPEGThreaded(void);
	int RestartImages(void);
	int FreeImpressionImageCache( BOOL sync = FALSE );

	CString GetStatusText( int impnumber = -1 );
	CString Status_GetSlideshow();
	CString Status_GetEventList(void);
	CString Status_GetSlideShowsList( void );

	int AdminShowStatus( int impnumber = -1 );
	int AdminShowSlideshow( void );

	int DisplayAdminMenu(int menupart, int command);
	int ChooseAdminMenu(int menuSelected);

	int DisplayScreenMessage( CString message, long duration=3 );
	int DisplayStatusInfoLine(void);
	int PositionWindow(void);

	int FreeAllLists(void);
	int ResetAllLists(void);
	int ResetPlaylist(void);
	int ResetNewsflashlist(void);

	UINT PlayBlankSoundThread(LPVOID param);

	int PrintXY(int x, int y, char * text, ...);
	int PrintXYC(int x, int y, int color, char * text, ...);
	int HideNewsLogo(void);
	// Ask user to enter new login details to MDID (plasmanet)
	void InputLoginDetails(void);
	// Pause all screen action, ie  video and image cycling
	int PauseSystem(void);
	void ActivateSleepMode(void);
	void StopSleepMode(void);
	bool PerformSleepModeCheck(void);
	int RecordEventLog(ImpressionData imp);
};





//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WIDGIEDLG_H__C53F83F8_D773_44D5_852F_1877658B730B__INCLUDED_)
