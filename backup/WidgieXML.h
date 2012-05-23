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


	CString currentDownloadFile;
	long	m_currentDownloaded;


	BOOL IsServerAlive( void );				// check to see if server is contactable.
	long DownloadXMLAll();                  // See C++ implementation
	long DownloadXMLNews();                 // See C++ implementation
	long DownloadXMLLoop();                 // See C++ implementation
	long DownloadXMLManifest();             // See C++ implementation
	long DownloadXMLConfig();               // See C++ implementation
    long DownloadContent();                 // See C++ implementation
	long DownloadMissingContent();
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

protected:
	HICON m_hIcon;


private:
    CEdit m_OutputEdit;

    
    long DownloadText(CString fileToGet,    // See C++ implementation
                      CString fileToWrite,
					  CString uid = NULL );	
    
    long DownloadBin(CString fileToGet,     // See C++ implementation
                     CString fileToWrite,
					 CString uid = NULL );

    long DownloadManifestBin(CString fileToGet,     // See C++ implementation
                     CString fileToWrite,
					 CString uid = NULL );

	void DeleteFiles();                     // See C++ implementation
    void DeleteDir();                       // See C++ implementation
    
	
public:
	CImage *GetImpressionImage(int pos);
	int FreeImpressionImage(int pos);
	int DeleteUnusedContent();
	int UploadErrorsLog( void );
};

#endif // !defined(AFX_WidgieXMLDLG_H__E5735058_D943_45CE_ACD9_D7DC23C54C0D__INCLUDED_)
