// Widgie XMLDlg.h : header file
//

#if !defined(AFX_WidgieXMLDLG_H__E5735058_D943_45CE_ACD9_D7DC23C54C0D__INCLUDED_)
#define AFX_WidgieXMLDLG_H__E5735058_D943_45CE_ACD9_D7DC23C54C0D__INCLUDED_

#include <afxtempl.h>
#include <afxcoll.h>

#include "NewsXML_Handler.h"
#include "LoopSeqInfo.h"
#include "ManifestInfo.h"

#include <afxstr.h>
#include <atlimage.h> 

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CWidgieXMLDlg dialog

#define BUFFER_LENGTH       256



class ImpressionList : public CList<ImpressionData,ImpressionData>
{
	ImpressionData FindImpIndex(int pos) {
		POSITION p = FindIndex(pos); 
		return GetAt(p); 
	};
	ImpressionData FindImpId(int id) {
		int pos = 0;
		POSITION p;
		while( p=FindIndex(pos++) )
		{
			ImpressionData idata = GetAt(p);
			if( idata.m_id == id )
				return idata;
		}
	};
};
	
typedef	CList<ImpressionData,ImpressionData>	ImpList;




#define		MAX_LANGUAGES		6

class CInternetSession;

class CWidgieXML
{

public:

	CWidgieXML(CString language, CEdit* debugEdit = NULL);	// standard constructor
	
	CList <NewsflashData, NewsflashData> newsflashList;
	CList <ManifestData, ManifestData> fileList;
    CList <CString, CString> languageList;

	// ----------------------- Impression Lists and language handeling... ---------------------
//	ImpList impressionList;			// original list for old code.
	ImpList *impressionListP;		// current pointer
	ImpList impressionLists[MAX_LANGUAGES];		// EN,Ch,JP

	// return impression list for current language
	ImpList *GetImpressionList( int lang = -1 ) {
		if( lang >= 0 && lang< languageList.GetCount() )
			return &impressionLists[lang];
		else
			return impressionListP;
	}
	// set the currently active list to the language one we specify
	void SetImpressionList( int lang ) {
		if( lang >= 0 && lang< languageList.GetCount() )
			impressionListP = GetImpressionList( lang );
		else
			impressionListP = &impressionLists[0];		// special case to use the original default one
	}
	// flush all the language lists, byee..
	void FreeImpressionLists( void ) {
		for( int lp=0; lp<languageList.GetCount(); lp++)
		{
			ImpList *iList = GetImpressionList( lp );
			iList->RemoveAll();
		}
	}
	CString StrLanguage;
	int		IntLanguage;

	// -----------------------------------

	LoopDefaults impressionDefaults;

	CInternetSession* myInternetSession;

	// MDID Support vars
	CString	m_sessionToken;					// used for form access login details (cookie)
	CTime	m_sessionStartTime;				// the time when we got the token
	CString m_slideshowElement;
	int		m_slideshowID;					// based on our login, we then have this slideID to use
	CString m_slideshowDate;				// last slideshow moddified date.
	CList<CString,CString> m_slideshowList;	// a complete list of all the slideshow ID names available.
	// -------------------------------------

	CString	m_configDateStamp;
	CString currentDownloadFile;
	long	m_currentDownloaded;

	long	m_downloadNewsCount;
	long	m_downloadConfigCount;
	long	m_downloadSlideshowCount;
	long	m_downloadLoginCount;
	long	m_downloadMediaCount;


	BOOL IsServerAlive( void );				// check to see if server is contactable.
	long DownloadXMLAll();                  // See C++ implementation
	long DownloadXMLNews();                 // See C++ implementation
	long DownloadXMLLoop();                 // See C++ implementation
	long DownloadXMLManifest();             // See C++ implementation
	long DownloadXMLConfig();               // See C++ implementation
    long DownloadContent();                 // See C++ implementation
	long DownloadMissingContent( long *filesTotal, long *filesDone );
	long DownloadSupportImages( CString option );
	long DownloadNewExecuteable();
    void Debug(CString debugMessage);       // See C++ implementation

    void ParseAll();                        // See C++ implementation
	void ParseManifest();
    void ParseNews();                       // See C++ implementation
	bool ParseLoop( ImpList *impList = NULL, CString languageToUse = NULL );		// See C++ implementation	void ParseManifest();                   // See C++ implementation
    bool ParseLoops();                      // See C++ implementation
	bool ParseConfigDefaults( void );

	void CloseDown();	                    // See C++ implementation
	BOOL IsShuttingDown();

	bool ImportTelenticeMPL( void );

protected:
	HICON m_hIcon;


private:
    CEdit m_OutputEdit;
	BOOL	m_shuttingDown;

	// MDID SUPPORT
	CString GetElement( CString p_xml, CString p_elementName );
	CString GetElementParam( CString p_xml, CString p_paramName );

	long MDID_Login( CString server, CString remoteFile, CString parameters  );
	long MDID_DownloadSlideShow( CString server, CString fileToWrite );

	long MDID_ProcessSlideShow( CString slideshowXML );
	long MDID_ProcessLogin( CString loginXML );
	//----------------------------------------------------

	long DownloadText(	CString server,
						CString fileToGet,    // See C++ implementation
						CString fileToWrite,
						CString parameters = NULL,
						CString *lastDownloadDate = NULL);	

    long DownloadBin(	CString server,
						CString fileToGet,				// file to retrieve
					    CString fileToWrite,			// file to write to
						CString parameters = NULL,		// cgi params
						CString *lastDownloadDate = NULL );	// last download date to check against

    long DownloadManifestBin( CString fileToGet,     // See C++ implementation
						CString fileToWrite,
						CString uid = NULL );

	long ServerPostAction( CString server, CString remoteFile, CString postArgs );

	void DeleteFiles();                     // See C++ implementation
    void DeleteDir();                       // See C++ implementation
    
	
public:
	long DownloadFile( CString server, CString url );
	CImage *GetImpressionImage(int pos);
	int FreeImpressionImage(int pos);
	int DeleteUnusedContent();
	int UploadErrorsLog( void );
	// Upload event log via a POST using http
	int PostEventLog(CList<CString,CString> *events);
};

#endif // !defined(AFX_WidgieXMLDLG_H__E5735058_D943_45CE_ACD9_D7DC23C54C0D__INCLUDED_)
