#if !defined(AFX_logoDialog_H__B3275B44_CA5C_4A77_855F_7A6BF75A4925__INCLUDED_)
#define AFX_logoDialog_H__B3275B44_CA5C_4A77_855F_7A6BF75A4925__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxstr.h>
#include <atlimage.h> 


// logoDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// logoDialog dialog

class logoDialog : public CDialog
{
// Construction
public:
	logoDialog(CWnd* pParent = NULL);   // standard constructor
	~logoDialog();

	void LoadImage();
	void ShowDialog( BOOL forceit = FALSE );
	void Hide();

    BOOL m_dlgVisible;            // flag that is set to true when the flag dialog
                                // is visible.

// Dialog Data
	//{{AFX_DATA(logoDialog)
	enum { IDD = IDD_LOGO_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(logoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(logoDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nChar, CPoint p);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


    CImage logoImage;			// Image of logo (bmp file)

	CRect myRect;               // rectangle used to pass the dialogs bounds
                                // to display functions
	CDC* dialogDC;              // a handle to the device context that is assigned
                                // the address if this diaslog and used passed to 
                                // display functions
    
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_logoDialog_H__B3275B44_CA5C_4A77_855F_7A6BF75A4925__INCLUDED_)
