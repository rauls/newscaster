//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx3d/cdx3dlight.h,v $
// $Author: hebertjo $
//
// $Log: cdx3dlight.h,v $
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
#ifndef CDX3DLIGHT_H
#define CDX3DLIGHT_H

//////////////////////////////////////////////////////////////////////////////////
// CRMLight Class
//////////////////////////////////////////////////////////////////////////////////
class CRMLight : public CRMFrame
{
public:
        CRMLight(CRMEngine* pEngine);
        ~CRMLight(void);

        BOOL Create(D3DRMLIGHTTYPE Type, double r, double g, double b);
        BOOL CreateAmbient(double r, double g, double b);
        BOOL CreateDirectional(double r, double g, double b);
        BOOL CreatePoint(double r, double g, double b);
        BOOL CreateParallelPoint(double r, double g, double b);
        BOOL CreateSpot(double r, double g, double b);
        void SetColor(double r, double g, double b);

public:
        LPDIRECT3DRMLIGHT m_Light;
};

#endif
