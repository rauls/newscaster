//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx3d/cdx3d.h,v $
// $Author: hebertjo $
//
// $Log: cdx3d.h,v $
// Revision 1.1.1.1  2000/04/22 16:08:28  hebertjo
// Initial checkin of v3.0 to SourceForge CVS.
//
// Revision 2.2  1999/07/11 17:50:52  MICHAELR
// define for D3DError
//
// Revision 2.1  1999/07/02 17:26:37  MICHAELR
// Added necessary CDXINCLUDEALL define
// rearranged includes to include cdx.h first
//
// Revision 2.0  1999/05/01 13:51:18  bsimser
// Updated revision number to 2.0
//
// Revision 1.1.1.1  1999/05/01 04:10:56  bsimser
// Initial revision to cvs
//
// $Revision: 1.1.1.1 $
//////////////////////////////////////////////////////////////////////////////////
#ifndef CDX3D_H
#define CDX3D_H

#define CDXINCLUDEALL
#include <cdx.h>
#include <d3drmwin.h>

//////////////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////////////
extern "C"
{
	// Direct3D Errors
	extern void D3DError(HRESULT hErr, void* hWnd, LPCSTR szFile, int iLine);
}

class CRMScene;
class CRMCamera;
class CRMLight;

#include "cdx3dengine.h"

#include "cdx3dframe.h"

#include "cdx3dscene.h"

#include "cdx3dcamera.h"

#include "cdx3dlight.h"

#include "cdx3dshape.h"

//////////////////////////////////////////////////////////////////////////////////
// Macros
//////////////////////////////////////////////////////////////////////////////////
#define RELEASE(x) if(x != NULL) { x->Release(); x = NULL; }
#define SAFEDELETE(x) if(x != NULL) { delete x; x = NULL; }

#endif
