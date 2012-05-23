//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx/cdxdebug.h,v $
// $Author: hebertjo $
//
// $Log: cdxdebug.h,v $
// Revision 1.1.1.1  2000/04/22 16:09:12  hebertjo
// Initial checkin of v3.0 to SourceForge CVS.
//
// Revision 1.1  1999/05/30 03:32:04  MICHAELR
// add to the repository
//
//
//
// $Revision: 1.1.1.1 $
//////////////////////////////////////////////////////////////////////////////////
#ifndef CDXDEBUG_H
#define CDXDEBUG_H

#include <assert.h>

#ifdef  NDEBUG

#define CDXASSERT(exp)     ((void)0)

#else

#define CDXASSERT(exp) (void)( (exp) || (_assert(#exp, __FILE__, __LINE__), 0) )

#endif

#endif CDXDEBUG_H
