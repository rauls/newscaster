/*
 * (c) Raul Sobon.
 * All rights reserved. .
 */

/**
  * Newsflash.cpp: implementation of the Newsflash graphics class.
  *
  * Author: Nick Mouskos
  *
  */

// 
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Newsflash.h"
#include "Widgie.h"
#include "WidgieDlg.h"
#include "utils.h"

/** 
 *  Constructor for Newsflash
 *
 *  Loads graphics setting from the initialisation file
 *
 *	Initialises graphics ready to display a newsflash.
 * 
 */
Newsflash::Newsflash(CDialog* pDialog)
{
   
	textPosition = CFG->cfgWidth();

	/* Loading Newsflash strings */
	NewsString = " ";
    pNewsText = NewsString.GetBuffer(NewsString.GetLength());

	/* Get a handle to the effects drawing will be done to this handle */
	//theScreen = CreateDC("DISPLAY", NULL, NULL, NULL);// handle to the screen;

	/* Get a handle to the dialog that the newsflash rect will be displayed on */
	CDC* pDC;
    pDC = pDialog->GetDC();
    newsDisplay = pDC->m_hDC;

	/* These are the hadles for manipulating the newsflash and the JPEG
	   and Shockwave (Flash)Content*/
	newsDlg = pDialog;


	/* Setting up the other buffers to implement double buffering */
  	tempImage = CreateCompatibleDC(newsDisplay);
	BlankImage = CreateCompatibleDC(newsDisplay);

   
	secondBuffer = CreateCompatibleBitmap(newsDisplay, CFG->cfgWidth()*2, CFG->cfgBarHeight );
    BlankBuffer = CreateCompatibleBitmap(newsDisplay, CFG->cfgWidth()*2, CFG->cfgBarHeight );
	
	if (secondBuffer && !SelectObject(tempImage, secondBuffer))
	{
#ifdef _DEBUG2
		MessageBox( NULL, "Select Object Failed", "Error", MB_OK | MB_ICONINFORMATION );
#endif
	}

	if (BlankBuffer && !SelectObject(BlankImage, BlankBuffer))
	{
#ifdef _DEBUG2
		MessageBox( NULL, "Select Object Failed", "Error", MB_OK | MB_ICONINFORMATION );
#endif
	}


    /* Setting up the font for the newsflash display */
	SetTextColor( tempImage, CFG->cfgTextColor );
	SetBkColor( tempImage, CFG->cfgBackgroundColor );
    SetBkMode( tempImage, OPAQUE );		//TRANSPARENT

	newsFont = CreateFont(CFG->cfgFontHeight,0, 0, 0,FW_NORMAL, FALSE, FALSE, FALSE,
			  		    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS,
					    ANTIALIASED_QUALITY, DEFAULT_PITCH + FF_DONTCARE, CFG->cfgFontFace);
    SetTextColor( newsDisplay, CFG->cfgTextColor );
	SetBkColor( newsDisplay, CFG->cfgBackgroundColor );
    SetBkMode( newsDisplay, OPAQUE );

    SelectObject( newsDisplay, newsFont );

	LoadScrollImage();

	//lpNewsFlashSize = &newsFlashSize;

	//GetTextExtentPoint32(tempImage, pNewsText, strlen(pNewsText), lpNewsFlashSize);
	ReleaseDC( pDialog->m_hWnd, (HDC)pDC );
}


Newsflash::~Newsflash()
{
	OutDebugs( "Newsflash::~Newsflash() ..." );
	if( bm_scrollerLogo )
		DeleteObject( bm_scrollerLogo );
	DeleteObject( newsFont );
	DeleteObject( BlankBuffer );
	DeleteObject( secondBuffer );
	OutDebugs( "Newsflash::~Newsflash() Done." );
}


void Newsflash::LoadScrollImage()
{
	if( bm_scrollerLogo )
		DeleteObject( bm_scrollerLogo );

    CString filetoload = CFG->cfgLocalImagesDir + CFG->cfgScrollLogoFileName;

	// load image defined
	bm_scrollerLogo = (HBITMAP)LoadImage( AfxGetInstanceHandle(), filetoload.GetBuffer(0), IMAGE_BITMAP,0,0, LR_LOADFROMFILE );
	// if failed, try orig default
	if( !bm_scrollerLogo )
	{
		filetoload = CFG->cfgLocalImagesDir + SCROLLNEWSLOGO_FN;
		bm_scrollerLogo = (HBITMAP)LoadImage( AfxGetInstanceHandle(), filetoload.GetBuffer(0), IMAGE_BITMAP,0,0, LR_LOADFROMFILE );
	}

	if( bm_scrollerLogo )
	{
		BITMAPINFO bInfo;
		ZeroMemory(&bInfo, sizeof(bInfo));
		bInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bInfo.bmiHeader.biBitCount = 0;

		if (!GetDIBits( ::GetDC(this->newsDlg->m_hWnd), bm_scrollerLogo, 0, 0, NULL, &bInfo, DIB_RGB_COLORS))
		{
				DWORD error = GetLastError();
				ASSERT(FALSE);
		}
		logoDim.cx = bInfo.bmiHeader.biWidth;
		logoDim.cy = bInfo.bmiHeader.biHeight;
	}
}



long Scale_RGB( double perc,long rgb )
{
	long r,g,b;

	r = (rgb & 0xff0000) >> 16;
	g = (rgb & 0xff00) >> 8;
	b = (rgb & 0xff);

	r = (long)(r * perc / 100.0);
	if ( r < 0 ) r = 0;
	if ( r > 255 ) r = 255;

	g = (long)(g * perc / 100.0);
	if ( g < 0 ) g = 0;
	if ( g > 255 ) g = 255;

	b = (long)(b * perc / 100.0);
	if ( b < 0 ) b = 0;
	if ( b > 255 ) b = 255;

	return ( r<<16 | g<<8 | b );
}


/** 
 * Display a newflash Caption 
 *  		 
 * Repeated calls to this function implement scrolling motion of the newsflash
 * Because it also updates the next position to display the caption.
 * 
 */

void ClearRect( HDC hdc, int x, int y, int w, int h )
{
	RECT rc;
	rc.top = y;
	rc.bottom = y+h;
	rc.left = x;
	rc.right = x+w;

	HBRUSH bgBrush = CreateSolidBrush(0); 
	FillRect( hdc, &rc, bgBrush );
}


#include <mmsystem.h>

// ##
// ## Move scroller 1 unit to the left, if at the end, update the new text.
// ##
// ##
// ##
BOOL Newsflash::AdvanceText()
{
	long time1 = timeGetTime();
	int width = CFG->cfgWidth();

    pNewsText = NewsString.GetBuffer(NewsString.GetLength());

    /* Calculate how long the text is (in pixels) */    
  	lpNewsFlashSize = &newsFlashSize;

    
    BOOL scrolling = TRUE;

	// ok init the text images and buffers for scrolling
	if( textPosition >= width )
	{
		CString resToken;
		int curPos=0, newsSegments = 1;

		AFXDIALOG->newsThread->SetThreadPriority( THREAD_PRIORITY_HIGHEST );		//THREAD_PRIORITY_NORMAL

		resToken= NewsString.Tokenize("\t",curPos);
		while (resToken != "")
		{
			newsSegments++;
			resToken= NewsString.Tokenize("\t",curPos);
		};

		// get text width in pixels
		GetTextExtentPoint32(tempImage, pNewsText, strlen(pNewsText), lpNewsFlashSize);
		// add the amount of logos * 32*3
		lpNewsFlashSize->cx += (newsSegments*32*3);

		DeleteObject( secondBuffer );
		secondBuffer = CreateCompatibleBitmap( newsDisplay, newsFlashSize.cx + width, newsFlashSize.cy*2 );
		if( secondBuffer )
		{
			SelectObject(tempImage, secondBuffer);
		} else {
			OutDebugs( "ERROR: news scroller is TOOO WIDE at %dpx*%dpx (%d chars), please reduce news segments from %d items", 
				newsFlashSize.cx + width, newsFlashSize.cy*2, strlen(pNewsText), newsSegments );
			newsFlashSize.cx = 31900;
			secondBuffer = CreateCompatibleBitmap( newsDisplay, newsFlashSize.cx + width, newsFlashSize.cy*2 );	
		}

		if( secondBuffer == NULL )
		{
			return (scrolling);
		}

		if( AFXDIALOG->brightnessLevelLow )
			SetTextColor( tempImage, Scale_RGB( 80.0, CFG->cfgTextColor ) );
		else
			SetTextColor( tempImage, CFG->cfgTextColor );

		int bg = CFG->cfgBackgroundColor;
		SetBkColor( tempImage, bg );
		SetBkMode( tempImage, OPAQUE );		//TRANSPARENT

		// make intermediate image BLACK
		BitBlt(tempImage, 0, 0, width*2, CFG->cfgBarHeight, BlankImage, 0, 0, SRCCOPY);
		HBRUSH bgBrush = CreateSolidBrush(0); 
		RECT rc = { 0, 0, width*2, CFG->cfgBarHeight };
		//FillRect( tempImage, &rc, bgBrush );

		// Write the newsflash to the intermediate Image 
		if( newsFlashSize.cx > 15 )
		{
			// do multi line news
			if( newsSegments > 1 )
			{
				int xpos = 0;
				curPos = 0;
				resToken = NewsString.Tokenize("\t",curPos);
				while( resToken != "" )
				{
					SIZE textWidth;
				    SelectObject( tempImage, newsFont );
					GetTextExtentPoint32( tempImage, resToken, strlen(resToken), &textWidth );
					TextOut(tempImage, xpos, CFG->cfgBarHeight/2 - CFG->cfgFontHeight/2, resToken.GetBuffer(0), resToken.GetLength() );

					xpos += textWidth.cx;
					if( bm_scrollerLogo && textWidth.cx>8 )
					{
						HGDIOBJ old = SelectObject(BlankImage, bm_scrollerLogo);
						xpos+=32;
						//StretchBlt( tempImage, xpos, 0, CFG->cfgBarHeight, CFG->cfgBarHeight,	BlankImage, 0, 0, SRCCOPY);
						StretchBlt( tempImage, xpos, 0, logoDim.cy, CFG->cfgBarHeight,	BlankImage, 0, 0, logoDim.cx, logoDim.cy, SRCCOPY);
						xpos+=logoDim.cx;
						xpos+=32;
						SelectObject(BlankImage, old);
					}
					resToken= NewsString.Tokenize("\t",curPos);
				}

			} else
			{
				TextOut(tempImage, 0, CFG->cfgBarHeight/2 - CFG->cfgFontHeight/2, pNewsText, strlen(pNewsText));
				if( bm_scrollerLogo && strlen(pNewsText)>2 && newsFlashSize.cx>8 )
				{
					HGDIOBJ old = SelectObject(BlankImage, bm_scrollerLogo);
					//BitBlt( tempImage, newsFlashSize.cx+32, 0, CFG->cfgBarHeight, logoDim.cy,	BlankImage, 0, 0, SRCCOPY);
					StretchBlt( tempImage, newsFlashSize.cx+32,0, logoDim.cx, CFG->cfgBarHeight,	BlankImage, 0, 0, logoDim.cx, logoDim.cy, SRCCOPY);
					SelectObject(BlankImage, old);
				}
			}
		}

		// make sure the image on screen is BLACK just in case
		BitBlt( newsDisplay, 0, 0, width, CFG->cfgBarHeight, BlankImage, 0, 0, SRCCOPY);
		{
			HBRUSH bgBrush = CreateSolidBrush(0); 
			RECT rc = { 0, 0, width, CFG->cfgBarHeight };
			//FillRect( newsDisplay, &rc, bgBrush );
		}

	    AFXDIALOG->newsThread->SetThreadPriority( THREAD_PRIORITY_HIGHEST );

		//BitBlt(	newsDisplay, 0, 0, CFG->cfgWidth(), CFG->cfgBarHeight,
		//		tempImage, -textPosition, 0, SRCCOPY);
	}
	
	textPosition -= CFG->cfgStepSpeed;
	if (textPosition >= (-newsFlashSize.cx))
	{
		// Copy the intermediate Image to the screen
		BitBlt(	newsDisplay, 0, 0,  width, CFG->cfgBarHeight, tempImage, -textPosition, 0, SRCCOPY);
	}
	else
	{
		// Reset the textPosition ready for the next newsflash 
		textPosition = width;

		// Let the main application know that we are done showing the newsflash 
		scrolling = FALSE;
	}

	time1 = (timeGetTime()-time1);
	CFG->m_timetaken_ms = time1;

	//OutDebugs( "test pos = %d", textPosition );
	return (scrolling);
}

void Newsflash::displayCentered(CString displayString)
{
	if( displayString && displayString.IsEmpty()==FALSE )
	{
		/* Make sure that the newsbar is showing */
		newsDlg->MoveWindow(CFG->cfgLeft, CFG->cfgBottom - CFG->cfgBarHeight,
							CFG->cfgWidth(), CFG->cfgBarHeight, TRUE);


		textPosition = CFG->cfgWidth();

		/* Clear the contents of the newsbar */
		CDC* dialogDC;
		dialogDC = newsDlg->GetDC();

		SetTextColor( newsDisplay, CFG->cfgTextColor );
		SetBkColor( newsDisplay, CFG->cfgBackgroundColor );
		SetBkMode( newsDisplay, OPAQUE );		//TRANSPARENT

		CRect myRect;
		newsDlg->GetClientRect(&myRect);

		long col = CFG->cfgBackgroundColor;
		CBrush bgBrush;
		bgBrush.CreateSolidBrush( col );

		dialogDC->FillRect( &myRect, &bgBrush );

		dialogDC->FillRect( &myRect, &bgBrush );

		/* Calculate where the string should be displayed */
		lpNewsFlashSize = &newsFlashSize;
	    
		/* Display the string */
		myRect.top += 5;
		DrawText( newsDisplay, displayString, strlen(displayString), &myRect, DT_CENTER );

		/* Put text position to the right side of the screen so that it is ready for the next newsflash */
		textPosition = CFG->cfgWidth();

		newsDlg->ReleaseDC( dialogDC );
	}
}

           