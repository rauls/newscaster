// DirectXDialog.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "DlgBackgroundArtDeco.h"
#include "DirectXDialog.h"
#include "dxutil.h"

#include <assert.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CDirectXDialog 


CDirectXDialog::CDirectXDialog(int ID, CWnd* pParent /*=NULL*/)
	: CDialog(ID, pParent)
{
	//{{AFX_DATA_INIT(CDirectXDialog)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}




BEGIN_MESSAGE_MAP(CDirectXDialog, CDialog)
	//{{AFX_MSG_MAP(CDirectXDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CDirectXDialog::OnMove(int x, int y) 
{
	CDialog::OnMove(x, y);
	
	g_pDisplay.UpdateBounds();	
}

void CDirectXDialog::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);	

	LPDIRECTDRAW7 ddraw = g_pDisplay.GetDirectDraw();
	if (ddraw)
	{
		freeDirectXResources();
		initDirectDraw();
		paintFrame();
	}

	g_pDisplay.UpdateBounds();
}


int CDirectXDialog::getNumberOfBits(DWORD mask)
{
	int nob = 0;
	while (mask)
	{
		mask = mask & (mask - 1);
		nob++;
	}
	return nob;
}

int CDirectXDialog::getBitMaskPosition(DWORD mask)
{
	int pos = 0;
	while (!(mask & 1 << pos)) pos++;
	return pos;
}
// Wollen Sie Ihrem Dialogfeld eine Schaltfläche "Minimieren" hinzufügen, benötigen Sie 
//  den nachstehenden Code, um das Symbol zu zeichnen. Für MFC-Anwendungen, die das 
//  Dokument/Ansicht-Modell verwenden, wird dies automatisch für Sie erledigt.
BOOL CDirectXDialog::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}
BOOL CDirectXDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (FAILED(initDirectDraw()))
		return FALSE;

	return TRUE;  // Geben Sie TRUE zurück, außer ein Steuerelement soll den Fokus erhalten
}

HRESULT CDirectXDialog::restoreSurfaces()
{
	HRESULT hr;
	if (FAILED(hr = g_pDisplay.GetDirectDraw()->RestoreAllSurfaces()))
		return hr;


	return hr;
}
void CDirectXDialog::OnPaint() 
{
	if (!IsIconic())
		paintFrame();

	CDialog::OnPaint();
}

void CDirectXDialog::paintFrame()
{
	HRESULT hr;
	if (FAILED(hr = g_pDisplay.GetDirectDraw()->TestCooperativeLevel() ) )
	{
		switch(hr)
		{
			case DDERR_WRONGMODE:
			{
				freeDirectXResources();
				initDirectDraw();
			}
		}
	}

	displayFrame();
	if (g_pDisplay.Present() == DDERR_SURFACELOST)
	{
		restoreSurfaces();
		displayFrame();
	}
}
void CDirectXDialog::freeDirectXResources()
{
	SAFE_DELETE(g_pSecondarySurface);
}
HRESULT CDirectXDialog::initDirectDraw()
{
	HRESULT hr;
	CRect rect;
	GetClientRect(&rect);
	if (rect.Height() <= 0)
		return DD_OK;
	// ZU ERLEDIGEN: Hier zusätzliche Initialisierung einfügen
	if (FAILED( hr = g_pDisplay.CreateWindowedDisplay(m_hWnd, rect.Width(), rect.Height())))
	{
		AfxMessageBox("Failed initializing DirectDraw");
		return hr;
	}

	if (FAILED(hr = g_pDisplay.CreateSurface(&g_pSecondarySurface, rect.Width(), rect.Height() )))
	{
		AfxMessageBox("Failed creating DirectDraw Surface");
		return hr;
	}

	return DD_OK;
}


//http://www.geocities.com/foetsch/locking/locking.htm
HRESULT CDirectXDialog::BackbufferLock()
{
	HRESULT hr;
	sDesc.dwSize=sizeof(sDesc);
	if (FAILED(hr = ((IDirectDrawSurface7*)g_pDisplay.GetBackBuffer())->Lock(NULL, &sDesc, DDLOCK_WAIT, NULL)))
		return hr;
	backbuffervideodata = (char*)sDesc.lpSurface;

	DDPIXELFORMAT pf = sDesc.ddpfPixelFormat;
	assert(sDesc.dwFlags & DDSD_PITCH);
	assert(sDesc.dwFlags & DDSD_PIXELFORMAT);
	assert(pf.dwFlags & DDPF_RGB);

	rbits = CDirectXDialog::getNumberOfBits(pf.dwRBitMask);
	gbits = CDirectXDialog::getNumberOfBits(pf.dwGBitMask);
	bbits = CDirectXDialog::getNumberOfBits(pf.dwBBitMask);

	rpos = CDirectXDialog::getBitMaskPosition(pf.dwRBitMask);
	gpos = CDirectXDialog::getBitMaskPosition(pf.dwGBitMask);
	bpos = CDirectXDialog::getBitMaskPosition(pf.dwBBitMask);

	if (rbits == 8 && gbits == 8 && bbits == 8)
		setPixel = &CDirectXDialog::setPixelOPTIMIZED;
	else
		setPixel = &CDirectXDialog::setPixelSECURE;


	y_pitch = sDesc.lPitch;	
	x_pitch = pf.dwRGBBitCount >> 3;

	return DD_OK;
}

void CDirectXDialog::BackbufferUnlock()
{
	((IDirectDrawSurface7*)g_pDisplay.GetBackBuffer())->Unlock(NULL);
}

inline void CDirectXDialog::setPixelOPTIMIZED(int x, int y, DWORD color)
{
	*(unsigned int*)(backbuffervideodata + x*x_pitch + y*y_pitch) = color;
}


inline void CDirectXDialog::setPixelSECURE(int x, int y, DWORD color)
{
	int offset = x*x_pitch + y*y_pitch;
	DWORD Pixel = *(LPDWORD)((DWORD)backbuffervideodata + offset);
	

	Pixel = (Pixel & ~ sDesc.ddpfPixelFormat.dwRBitMask) | ((RGB_GETRED(color) >> (8 - rbits)) << rpos);
	Pixel = (Pixel & ~ sDesc.ddpfPixelFormat.dwGBitMask) | ((RGB_GETGREEN(color) >> (8 - gbits)) << gpos);
	Pixel = (Pixel & ~ sDesc.ddpfPixelFormat.dwBBitMask) | ((RGB_GETBLUE(color) >> (8 - bbits)) << bpos);

	*(unsigned int*)(backbuffervideodata + offset) = Pixel;
}

CDirectXLockGuard::CDirectXLockGuard(CDirectXDialog* dlg) : dialog(dlg) 
{
	if (hr = dialog->BackbufferLock() == DDERR_SURFACELOST)
	{
		dialog->restoreSurfaces();		
		hr = dialog->BackbufferLock();
	}
}
CDirectXLockGuard::~CDirectXLockGuard() 
{
	if (!FAILED(hr)) 
		dialog->BackbufferUnlock();
}



/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CDirectXDialog 

void CDirectXDialog::OnDestroy() 
{
	CDialog::OnDestroy();
	
	freeDirectXResources();
}

