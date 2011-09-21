// AVI PlayerDlg.h : header file
//

#if !defined(AFX_AVIPLAYERDLG_H__F88E8973_5775_4DC0_ADFD_E821627A0F71__INCLUDED_)
#define AFX_AVIPLAYERDLG_H__F88E8973_5775_4DC0_ADFD_E821627A0F71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CAVIPlayerDlg dialog

class CAVIPlayerDlg : public CDialog
{
// Construction
public:
	CAVIPlayerDlg(CWnd* pParent = NULL);	// standard constructor
	~CAVIPlayerDlg();

    void ResetTimer();
	void StartMCIPlay();

	void EndVideo( void );
	void OnPlay();
	void OnPause();
	void OnStop();
	void OnCancel();

	void Hide();
	void ShowDialog();

//{{AFX_DATA(NewsDialog)
	enum { IDD = IDD_PLAYER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAVIPlayerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CAVIPlayerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg LRESULT OnGraphNotify( WPARAM wp, LPARAM lp );
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


private:
	HWND	m_MCIVideo;
	HWND	m_Video;
	int		m_length;
	int		m_playCount;

	BOOL	Pause;
	BOOL	dlgVisible;


public:
	CString m_Path;
	BOOL	moviePlaying;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVIPLAYERDLG_H__F88E8973_5775_4DC0_ADFD_E821627A0F71__INCLUDED_)
