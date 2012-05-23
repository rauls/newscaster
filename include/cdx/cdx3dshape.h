//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx3d/cdx3dshape.h,v $
// $Author: mindcry $
//
// $Log: cdx3dshape.h,v $
// Revision 1.2  2000/08/24 19:16:43  mindcry
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
#ifndef CDX3DSHAPE_H
#define CDX3DSHAPE_H

////////////////////////////////////////////////////////////////////////////////
// CRMShape Class
////////////////////////////////////////////////////////////////////////////////
class CRMShape : public CRMFrame
{
public:
        CRMShape(CRMEngine* pEngine);
        ~CRMShape(void);

        BOOL Create(D3DVECTOR* pVectors, int nVectors, D3DVECTOR* pNormals, 
			int Normals,THIS_ D3DVALUE crease,
			int* pFaceData, BOOL bAutoGen = FALSE,DWORD dwFlags=D3DRMGENERATENORMALS_PRECOMPACT);
        BOOL CreateCube(double x, double y, double z);
        BOOL CreateSphere(double r, int nBands);
        BOOL Load(const char* szFilename);
        HRESULT SetColor(double r, double g, double b);

public:
        LPDIRECT3DRMMESHBUILDER3 m_MeshBld;
};

#endif
