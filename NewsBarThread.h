#if !defined(AFX_NEWSBARTHREAD_H__2F998AB8_0080_4D80_970B_BD18AA2F9E03__INCLUDED_)
#define AFX_NEWSBARTHREAD_H__2F998AB8_0080_4D80_970B_BD18AA2F9E03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewsBarThread.h : header file
//

//#define	USE_PEEKMSG		1

#include "NewsDialog.h"
#include "WidgieXML.h"


/////////////////////////////////////////////////////////////////////////////
// NewsBarThread thread

class NewsBarThread : public CWinThread
{
	DECLARE_DYNCREATE(NewsBarThread)
protected:
	NewsBarThread();           // protected constructor used by dynamic creation
    
// Attributes
public:
    CWidgieXML** pAppData;                      /* reference to display lists 
                                                   that is passed through to the 
                                                   news dialog so as to retrive 
                                                   news captions */
    
    LPCRITICAL_SECTION lpCRITICAL_SECTION;      /* reference to critical section 
                                                   that is passed through to the 
                                                   news dialog */

	BOOL running;
    
    NewsDialog CNewsBar;                         /* the dialog that holds the 
                                                   newsbar graphics */

	void setAppData(CWidgieXML* pAppDataRef);   // See C++ implementation
    void Die();                                 // See C++ implementation
    void BringToFront();                        // See C++ implementation
    
    virtual ~NewsBarThread();

    // Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NewsBarThread)
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
	//{{AFX_MSG(NewsBarThread)
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

#endif // !defined(AFX_NEWSBARTHREAD_H__2F998AB8_0080_4D80_970B_BD18AA2F9E03__INCLUDED_)
