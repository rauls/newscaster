//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx3d/cdx3dcamera.h,v $
// $Author: mindcry $
//
// $Log: cdx3dcamera.h,v $
// Revision 1.2  2000/08/24 19:17:26  mindcry
// Now uses the IDirect3DRM3 Interface. Thanks to Daniel Polli.
//
// Revision 1.1.1.1  2000/04/22 16:08:28  hebertjo
// Initial checkin of v3.0 to SourceForge CVS.
//
// Revision 2.0  1999/05/01 13:51:18  bsimser
// Updated revision number to 2.0
//
// Revision 1.1.1.1  1999/05/01 04:10:56  bsimser
// Initial revision to cvs
//
// $Revision: 1.2 $
//////////////////////////////////////////////////////////////////////////////////
#ifndef CDX3DCAMERA_H
#define CDX3DCAMERA_H

//////////////////////////////////////////////////////////////////////////////////
// CRMCamera Class
//////////////////////////////////////////////////////////////////////////////////
class CRMCamera : public CRMFrame
{
public:
        CRMCamera(CRMEngine* pEngine);
        ~CRMCamera(void);

        BOOL Create(CRMScene* pScene);
        HRESULT Clear(DWORD dwFlags);
        HRESULT Render(CRMScene* pScene);
        HRESULT ForceUpdate(DWORD X1, DWORD Y1, DWORD X2, DWORD Y2);

public:
        LPDIRECT3DRMVIEWPORT2 m_View;
};

#endif
