// NewsXML_Handler.h: interface for the NewsXML_Handler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NewsXML_Handler_H__B878C8DD_8F2C_4949_9592_733604A86FDD__INCLUDED_)
#define AFX_NewsXML_Handler_H__B878C8DD_8F2C_4949_9592_733604A86FDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "xercesc/sax2/DefaultHandler.hpp"
#include <afxtempl.h>
#include "NewsflashInfo.h"

XERCES_CPP_NAMESPACE_USE


enum NewsSource 
{
	NEWS_CUSTOM,
	NEWS_YAHOO,
	NEWS_SEVEN,
	NEWS_FOX,
	NEWS_BBC,
	NEWS_CNN,
	NEWS_BLOOMBERG,
	NEWS_CBS,
	NEWS_EURO
};


enum NewsState                  // all possible states of newsflash 
{                               // state machine
   FINDING_NEWSFLASH,
   FOUND_NEWSFLASH
};                


enum CharacterState             // all possible states of characters 
{                               // state machine
   NEWS_TEXT,
   NEWS_DATE,
   NEWS_TIME,
   NEWS_INVALID
};                




class NewsXML_Handler : public DefaultHandler
{
public:
	
    // See C++ implementation
    NewsXML_Handler(CEdit* debugEdit, 
                    CList <NewsflashData, NewsflashData>* newsCaptionList);

	~NewsXML_Handler();                     // See C++ implementation

	void Reset_Handler(CEdit* debugEdit, 
                    CList <NewsflashData, NewsflashData>* newsCaptionList);

	void startElement(                              // See C++ implementation
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attrs
    );

	void endElement(                                // See C++ implementation
		const	XMLCh* const,
		const	XMLCh* const,
		const	XMLCh* const
	);

    void fatalError(const SAXParseException&);      // See C++ implementation

	void characters(const   XMLCh* const    chars,  // See C++ implementation
					const unsigned int    length);

private:
	CEdit* m_OutputEdit;                // reference to edit control to send debug text
	
	bool gotText;
	bool gotDate;
	bool gotTime;

	NewsSource	newsSource;
    
    NewsState XML_State;                // current state of newsflash
                                        // state machine

	CharacterState XML_CharState;       // current state of charecters
                                        // state machine

	// structure to hold elements of the newsflash as they are found
    NewsflashInfo currentNewsFlash;

    // list to hold complete newsflashes as they are found
    CList <NewsflashData, NewsflashData>* m_NewsCaptions;

   	void Debug(CString debugMessage);   // See C++ implementation


};

#endif // !defined(AFX_NewsXML_Handler_H__B878C8DD_8F2C_4949_9592_733604A86FDD__INCLUDED_)
