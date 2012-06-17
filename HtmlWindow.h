#pragma once

#if !defined(HTMLWINDOW_H)
#define HTMLWINDOW_H

#include "HtmlCtrl.h"
#include "resource.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CMovieDialog.h : header file
//

// CHtmlWindow dialog

class CHtmlWindow : public CDialog
{
	DECLARE_DYNAMIC(CHtmlWindow)

public:
	CHtmlWindow(CWnd* pParent = NULL );   // standard constructor
	virtual ~CHtmlWindow();

    void		ResetTimer();
	BOOL		ShowPageURL( CString url );
	BOOL		ChangePageURL( CString url );
	void		Hide( void );
	void		Blank( void );
	void		Black( void );
	int			StopMovie( void );

	CRect		m_InitRect;
	CRect		m_InitPageRect;
	CString		m_HtmlFile;
	CString		m_mediaDir;

	BOOL		moviePlaying;
	time_t		m_startTime;
	int			m_playLength;

// Dialog Data
	//{{AFX_DATA(CMovieDialog)
	enum { IDD = IDD_HTML_DIALOG };
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();

	CHtmlCtrl	m_page;

	//{{AFX_MSG(CHtmlWindow)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
