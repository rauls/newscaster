#if !defined(AFX_WidgieTHREAD_H__2F998AB8_0080_4D80_970B_BD18AA2F9E03__INCLUDED_)
#define AFX_WidgieTHREAD_H__2F998AB8_0080_4D80_970B_BD18AA2F9E03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WidgieThread.h : header file
//

//#define	USE_PEEKMSG		1

#include "resource.h"
#include "Widgie.h"


/////////////////////////////////////////////////////////////////////////////
// WidgieThread thread

class WidgieThread : public CWinThread
{
	DECLARE_DYNCREATE(WidgieThread)
protected:
	WidgieThread();           // protected constructor used by dynamic creation
	BOOL running;
    
// Attributes
public:
    virtual ~WidgieThread();
    void Die();                                 // See C++ implementation

// Operations
public:
	CWidgieDlg	*m_pMainWnd;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WidgieThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
#ifdef USE_PEEKMSG
	virtual int Run();
#endif
	//}}AFX_VIRTUAL

// Implementation
protected:
	

	// Generated message map functions
	//{{AFX_MSG(WidgieThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

#ifndef USE_PEEKMSG
	DECLARE_MESSAGE_MAP()
#endif

private:

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WidgieTHREAD_H__2F998AB8_0080_4D80_970B_BD18AA2F9E03__INCLUDED_)
