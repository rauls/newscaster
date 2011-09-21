// Newsflash.h: interface for the Newsflash class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_NEWSFLASH__H_)
#define _NEWSFLASH__H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string.h>
#include <time.h>

class Newsflash  
{
	public:
        virtual ~Newsflash();               // See C++ implementation

		CString NewsString;                 // The current news caption

        HDC BlankImage;                     // background image

        HDC tempImage;                      // off-screen buffer where the the backgound and
                                            // text are assebled before copyingto the screen

        int textPosition;                   // the y co-ordinate of the position where to
                                            // draw the the news caption

        Newsflash(CDialog* pDialog);        // See C++ implementation
		
		BOOL DisplayBar(int appliedEffect); // See C++ implementation
		
        //HideBar(int appliedEffect);         // See C++ implementation
		
        BOOL AdvanceText();                 // See C++ implementation
		
        BOOL BarActive();                   // See C++ implementation

        void displayCentered(CString displayString);

		void LoadScrollImage();
		
   
	private:
        
        const char *pNewsText;      // char* representation of the news caption

		HDC newsDisplay;            // handle to window of the news bar dialog 
		
		CDialog* newsDlg;           // handle to the news bar dialog
			
		HFONT newsFont;

		SIZE newsFlashSize;         // the length of the newsflash text when rendered
		LPSIZE lpNewsFlashSize;     // a pointed to the newsFlashSize

		HBITMAP	bm_scrollerLogo;
		SIZE logoDim;

		 	
		HBITMAP secondBuffer;       // byffer used when constructing the 
                                    // tempImage HDC
   		HBITMAP BlankBuffer;        // byffer used when constructing the 
                                    // BlankImage HDC

};

#endif // !defined(_NEWSFLASH__H_)
