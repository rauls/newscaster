#if !defined(AFX_NEWSDIALOG_H__57675757_D407_45F1_A0AA_BFFB92AA38C1__INCLUDED_)
#define AFX_NEWSDIALOG_H__57675757_D407_45F1_A0AA_BFFB92AA38C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewsDialog.h : header file
//

#include "stdafx.h"
#include <afxtempl.h>
#include <mmsystem.h>
#include "newsflash.h"
#include "NewsflashInfo.h"
#include "logoDialog.h"


#define KEEP_FRONT_TIMER		1
#define SCROLL_SPEED_TIMER      2

extern "C"
void CALLBACK internalTimerProc(UINT id, UINT msg,DWORD dwUser, DWORD dw1, DWORD dw2);

/////////////////////////////////////////////////////////////////////////////
// NewsDialog dialog

class NewsDialog : public CDialog
{
// Construction
public:
    BOOL 	m_stillScrolling;    			// the return value of a funtion that tests is the 
         	                   				// the newscaption is still scrolling is stored in this
         	                   				// variable
	BOOL 	m_newsbarActive;				// yes our news bar is active
	BOOL 	m_nextnews;						// time to get the next news headline
	BOOL 	m_insideTimer;					// true when we are inside the timer event

	int 	m_zipoff;						// if true, the scroll text will accelerate fast till its gone, then after go back to normal
	int 	m_pausetime;        			// the amount of seconds to pause for (do nothing)
	time_t	m_pauseendtime;					// time to stop pausing
	int 	m_runcount;						// how many times the timer msg gets called.
	int		m_screenHz;						// screen rate to scroll properly

	LARGE_INTEGER	m_freqCounter;			// freq for high res counter
	double			m_msinterval;
	double			m_usinterval;

	LPCRITICAL_SECTION dataChangeSection;

	Newsflash* newsflash1;  				// the placeholder for the one and only newsbar graphics object
	logoDialog logoDlg;						// dilaog that will display the news logo
	

    NewsDialog(CWnd* pParent = NULL);		// standard constructor
	virtual ~NewsDialog();                  // See C++ implementation

    // See C++ implementation    
    void BindCaptions(CList <NewsflashData, NewsflashData>* captionRef);
    
    void BindSection(LPCRITICAL_SECTION lpCRITICAL_SECTION);

    void Hide();

private:
    int m_captionCount;        			// index in the display list of the newscaption being shown  
    BOOL m_bkInit;             			// flag to indiate if the background colour has been painted.
	BOOL m_timerActive;
    // list of complete newsflashed with their archive times and dates
    CList <NewsflashData, NewsflashData>* m_captionList;

    BOOL LoadNextCaption();  // See C++ implementation

	bool			StartMMTimer(UINT period, bool oneShot, UINT resolution);
	bool			StopMMTimer(bool bEndTime=FALSE);
	void			SafeStartTimer( int rate=2 );
	UINT            m_timerRes;
    UINT            m_timerId;

// Dialog Data
	//{{AFX_DATA(NewsDialog)
	enum { IDD = IDD_NEWSFLASH_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NewsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NewsDialog)
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void DoTimer(void);
	void EndCleanup( void );
	void StartTimers( int mstimer = 2 );
	void StopTimers(void);
	DWORD timeGetTime_us( void );
	void displayCentered(CString displayString, int pausefor=120);
	void PauseScroller( int seconds );
	void PauseOff(void);
	void ShowDialog( BOOL showLogo=TRUE );
	void ReloadLogo( void );
	void ShowLogo( void );
	void HideLogo(void);
	int ShowNextCaption(void);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWSDIALOG_H__57675757_D407_45F1_A0AA_BFFB92AA38C1__INCLUDED_)
