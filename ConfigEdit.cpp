// ConfigEdit.cpp : implementation file
//

#include "stdafx.h"
#include "utils.h"
#include "history.h"
#include "Widgie.h"
#include "WidgieDlg.h"
#include "WidgieXML.h"
#include "LoopSeqXML_Handler.h"
#include "ConfigEdit.h"

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

// CConfigEdit dialog

IMPLEMENT_DYNAMIC(CConfigEdit, CDialog)
CConfigEdit::CConfigEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigEdit::IDD, pParent)
	, m_urlStr(_T(""))
	, m_rssStr(_T(""))
	, m_defaultLenInt(0)
	, m_blendFadeB(FALSE)
{
}

CConfigEdit::~CConfigEdit()
{
}

void CConfigEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEFAULTLENGTH, m_defLength);
	DDX_Control(pDX, IDC_URLS, m_urls);
	DDX_Control(pDX, IDC_RSSLIST, m_rss);
	DDX_Control(pDX, IDC_RSSEDIT, m_rssText);
	DDX_Control(pDX, IDC_URLEDIT, m_urlText);
	DDX_Text(pDX, IDC_URLEDIT, m_urlStr);
	DDX_Text(pDX, IDC_RSSEDIT, m_rssStr);
	DDX_Text(pDX, IDC_DEFAULTLENGTH, m_defaultLenInt);
	DDX_Check(pDX, IDC_BLENDFADE, m_blendFadeB);
}


BEGIN_MESSAGE_MAP(CConfigEdit, CDialog)
	ON_BN_CLICKED(IDC_ADDURL, OnBnClickedAddurl)
	ON_LBN_SELCHANGE(IDC_RSSLIST, OnLbnSelchangeRsslist)
	ON_BN_CLICKED(IDC_ADDRSS, OnBnClickedAddrss)
	ON_LBN_SELCHANGE(IDC_URLS, OnLbnSelchangeUrls)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

static 	ImpList impList;

BOOL CConfigEdit::OnInitDialog()
{
    CDialog::OnInitDialog();

	m_defaultLenInt = CFG->cfgDefaultPlayLength;
	m_blendFadeB = CFG->cfgImageBlend;

	ParseLoop( &impList );

	int index = 0;
	POSITION newPos;
	// go through all impressions, add them to the gui list
	while( newPos = impList.FindIndex(index++) )
	{
	    ImpressionData nextImpression = impList.GetAt(newPos);
		if( nextImpression.m_image_file.GetLength()>0 )
			m_urls.AddString( nextImpression.m_image_file.GetBuffer(0) );
		else
		if( nextImpression.m_image_url.GetLength()>0 )
			m_urls.AddString( nextImpression.m_image_url.GetBuffer(0) );
	}

	// add rss links
	for( int i=0; i< CFG->cfgNewsRSSFeeds.GetCount();i++)
	{
		CString rssurl = CFG->cfgNewsRSSFeeds.GetAt( CFG->cfgNewsRSSFeeds.FindIndex(i) );
		m_rss.AddString( rssurl.GetBuffer(0) );
	}

	return TRUE;
}

// =====================     CConfigEdit message handlers     ==================
void CConfigEdit::OnBnClickedAddurl()
{
	m_urls.AddString( m_urlStr.GetBuffer(0) );
}

void CConfigEdit::OnLbnSelchangeUrls()
{
	int item = m_urls.GetCurSel();

	int n = m_urls.GetTextLen( item );
	m_urls.GetText( item, m_urlStr.GetBuffer(n) );
	m_urlStr.ReleaseBuffer();
}

void CConfigEdit::OnBnClickedAddrss()
{
	m_rss.AddString( m_rssStr.GetBuffer(0) );
}

void CConfigEdit::OnLbnSelchangeRsslist()
{
	int item = m_rss.GetCurSel();

	int n = m_rss.GetTextLen( item );
	m_rss.GetText( item, m_rssStr.GetBuffer(n) );
	m_rssStr.ReleaseBuffer();
}

void CConfigEdit::OnBnClickedOk()
{
	CFG->cfgDefaultPlayLength = m_defaultLenInt;
	CString num;
	num.Format( "%d", m_defaultLenInt );
	WritePrivateProfileString("GRAPHICS", "DefaultPlayLen", num, INI_FILE);

	num.Format( "%d", m_blendFadeB );
	WritePrivateProfileString("GRAPHICS", "ImageBlend", num, INI_FILE);

	// populate RSS links...
	CFG->cfgNewsRSSFeeds.RemoveAll();
	for( int i=0; i<m_rss.GetCount(); i++ )
	{
		CString itemStr, itemName;

		int n = m_rss.GetTextLen( i );
		m_rss.GetText( i, itemStr.GetBuffer(n) );
		itemStr.ReleaseBuffer();

		CFG->cfgNewsRSSFeeds.AddTail( itemStr );

		itemName.Format( "Item_%d", i );
		WritePrivateProfileString("RSSNEWS", itemName, itemStr, INI_FILE);
	}

	OnOK();
}




bool CConfigEdit::ParseLoop( ImpList *impList )
{
	try {
		 XMLPlatformUtils::Initialize();
	}
    catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
        
        OutDebugs("Error during initialization! :\n");
        OutDebugs("Exception message is: \n");
           
		CString messageString;
		message = messageString.GetBuffer(100);
		OutDebugs(messageString + "\n");

        Log_App_FileError(errXML, "The Parser!", "Error during initialization! : " + messageString);

		messageString.ReleaseBuffer();
        XMLString::release(&message);
	}
     
    SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
    parser->setFeature(XMLUni::fgSAX2CoreValidation, true);   // optional
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);   // optional
    parser->setFeature(XMLUni::fgXercesContinueAfterFatalError, true);

	LoopSeqXML_Handler* defaultHandler = NULL;

	if( defaultHandler == NULL )
		defaultHandler = new LoopSeqXML_Handler(&m_OutputEdit, impList, NULL, "" );
	else
		defaultHandler->Reset_Handler(&m_OutputEdit, impList, NULL, "" );

    parser->setContentHandler(defaultHandler);
    parser->setErrorHandler(defaultHandler);

	try 
	{
		// first we must try "slideshow_Display_####.xml", if that fails open "slideshow.xml"
		CString defaultLoopFN = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName;
		CString currentLoopFN = defaultLoopFN;
		currentLoopFN.Replace( ".xml", "_" );
		currentLoopFN.AppendFormat( "%s.xml", CFG->cfgID );

		// now if we have files, process them...
		if( FileExists(currentLoopFN) )
		{
			OutDebugs( "Parsing Loop XML file - %s",  currentLoopFN.GetBuffer(0) );
			parser->parse( currentLoopFN.GetBuffer(0) );
		} else
		if( FileExists(defaultLoopFN) )
		{
			OutDebugs( "Parsing Loop XML file - %s",  defaultLoopFN.GetBuffer(0) );
			parser->parse( defaultLoopFN.GetBuffer(0) );
		}

		BOOL currentLoop = defaultHandler->IsCurrent();

		delete parser;
		delete defaultHandler;
		defaultHandler = NULL;
		return (bool)currentLoop;
    }
    catch (const XMLException& toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        OutDebugs("Exception message is: ");
		CString* messageString = new CString(message, strlen(message));
        OutDebugs (*messageString);
        Log_App_FileError(errXML, "XML Exception", *messageString);
		XMLString::release(&message);
    }

	catch (const SAXParseException& toCatch) {
        char* message = XMLString::transcode(toCatch.getMessage());
        OutDebugs("Exception message is: ");
		CString* messageString = new CString(message, strlen(message));
		OutDebugs (*messageString);
        
        Log_App_FileError(errXML, "XML Parse Exception", *messageString);
		XMLString::release(&message);
    }

	delete parser;
	delete defaultHandler;
	defaultHandler = NULL;

	return true;
}
