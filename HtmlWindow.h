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

	BOOL		ShowPageURL( CString url );
	void		Hide( void );
	void		Blank( void );

	CRect		m_InitRect;
	CRect		m_InitPageRect;
	CString		m_HtmlFile;

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
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
