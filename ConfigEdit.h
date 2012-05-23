#pragma once
#include "afxwin.h"


// CConfigEdit dialog

class CConfigEdit : public CDialog
{
	DECLARE_DYNAMIC(CConfigEdit)

public:
	CConfigEdit(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigEdit();

// Dialog Data
	enum { IDD = IDD_CONFIGEDIT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// Default length of image or video
	afx_msg void OnBnClickedAddurl();
	afx_msg void OnLbnSelchangeRsslist();
	afx_msg void OnBnClickedAddrss();
	afx_msg void OnLbnSelchangeUrls();
	afx_msg BOOL OnInitDialog();

	bool ParseLoop( ImpList *impList );

    CEdit 		m_OutputEdit;
	CEdit 		m_defLength;
	CListBox 	m_urls;
	CListBox 	m_rss;
	CEdit 		m_rssText;
	CEdit 		m_urlText;
	CString 	m_urlStr;
	CString 	m_rssStr;
	afx_msg void OnBnClickedOk();
	int m_defaultLenInt;
	BOOL m_blendFadeB;
};
