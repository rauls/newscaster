//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx3d/cdx3dscene.h,v $
// $Author: hebertjo $
//
// $Log: cdx3dscene.h,v $
// Revision 1.1.1.1  2000/04/22 16:08:30  hebertjo
// Initial checkin of v3.0 to SourceForge CVS.
//
// Revision 2.0  1999/05/01 13:51:18  bsimser
// Updated revision number to 2.0
//
// Revision 1.1.1.1  1999/05/01 04:10:56  bsimser
// Initial revision to cvs
//
// $Revision: 1.1.1.1 $
//////////////////////////////////////////////////////////////////////////////////
#ifndef CDX3DSCENE_H
#define CDX3DSCENE_H

//////////////////////////////////////////////////////////////////////////////////
// CRMScene Class
//////////////////////////////////////////////////////////////////////////////////
class CRMScene : public CRMFrame
{
public:
        CRMScene(CRMEngine* pEngine);
        ~CRMScene(void);

        BOOL Create(void);
        HRESULT SetAmbientLight(double r, double g, double b);

public:
        LPDIRECT3DRMLIGHT m_Light;
};

#endif
