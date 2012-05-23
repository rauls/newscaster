//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx3d/cdx3dframe.h,v $
// $Author: mindcry $
//
// $Log: cdx3dframe.h,v $
// Revision 1.2  2000/08/24 19:16:06  mindcry
// Now uses the IDirect3DRM3 Interface. Thanks to Daniel Polli.
//
// Revision 1.1.1.1  2000/04/22 16:08:30  hebertjo
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
#ifndef CDX3DFRAME_H
#define CDX3DFRAME_H

//////////////////////////////////////////////////////////////////////////////////
// CRMFrame Class
//////////////////////////////////////////////////////////////////////////////////
class CRMFrame
{
public:
        CRMFrame(void);
        CRMFrame(CRMEngine* pEngine);
        ~CRMFrame(void);

        BOOL Create(CRMFrame* Parent);
        void AddChild(CRMFrame* pChild);
        void AddLight(CRMLight* pLight);
        void SetDirection(double dx, double dy, double dz, double ux, double uy, double uz,
                          CRMFrame* pRef = NULL);
        void GetDirection(double &dx, double &dy, double &dz, double &ux, double &uy, double &uz,
                          CRMFrame* pRef = NULL);
        void SetPosition(double x, double y, double z, CRMFrame* pRef = NULL);
        void GetPosition(double &x, double &y, double &z, CRMFrame* pRef = NULL);
        void SetRotation(double x, double y, double z, double t, CRMFrame* pRef = NULL);
        void GetRotation(double &x, double &y, double &z, float t, CRMFrame* pRef = NULL);
        LPDIRECT3DRMFRAME3 GetRef(CRMFrame* pRef);
        HRESULT Move(double delta);
//        HRESULT AddTransform(CRMMatrix& m, D3DRMCOMBINETYPE ct = D3DRMCOMBINE_AFTER);
        HRESULT AddRotation(double x, double y, double z, double t,
                            D3DRMCOMBINETYPE ct = D3DRMCOMBINE_AFTER);

public:
        CRMEngine* m_Engine;
        LPDIRECT3DRMFRAME3 m_Frame;
};

#endif
