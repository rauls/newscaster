// NewsXML_Handler.cpp: implementation of the NewsXML_Handler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewsXML_Handler.h"
#include "NewsflashInfo.h"
#include "utils.h"

#include <xercesc/sax2/Attributes.hpp>

#include <afxtempl.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destfruction
//////////////////////////////////////////////////////////////////////
/**
  * Constructor for NewsXML Handler.
  *
  * Initialised the data mambers that will be used to collect
  * news flash information.
  *
  * Initialises the state machines that are used by the event
  * handlers that retrieve XML data.
  *
  * @param debugEdit        a pointer to an edit box used for debug
  *                         this can be NULL but only if #define DEBUG_TO_EDIT is
  *                         commented out in teh Debug function for this class
  *
  * @param impressionList   a pointer to a list to place all impression data that
  *                         is found during parsing
  *
  * @param defaults         a pointer to a structure used to store the data found
  *                         in the defaults section of of the loopsequence file.
  *
  */

NewsXML_Handler::NewsXML_Handler(CEdit*	debugEdit, CList <NewsflashData, NewsflashData>* newsCaptionList)
{
	m_OutputEdit = debugEdit;
	m_NewsCaptions = newsCaptionList;

	currentNewsFlash.SetDebug(debugEdit);

	
	XML_State = FINDING_NEWSFLASH;
	XML_CharState = NEWS_INVALID;

	gotText = false;
	gotDate = false;
	gotTime = false;
}


void NewsXML_Handler::Reset_Handler(CEdit*	debugEdit, CList <NewsflashData, NewsflashData>* newsCaptionList)
{
	m_OutputEdit = debugEdit;
	m_NewsCaptions = newsCaptionList;

	currentNewsFlash.SetDebug(debugEdit);

	
	XML_State = FINDING_NEWSFLASH;
	XML_CharState = NEWS_INVALID;

	gotText = false;
	gotDate = false;
	gotTime = false;
}


NewsXML_Handler::~NewsXML_Handler()
{
	OutDebugs( "NewsXML_Handler::~NewsXML_Handler()" );
}

/**
  * Event handler that is invoked when text is found between tags.
  *
  * A State machine that populates the current newsflash member
  * and the defaults structure with string text found 
  * between start and end tags i.e <tag>String Text</tag>
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class










SEVEN NEWS XML data

	<ticker update="2004-04-23 15:15:00">
	<datafile update="2004-04-23 15:15:00">
<title>National News</title>
<file>http://ticker.seven.com.au/xml/national.xml</file>
</datafile>
	<datafile update="2004-04-23 15:15:00">
<title>International News</title>
<file>http://ticker.seven.com.au/xml/intl.xml</file>
</datafile>
	<datafile update="2004-04-23 15:15:00">
<title>Sport News</title>
<file>http://ticker.seven.com.au/xml/sport.xml</file>
</datafile>
</ticker>

============================================================
  */

void NewsXML_Handler::characters(const   XMLCh* const    chars,
								   const unsigned int    length)
{
	char* inText = XMLString::transcode(chars);
	CString inString = inText;
	
	switch (XML_CharState)
	{
		case NEWS_TEXT :
			if( currentNewsFlash.data.text.IsEmpty() )
				currentNewsFlash.data.text = inText;
			else
				currentNewsFlash.data.text += inText;
			currentNewsFlash.data.text.Replace( "&quot;", "\"" );
			currentNewsFlash.data.text.Replace( "&amp;", "&" );
			currentNewsFlash.data.text.Replace( "&#039;", "'" );
			currentNewsFlash.data.text.Replace( "&#036;", "$" );
			currentNewsFlash.data.text.Replace( "\n", "\t" );

			// remove any tags from the string
			int pos;
			while( (pos=currentNewsFlash.data.text.Find("<")) >= 0 )
			{
				int len = currentNewsFlash.data.text.Find( ">", pos );
				if( len>=0 )
				{
					currentNewsFlash.data.text.Delete( pos, (len-pos)+1 );
				}
			}
			//OutDebugs("Loaded News '%s'", currentNewsFlash.data.text.GetBuffer(0) );
			break;

		case NEWS_DATE :
			//Debug("Got the Date");
			ExtractDate(&inString,
                        &currentNewsFlash.data.nYear,
                        &currentNewsFlash.data.nMonth,
                        &currentNewsFlash.data.nDay);
			break;

		case NEWS_TIME :
			//Debug("Got the Time");
			ExtractTime(&inString,
                        &currentNewsFlash.data.nHour,
                        &currentNewsFlash.data.nMin,
                        &currentNewsFlash.data.nSec);
			break;

		case NEWS_INVALID :
			break;

		default :
			Debug("XML Character record State is NOT VALID. The defualt");
			Debug("value for this state machine should never be reached");
			break;
	}
	inString.Empty();
}

/**
  * Event handler that is invoked when a start tag is found
  *
  * A State machine that populates the current newsflash member
  * and the defaults structure when an open tag is found 
  * and also controls the state of the charactes stage machibe
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class

<!DOCTYPE newsflashes>
<newsflashes>
  <newsflash>
    <text>Premier Steve Bracks is under pressure to reject an automatic pay rise for Victorian MPs</text>
    <archive_date>2004-11-22</archive_date>
    <archive_time>18:31:00</archive_time>
  </newsflash>
</newsflashes>




<tickerdata name="National News">
	<separator>
		<flashimage>7logo</flashimage>
	</separator>
		<item href="/news/business" target="">
		</item>
	<separator>
		<flashimage>7logo</flashimage>
	</separator>
		<item href="http://seven.com.au/news/topstories/76181">
			<string>Australia offers help to N Korea</string>
		</item>
	<separator>
		<flashimage>7logo</flashimage>
	</separator>
		<item href="http://seven.com.au/news/topstories/76188">
			<string>Jobless rates fall</string>
		</item>
*/

void NewsXML_Handler::startElement(const   XMLCh* const    uri,
                            const   XMLCh* const    localname,
                            const   XMLCh* const    qname,
                            const   Attributes&     attrs)
{
    char* message = XMLString::transcode(qname);
	if( !message ) return;
	CString messageString = message;
	XMLString::release(&message);
	
	switch (XML_State)
	{
		case FINDING_NEWSFLASH :
			if (messageString.Compare("newsflash") == 0)
			{	
				XML_State = FOUND_NEWSFLASH;
			} else
			// handle SEVEN format...
			if (messageString.Compare("item") == 0)
			{	
				XML_State = FOUND_NEWSFLASH;
			}		
			break;
	

		case FOUND_NEWSFLASH :
			if (messageString.Compare("text") == 0)
			{
				XML_CharState = NEWS_TEXT;
				gotText = true;
			}
			// handle SEVEN format...
			if (messageString.Compare("string") == 0)
			{
				XML_CharState = NEWS_TEXT;
				gotText = true;
			}
			// handle YAHOO NEWS format...
			if (messageString.Compare("description") == 0)
			{
				XML_CharState = NEWS_TEXT;
				gotText = true;
			}
			if (messageString.Compare("archive_date") == 0)
			{
				XML_CharState = NEWS_DATE;
				gotDate = true;
			}

			if (messageString.Compare("archive_time") == 0)
			{
				XML_CharState = NEWS_TIME;
				gotTime = true;
			}
			break;

		default :
			Debug("The stage machine for 'startElement' of newsflash XML");
			Debug("parser is broken it is in default state and it should");
			Debug("never have gotten here");
			break;
	}
	messageString.Empty();
}

/**
  * Event handler that is invoked when an end tag is found
  *
  * A State machine that controls the state of the charactes state machine
  *
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class
  */
void NewsXML_Handler::endElement(const XMLCh* const uri,
								   const XMLCh* const localname,
								   const XMLCh* const qname)
{
	char* message = XMLString::transcode(qname);
	CString messageString = message;
	XMLString::release(&message);

	/* Make sure we are not processing the character input*/
	XML_CharState = NEWS_INVALID;
	
	switch (XML_State)
	{
		case FOUND_NEWSFLASH :
			if (messageString.Compare("newsflash") == 0)
			{	
				if ((gotText == true) && (gotDate == true) && (gotText == true))
				{
					OutDebugs( "Adding news - %s", currentNewsFlash.data.text.GetBuffer(0) );
					m_NewsCaptions->AddTail(currentNewsFlash.data);
					currentNewsFlash.data.text.Empty();
					//currentNewsFlash = m_NewsCaptions->GetTail();
				}
				else
				{
					/* didn't find all the relevant data  log the error and purge the data */
					Debug("		INCOMPLETE newsflash found");
				}

				gotText = false;
				gotDate = false;
				gotTime = false;

				XML_State = FINDING_NEWSFLASH;
			} else
			if (messageString.Compare("item") == 0 ||			// handle SEVEN format...
				messageString.Compare("description") == 0 )		// handle YAHOO format
			{	
				if ((gotText == true) )
				{
					CTime currentTime = CTime::GetCurrentTime();		// get current time + 4hrs expiry

					// each new item can live for 24hrs
					currentTime += (60*60*24);

					currentNewsFlash.data.nDay = currentTime.GetDay();
					currentNewsFlash.data.nHour = currentTime.GetHour();
					currentNewsFlash.data.nMin = currentTime.GetMinute();
					currentNewsFlash.data.nMonth = currentTime.GetMonth();
					currentNewsFlash.data.nSec = currentTime.GetSecond();
					currentNewsFlash.data.nYear= currentTime.GetYear();
					OutDebugs( "Adding news - %s", currentNewsFlash.data.text.GetBuffer(0) );
					m_NewsCaptions->AddTail(currentNewsFlash.data);

					currentNewsFlash.data.text.Empty();
				}
				XML_State = FINDING_NEWSFLASH;
			}
			break;

		case FINDING_NEWSFLASH :
			gotText = false;
			gotDate = false;
			gotTime = false;
			break;

		default :
			Debug("The stage machine for 'endElement' of newsflash XML");
			Debug("parser is broken it is in default state and it should");
			Debug("never have gotten here");
			break;
	}
	messageString.Empty();
}


/** 
 *  If the XML file is misformed then this event handler gets called
 *
 *  Just does debug for now
 *
 *  TODO: Check what happens if Validation is switched off and there
 *        is an error in the XML.
 *
 */
void NewsXML_Handler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    
    CString errorMessage;
    long i = exception.getLineNumber();
    errorMessage.Format("Fatal Error:  %s at line: %i", message, i);
    Debug(errorMessage);
}

/**
 *  Writes a debug string to a file or an Edit box
 *
 *  For speed purposes this code is #define out.
 *  
 *  Uncomment the appropriate #define section when debug
 *  is required
 */
void NewsXML_Handler::Debug(CString debugMessage)
{
#define DEBUG_TO_EDIT 
#ifdef DEBUG_TO_EDIT
    if (m_OutputEdit->m_hWnd != 0)
    {
        CString tempString;
	    this->m_OutputEdit->GetWindowText(tempString);
	    this->m_OutputEdit->SetWindowText(tempString + debugMessage + "\r\n");
    }
#endif


    CStdioFile* errorFile = new CStdioFile("C:\\debug.txt", CFile::modeCreate 
                                                          | CFile::modeNoTruncate
                                                          | CFile::modeWrite );
    LPCTSTR errorMessage = (LPCTSTR)debugMessage;
    errorFile->Seek(0, CFile::end);
    errorFile->WriteString(errorMessage);
    errorFile->WriteString("\n");
    errorFile->Close();

	delete errorFile;
}
