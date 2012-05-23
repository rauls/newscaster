// LoopSeqXML_Handler.h: interface for the LoopSeqXML_Handler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOOPSEQXML_HANDLER_H__4ABF45EC_4F9F_4379_BB3E_7BF33AB2C881__INCLUDED_)
#define AFX_LOOPSEQXML_HANDLER_H__4ABF45EC_4F9F_4379_BB3E_7BF33AB2C881__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "xercesc/sax2/DefaultHandler.hpp"
#include <afxtempl.h>
#include "LoopSeqInfo.h"

XERCES_CPP_NAMESPACE_USE


enum ImpressionState            // all possible states of Loop sequence 
{                               // state machine
   FINDING_IMPRESSION,
   FOUND_IMPRESSION,
   FOUND_DEFAULTS
};                


enum ImpressionCharacterState   // all possible states of characters 
{                               // state machine
	IMPRESSION_ID,
	IMPRESSION_ORDER,
	IMPRESSION_PLAYLENGTH,
	IMPRESSION_VOLUME,
	IMPRESSION_LANGUAGE,
	IMPRESSION_TITLE,
	IMPRESSION_IMAGE_ID,
	IMPRESSION_IMAGE_CID,
	IMPRESSION_IMAGE,
	IMPRESSION_IMAGEURL,
	IMPRESSION_MULTIMEDIA,
	IMPRESSION_PRINT,
	IMPRESSION_VOICE,
	IMPRESSION_VOICESTATUS,
	IMPRESSION_VOICETIME1,
	IMPRESSION_VOICETIME2,
	IMPRESSION_VOICEDAYS,
	IMPRESSION_VOICEDURATION,
	IMPRESSION_INVALID,
	IMPRESSION_FIELD,

	DEFAULT_CONFIGFILE,
	DEFAULT_NAME,

	DEFAULT_LANGUAGE,
	DEFAULT_LANGUAGEFLAG,
	DEFAULT_VOLUME,
	DEFAULT_BRIGHTNESS,
	DEFAULT_DIALUP_NUMBER,
	DEFAULT_HTTP_SERVER,
	DEFAULT_LOCAL_SERVER,

    DEFAULT_FTP_SERVER,
	DEFAULT_RESET,
	DEFAULT_NEW_CONFIG,
	DEFAULT_UPDATE_EXE,
	DEFAULT_GPRSAP,
	DEFAULT_CALLBARING,


	DEFAULT_TESTCALLNUMBER,
	DEFAULT_BRIGHTNESSDOWN,
	DEFAULT_BRIGHTNESSUP,
	DEFAULT_BRIGHTNESSDOWNVALUE,
	DEFAULT_BRIGHTNESSUPVALUE,
	DEFAULT_MAXCALLDURATION,
	DEFAULT_DIALTIMEOUT,
	DEFAULT_BGCOLOR,
	DEFAULT_CALLINGPROMPT,
	DEFAULT_CALLINGMSG,
	DEFAULT_CALLINGMSG2,
	DEFAULT_CALLANSWERED,
	DEFAULT_CALLNOANSWER,
	DEFAULT_CALLBUSY,
	DEFAULT_CALLCANCELLED,
	DEFAULT_CALLHANGUP,
	DEFAULT_CALLTHANKYOU,
	DEFAULT_CALLENDING,
	DEFAULT_NOCALLPROMPT,
	DEFAULT_MUTESET,

	DEFAULT_LOGOIMAGE,
	DEFAULT_NEWSIMAGE,
	DEFAULT_SCROLLIMAGE,

    DEFAULT_NEWS_TIME1,
    DEFAULT_NEWS_TIME2,
    DEFAULT_NEWS_TIME3,

	DEFAULT_BUTTONCODEA,
	DEFAULT_BUTTONCODEB,
	
	DEFAULT_NEWS_BACKGROUND,
    DEFAULT_NEWS_FONT,
    DEFAULT_NEWS_LOGO,
    DEFAULT_TEXT_COLOR,
    DEFAULT_FONT_HEIGHT,
    DEFAULT_BAR_HEIGHT,
    DEFAULT_BAR_DELAY,
    DEFAULT_NEWS_INTERVAL,
    DEFAULT_SCROLL_SPEED,
    DEFAULT_STEP_SPEED,
    DEFAULT_CONTENT_PATH,
    DEFAULT_NEWS_FILENAME,
    DEFAULT_MANIFEST_FILENAME,
    DEFAULT_LOOP_SEQUENCE_FILENAME,
    DEFAULT_GET_XML_TIME,
    DEFAULT_GET_XML_DAY,
    DEFAULT_GET_NEWS_TIME,
    DEFAULT_GET_NEWS_DAY,

	DEFAULT_SLEEP_START,
	DEFAULT_SLEEP_STOP,
	DEFAULT_UPLOAD_INTERVAL,

	DEFAULT_VIDEODOWNLOADALWAYS,
	DEFAULT_IMAGESDOWNLOADALWAYS,

    DEFAULT_PARAM01,
    DEFAULT_PARAM02,
    DEFAULT_PARAM03,
    DEFAULT_PARAM04,
    DEFAULT_PARAM05,

	START_DATE,
    END_DATE
};


class LoopSeqXML_Handler : public DefaultHandler 
{
public:
	LoopSeqXML_Handler(CEdit* debugEdit,
					   CList <ImpressionData, ImpressionData>* impressionList,
					   CList <CString, CString>* languageList,
                       CString language);

	void Reset_Handler(CEdit* debugEdit,
					   CList <ImpressionData, ImpressionData>* impressionList,
					   CList <CString, CString>* languageList,
                       CString language);

	virtual ~LoopSeqXML_Handler();                  // See C++ implementation
	
	void startElement(                              // See C++ implementation
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attrs
    );

	void endElement(                                // See C++ implementation
		const	XMLCh* const,
		const XMLCh* const,
		const XMLCh* const
	);

    void fatalError(const SAXParseException&);      // See C++ implementation

	void characters(const   XMLCh* const    chars,  // See C++ implementation
					const unsigned int    length);

    BOOL IsCurrent();                               // See C++ implementation



private:
	CEdit* m_OutputEdit;    // Edit Component used for display during debug

		
	/* Flags that are used to establish if a complete
       impression has been encountered */
    BOOL gotID;             
	BOOL gotOrder;
	BOOL gotPlaylength;
	BOOL gotVolume;
	BOOL gotLanguage;			/* at least 1 language found */
	BOOL gotImage;
	BOOL gotImageType;
	BOOL gotMultimedia;
	BOOL gotMultimediaType;
	BOOL gotPrint;
	BOOL gotPrintType;
	BOOL gotVoice;
	BOOL gotVoiceStatus;
	BOOL gotVoiceTime1;
	BOOL gotVoiceTime2;
	BOOL gotVoiceDuration;
    
    BOOL gotXML_Time;
    BOOL gotXML_Day;

    BOOL gotNEWS_Time;
    BOOL gotNEWS_Day;

	ImpressionState LoopXML_State;          // current state of loop 
                                            // sequence state machine
	
    ImpressionCharacterState XML_CharState; // current state of charecters
                                            // state machine
	

	// list to hold complete impressions as they are found
    CList <ImpressionData, ImpressionData>* pImpressionList; 
    CList <CString, CString>* pLanguageList;
    CList <CString, CString>* pFlagList;
  

    CString contentLanguage;
	
    // structure to hold elements of an impression  
    // of a loop sequence file as they are found
    ImpressionData currentImpression;

   
    CString endDateString;              //Holds raw unformatted start date string

    CString startDateString;            //Holds raw unformatted end date string

	void LoopSeqXML_Handler::InitLanguageList(CList <CString, CString>* languages); // See C++ implementation

    void InitCurrentImpression();       // See C++ implementation

    void InitLanguageSpecificImpression();

    void InsertSorted();                // See C++ implementation

    void AddLanguage(CString languageStr);

	void Debug(CString debugMessage);   // See C++ implementation
};

#endif // !defined(AFX_LOOPSEQXML_HANDLER_H__4ABF45EC_4F9F_4379_BB3E_7BF33AB2C881__INCLUDED_)
