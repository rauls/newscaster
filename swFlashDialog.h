//{{AFX_INCLUDES()
#include "shockwaveflash.h"
//}}AFX_INCLUDES
#if !defined(AFX_SWFLASHDIALOG_H__34EEC57C_101F_4616_8562_F179A80C79CC__INCLUDED_)
#define AFX_SWFLASHDIALOG_H__34EEC57C_101F_4616_8562_F179A80C79CC__INCLUDED_

#include "WidgieDlg.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMovieDialog.h : header file
//

#define CHECK_FLASH_END_TIMER           1

/////////////////////////////////////////////////////////////////////////////
// CMovieDialog dialog

class CMovieDialog : public CDialog
{
// Construction
public:
	CMovieDialog(CWnd* pParent = NULL);    // standard constructor
	~CMovieDialog();
    BOOL moviePlaying;                      // flag to signify is the movies is
                                            // currently playing. Cleared when the
                                            // movie stops
	BOOL moviePaused;
	BOOL movieVisible;
    
// Dialog Data
	//{{AFX_DATA(CMovieDialog)
	enum { IDD = IDD_SHOCKWAVE_DIALOG };
	CShockwaveFlash	movie;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMovieDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMovieDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:
	void Hide(void);
	void WindowToTopCorner();
	int StopMovie(void);
	int PauseMovie(void);
	int PlayMovie(void);


private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SWFLASHDIALOG_H__34EEC57C_101F_4616_8562_F179A80C79CC__INCLUDED_)
