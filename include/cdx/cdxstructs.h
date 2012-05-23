//////////////////////////////////////////////////////////////////////////////////
// $Source: /cvsroot/cdx/cdx3.0/src/cdx/cdxstructs.h,v $
// $Author: hebertjo $
//
// $Log: cdxstructs.h,v $
// Revision 1.1.1.1  2000/04/22 16:12:04  hebertjo
// Initial checkin of v3.0 to SourceForge CVS.
//
// Revision 1.1  2000/01/13 23:33:23  jhebert
// New header file for structures used in CDX.
//
//
//
// $Revision: 1.1.1.1 $
//////////////////////////////////////////////////////////////////////////////////
#ifndef CDXSTURCTURES_H
#define CDXSTURCTURES_H

// This file is used to hold structure definitions that might be used
// thrroughout the CDX library.

typedef struct
{
    RGBQUAD depth;		// How many bits of precision per channel
    RGBQUAD position;	// At what bit position does the color start
	UINT    bpp;        // Bits Per Pixel
} RGBFORMAT;


#endif CDXSTURCTURES_H
