//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx3d/cdx3dengine.h,v $
// $Author: mindcry $
//
// $Log: cdx3dengine.h,v $
// Revision 1.2  2000/08/24 19:15:11  mindcry
// Now uses the IDirect3DRM3 Interface. Thanks to Daniel Polli.
//
// Revision 1.1.1.1  2000/04/22 16:08:30  hebertjo
// Initial checkin of v3.0 to SourceForge CVS.
//
// Revision 2.2  1999/11/24 19:44:13  MICHAELR
// Fixed return code problem, line 144 (error==DD_OK), should  have been !=
// Modified FindDevice so it does not return a local stack pointer to the GUID
// Thanks to Nick Lawroff for finding these problems
//
// Revision 2.1  1999/07/15 19:25:48  MICHAELR
// added InitCRMEngine() to class def
//
// Revision 2.0  1999/05/01 13:51:18  bsimser
// Updated revision number to 2.0
//
// Revision 1.1.1.1  1999/05/01 04:10:56  bsimser
// Initial revision to cvs
//
// $Revision: 1.2 $
//////////////////////////////////////////////////////////////////////////////////
#ifndef CDX3DENGINE_H
#define CDX3DENGINE_H

//////////////////////////////////////////////////////////////////////////////////
// CRMEngine Class
//////////////////////////////////////////////////////////////////////////////////
class CRMEngine
{
public:
        CRMEngine(void);
        ~CRMEngine(void);

        HRESULT CreateFullScreen(CDXScreen* pScreen, void *hWnd, int Width, int Height, int BPP);
        HRESULT CreateWindowed(CDXScreen* pScreen, void *hWnd, int Width, int Height);

		LPGUID FindDevice(int BPP, LPGUID OUT guid);
		//LPGUID FindDevice(int BPP);
		void SetDefaults(void);

        HRESULT CreateFrame(LPDIRECT3DRMFRAME3 pParent, LPDIRECT3DRMFRAME3* pFrame);
        HRESULT CreateLight(D3DRMLIGHTTYPE type, double r, double g, double b,
                            LPDIRECT3DRMLIGHT* pLight);
        HRESULT CreateMeshBuilder(LPDIRECT3DRMMESHBUILDER3* pMeshBld);

        HRESULT Update(void);
        HRESULT Tick(double Tick);
        HRESULT SetDither(BOOL Dither);
        HRESULT SetQuality(D3DRMRENDERQUALITY Quality);
        HRESULT SetShades(DWORD Shades);
        HRESULT SetTextureQuality(D3DRMTEXTUREQUALITY TextureQuality);

        LPDIRECT3D2 GetD3D(void) { return m_Direct3D; }
        LPDIRECT3DRM3 GetD3DRM(void) { return m_Direct3DRM; }

public:
        LPDIRECT3D2 m_Direct3D;
        LPDIRECT3DRM3 m_Direct3DRM;
        LPDIRECT3DDEVICE2 m_IMDevice;
        LPDIRECT3DRMDEVICE3 m_RMDevice;
		LPDIRECTDRAWCLIPPER m_lpDDClipper;
        CDXScreen* m_Screen;

private:
		DWORD bppToDbd(int bpp);
		void InitCRMEngine();
};

#endif
