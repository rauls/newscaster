#if !defined(AFX_DIRECTXDIALOG_H__6AA3B9D3_E7B3_4888_830D_9C573D57F0DA__INCLUDED_)
#define AFX_DIRECTXDIALOG_H__6AA3B9D3_E7B3_4888_830D_9C573D57F0DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DirectXDialog.h : Header-Datei
//
#include <ddraw.h>
#include <mmsystem.h>
#include "resource.h"
#include "ddutil.h"
/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CDirectXDialog 

class CDirectXLockGuard;
class CDirectXDialog;
typedef void (CDirectXDialog::*setPixelPTR)(int x, int y, DWORD color);

class CDirectXDialog : public CDialog
{
	friend class CDirectXLockGuard;
// Konstruktion
public:
	CDirectXDialog(int ID, CWnd* pParent = NULL);   // Standardkonstruktor	

	virtual HRESULT restoreSurfaces();
	virtual void displayFrame() = 0;
	virtual HRESULT initDirectDraw();
	virtual void freeDirectXResources();

// Dialogfelddaten
	//{{AFX_DATA(CDirectXDialog)
		// HINWEIS: Der Klassen-Assistent fügt hier Datenelemente ein
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CDirectXDialog)
	protected:
	//}}AFX_VIRTUAL

// Implementierung
private:
	void BackbufferUnlock();
	HRESULT BackbufferLock();
protected:
	static int getBitMaskPosition(DWORD mask);
	static int getNumberOfBits(DWORD mask);
	void paintFrame();
	inline void setPixelOPTIMIZED(int x, int y, DWORD color);
	inline void setPixelSECURE(int x, int y, DWORD color);

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CDirectXDialog)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	CDisplay g_pDisplay;
	CSurface* g_pSecondarySurface;
	DDSURFACEDESC2 sDesc;
	char *backbuffervideodata;
	int y_pitch;
	int x_pitch;
	int rbits, gbits, bbits;
	int rpos, gpos, bpos;
	setPixelPTR setPixel;
};

class CDirectXLockGuard
{
	friend class CDirectXDialog;
	CDirectXDialog* dialog;
public:
	CDirectXLockGuard(CDirectXDialog* dlg);
	~CDirectXLockGuard();
public:
	HRESULT hr;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_DIRECTXDIALOG_H__6AA3B9D3_E7B3_4888_830D_9C573D57F0DA__INCLUDED_
