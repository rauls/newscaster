// ManifestXML_Handler.cpp: implementation of the ManifestXML_Handler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ManifestXML_Handler.h"
#include "utils.h"

#include <xercesc/sax2/Attributes.hpp>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/**
  * Constructor for Manifest Handler.
  *
  * Initialised the data mambers that will be used to collect
  * manifest information.
  *
  * Initialises the state machines that are used by the event
  * handlers that retrieve XML data.
  *
  * @param debugEdit        a pointer to an edit box used for debug
  *                         this can be NULL but only if #define DEBUG_TO_EDIT is
  *                         commented out in teh Debug function for this class
  *
  * @param impressionList   a pointer to a list to place all manifest data that
  *                         is found during parsing
  *
  */

ManifestXML_Handler::ManifestXML_Handler(CEdit* debugEdit,
										 CList <ManifestData, ManifestData>* ManifestList)
{
	m_OutputEdit = debugEdit;
	pManifestList = ManifestList;

	ManifestXML_State = FINDING_FILE;
	XML_CharState = MANIFEST_INVALID;

	InitCurrentManifest();
}

void ManifestXML_Handler::Reset_Handler(CEdit* debugEdit,
										 CList <ManifestData, ManifestData>* ManifestList)
{
	m_OutputEdit = debugEdit;
	pManifestList = ManifestList;

	ManifestXML_State = FINDING_FILE;
	XML_CharState = MANIFEST_INVALID;

	InitCurrentManifest();
}

ManifestXML_Handler::~ManifestXML_Handler()
{
	OutDebugs( "ManifestXML_Handler::~ManifestXML_Handler()" );
}

/**
  * Event handler that is invoked when text is found between tags.
  *
  * A State machine that populates the current manifest member
  * with string text found between start and end tags
  * i.e <tag>String Text</tag>
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class
  */

void ManifestXML_Handler::characters(const   XMLCh* const    chars,
								    const unsigned int    length)
{
	char* inText = XMLString::transcode(chars);
	
	switch (XML_CharState)
	{
		case MANIFEST_IMAGE :
			currentManifest.manifImageFile = inText;
			Debug("Got the IMAGE");
			break;

		case MANIFEST_MULTIMEDIA :
			currentManifest.manifMultimediaFile = inText;
			Debug("Got the MULTIMEDIA");
			break;
	
		case MANIFEST_PRINT :
			currentManifest.manifPrintFile = inText;
			Debug("Got the PRINT");
			break;
	}
}

/**
  * Event handler that is invoked when an start tag is found
  *
  * A State machine that populates the current manifest menber
  * when an open tag is found and also controls the state of
  * the charactes stage machine
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class
  */

void ManifestXML_Handler::startElement(const   XMLCh* const    uri,
									    const   XMLCh* const    localname,
										const   XMLCh* const    qname,
										const   Attributes&     attrs)
{
	char* element = XMLString::transcode(qname);
	CString elementString = element;
	XMLString::release(&element);
	
    /* variables used to extact attibute strings */
    unsigned int len = attrs.getLength();
    char* QName;
	char* Value;

	Debug(elementString);

	switch (ManifestXML_State)
	{
		case FINDING_FILE :
			if (elementString.Compare("file") == 0)
			{	
				ManifestXML_State = FOUND_FILE;
				Debug("FOUND file");
                
                for (unsigned int attrCount = 0; attrCount < len; attrCount++)
	            {
		            QName = XMLString::transcode(attrs.getQName((short) attrCount));
					CString QNameString = QName;
		            XMLString::release(&QName);
	            
		            Value = XMLString::transcode(attrs.getValue((short) attrCount));

                    if (QNameString.Compare("method") == 0)
				    {
					    currentManifest.manifMethod = Value;
					    gotFileMethod = true;
				    }
                    				
				    if (QNameString.Compare("date_required") == 0)
				    {
					    currentManifest.manifDateRequired = Value;
					    gotDateRequired = true;
				    }
					QNameString.Empty();
	            }
			}
		break;

		case FOUND_FILE :
			if (elementString.Compare("image_file") == 0)
			{
				Debug("FOUND image_file");
				XML_CharState = MANIFEST_IMAGE;
				gotImageFile = true;

                for (unsigned int attrCount = 0; attrCount < len; attrCount++)
	            {
		            QName = XMLString::transcode(attrs.getQName((short) attrCount));
		            CString QNameString = QName;
		            XMLString::release(&QName);
	            
		            Value = XMLString::transcode(attrs.getValue((short) attrCount));

                    if (QNameString.Compare("type") == 0)
				    {
					    currentManifest.manifImageFileType = Value;
					    gotImageFileType = true;
				    }  
					QNameString.Empty();
	            }
			}
			
			if (elementString.Compare("multimedia_file") == 0)
			{
				Debug("FOUND multimedia_file");
				XML_CharState = MANIFEST_MULTIMEDIA;
				gotMultimediaFile = true;

                for (unsigned int attrCount = 0; attrCount < len; attrCount++)
	            {
		            QName = XMLString::transcode(attrs.getQName((short) attrCount));
		            CString QNameString = QName;
		            XMLString::release(&QName);
	            
		            Value = XMLString::transcode(attrs.getValue((short) attrCount));

                    if (QNameString.Compare("type") == 0)
				    {
					    currentManifest.manifMultimediaFileType = Value;
					    gotMultimediaFileType = true;
				    }
					QNameString.Empty();
	            }
			}

			if (elementString.Compare("print_file") == 0)
			{
				Debug("FOUND print_file");
				XML_CharState = MANIFEST_PRINT;
				gotPrintFile = true;

                for (unsigned int attrCount = 0; attrCount < len; attrCount++)
	            {
		            QName = XMLString::transcode(attrs.getQName((short) attrCount));
		            CString QNameString = QName;
		            XMLString::release(&QName);
	            
		            Value = XMLString::transcode(attrs.getValue((short) attrCount));

                    if (QNameString.Compare("type") == 0)
				    {
					    currentManifest.manifPrintFileType = Value;
					    gotPrintFileType = true;
				    }
					QNameString.Empty();
	            }
			}
		break;
	}
	elementString.Empty();
}

/**
  * Event handler that is invoked when an end tag is found
  *
  * A State machine controls the state of the charactes stage machine
  *
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class
  */

void ManifestXML_Handler::endElement(const XMLCh* const uri,
										const XMLCh* const localname,
										const XMLCh* const qname)
{
	char* element = XMLString::transcode(qname);
	CString elementString = element;
	XMLString::release(&element);
	
	Debug(elementString);
	/* Make sure we are not processing the character input*/
	XML_CharState = MANIFEST_INVALID;
	
	switch (ManifestXML_State)
	{
		case FOUND_FILE :
			if (elementString.Compare("file") == 0)
			{	
				Debug("FOUND /file");

				if ( (gotImageFile == true)
					&& (gotMultimediaFile == true) 
					&& (gotPrintFile == true) )
				{
					Debug("		Found COMPLETE file manifest");
					
					pManifestList->AddTail(currentManifest);
				}
				else
				{
					/* didn't find all the relevant data log the error and purge the data */
					Debug("		INCOMPLETE file Manifest found");
				}
				InitCurrentManifest();

				ManifestXML_State = FINDING_FILE;
			}		
			break;

		case FINDING_FILE :
			InitCurrentManifest();
			break;
	}
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

void ManifestXML_Handler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    
    CString errorMessage;
    long i = exception.getLineNumber();
    errorMessage.Format("Fatal Error:  %s at line: %i", message, i);

    Debug(errorMessage);
}

/** 
 *  Initialises all the local variables of the Current manifest
 *  and the flags that signify if a certain element has been found
 *
 */
void ManifestXML_Handler::InitCurrentManifest()
{
	gotFileMethod = false;
	gotDateRequired = false;
	gotImageFile = false;
	gotImageFileType = false;				
	gotMultimediaFile = false;
	gotMultimediaFileType = false;
	gotPrintFile = false;
	gotPrintFileType = false;

	currentManifest.manifMethod = "";
	currentManifest.manifDateRequired = "";
	currentManifest.manifImageFile = "";
	currentManifest.manifImageFileType = "";
	currentManifest.manifMultimediaFile = "";
	currentManifest.manifMultimediaFileType = "";
	currentManifest.manifPrintFile = "";
	currentManifest.manifPrintFileType = "";
}


/**
 *  Writes a debug string to a file or an Edit box
 *
 *  For speed purposes this code is #define out.
 *  
 *  Uncomment the appropriate #define section when debug
 *  is required
 */
void ManifestXML_Handler::Debug(CString debugMessage)
{
//#define DEBUG_TO_EDIT 
#ifdef DEBUG_TO_EDIT

    if (m_OutputEdit->m_hWnd != 0)
    {
	    CString tempString;
	    this->m_OutputEdit->GetWindowText(tempString);
	    this->m_OutputEdit->SetWindowText(tempString + debugMessage + "\r\n");
    }

#endif

//#define DEBUG_TO_FILE
#ifdef DUBUG_TO_FILE

    CStdioFile* errorFile = new CStdioFile("C:\\debug.txt", CFile::modeCreate 
                                                          | CFile::modeNoTruncate
                                                          | CFile::modeWrite );
    LPCTSTR errorMessage = (LPCTSTR)debugMessage;
    errorFile->Seek(0, CFile::end);
    errorFile->WriteString(errorMessage);
    errorFile->WriteString("\n");
    errorFile->Close();
#endif
}
