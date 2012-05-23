// Widgie XMLDlg.cpp : implementation file
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/stat.h>


#include "resource.h"
#include "version.h"

#include "NewsXML_Handler.h"
#include "LoopSeqXML_Handler.h"
#include "ManifestXML_Handler.h"

#include "NewsflashInfo.h"
#include "LoopSeqInfo.h"
#include "ManifestInfo.h"
#include "history.h"
#include "utils.h"
#include "winnet_io.h"

#include <wininet.h>

#include <afxtempl.h>
#include <mmsystem.h>
      

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <ctype.h>
#include <string> 

#include "afxinet.h"
#include "Widgie.h"
#include "WidgieDlg.h"
#include "WidgieXML.h"
#include "history.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWidgieXML

int AreWeOnline( void )
{
	if ( InternetAttemptConnect( 0 ) == ERROR_SUCCESS )
		return 1;
	else
		return 0;

}

BOOL myDeleteFile( CString file )
{
	int tries = 5;
	BOOL ret;

	while(  (ret = DeleteFile( file.GetBuffer(0)) ) &&
			(GetLastError() != 0) &&
			(tries>0) )
	{
		tries--;
	}

	if( ret && GetLastError() != 0 )
	{
		char *errorString = (char*)GetLastErrorString();
		if( !strstr( "The operation completed successfully", errorString ) )
		{
			CString msg;
			msg.Format( "Failed to delete file - %d , %s", GetLastError() , errorString );
			Log_App_FileError( errMediaMissing, file, msg.GetBuffer(0) );
		}
	}

	return ret;
}




CWidgieXML::CWidgieXML( CString language, CEdit* debugEdit )
{
    StrLanguage = language;
	IntLanguage = 0;
    
	myInternetSession = NULL;

	// default to the single list
	SetImpressionList( 0 );
}




void CWidgieXML::CloseDown() 
{
	if( myInternetSession )
	{
		myInternetSession->Close();
		delete myInternetSession;
	}
}




/**
 *  Downloads all XML files
 *
 *  TODO: In the future this function will disappear
 *        when the interval downloading logic is implemented
 *        in the OnTimer event.
 */


long CWidgieXML::DownloadXMLAll() 
{
	bool result = true;
	/* check if the Content directory exists
	   if it doesn't create it */

	BOOL xmlDirCreateResult;

	CString lpPathName = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + "\0";
	
	xmlDirCreateResult = CreateDirectory(lpPathName , NULL);
	
	LPVOID lpMsgBuf;

	if (xmlDirCreateResult == FALSE)
	{				
		DWORD createError = GetLastError();

		if (createError != ERROR_ALREADY_EXISTS)
		{
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				createError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);

			OutDebugs( "Could not create directory: %s", lpPathName );
			LocalFree( lpMsgBuf );
			result = false;
		}
		else
		{
			OutDebugs("XML directory (%s) created successfully", lpPathName );
		}
	}
	
	long downloadsize = 0;
	DWORD t_beg = timeGetTime();

//	OutDebug( "Downloading XML news" );
//	downloadsize +=
//	DownloadXMLNews();

	OutDebug( "Downloading XML loop" );
	downloadsize +=
	DownloadXMLLoop();

	OutDebug( "Downloading XML manifest" );
	downloadsize +=
	DownloadXMLManifest();

	CString msg;
	msg.Format( "Downloaded All XMLData, %d bytes in %.1f seconds", downloadsize, (timeGetTime()-t_beg)/1000.0 );
	Log_App_Event( 0 , msg );

	return downloadsize;
}


long CWidgieXML::DownloadXMLNews() 
{
	DWORD t_beg = timeGetTime();
static time_t	last_good_download_t = 0;


	if( (time(0) - last_good_download_t) < (60*15) )
	{
		OutDebugs( "ERROR: Downloading news too fast... 15min wait" );
		return 0;
	}

	long downloadsize = 0;
	// download other foreign NEWS items
	if( CFG->cfgUseSevenTicker )
	{
		downloadsize +=
		DownloadText( DEFURL_NATIONALNEWS, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + XMLFN_NATIONALNEWS );
	}
	if( CFG->cfgUseYahooTicker )
	{
		downloadsize +=
		DownloadText( DEFURL_YAHOOTOPNEWS, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + XMLFN_YAHOOWORLDNEWS );
	}
	if( CFG->cfgUseBBCTicker )
	{
		downloadsize +=
		DownloadText( DEFURL_BBCWORLDNEWS, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + XMLFN_BBCWORLDNEWS );
	}

	if( CFG->cfgIPandPort.IsEmpty() == false )
	{
		// iTV news XML
		downloadsize +=
		DownloadText( CFG->cfgNewsFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalNewsName);
	}

	if( downloadsize >0 )
		last_good_download_t = time(0);


	t_beg = timeGetTime()-t_beg;

	CString msg;
	msg.Format( "Downloaded XMLNews, %d bytes in %.1f seconds", downloadsize, t_beg/1000.0 );
	Log_App_Event( 0 , msg );


	return downloadsize;
}


long CWidgieXML::DownloadXMLConfig()
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	// check that the config version in the loop sequence is newer than the one that was preloaded in the global settings.
	// Config XML
	downloadsize +=
	DownloadText( CFG->cfgConfigFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + DEFURL_CONFIG );

	CString msg;
	msg.Format( "Downloaded Config XML, %d bytes in %.1f seconds", downloadsize, t_beg/1000.0 );
	Log_App_Event( 0 , msg );

	return downloadsize;
}


long CWidgieXML::DownloadXMLLoop()
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;
	// LOOP sequence
	downloadsize +=
	DownloadText( CFG->cfgLoopsequenceFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName);

	t_beg = timeGetTime()-t_beg;

	CString msg;
	msg.Format( "Downloaded XML Loop Sequence, %d bytes in %.1f seconds", downloadsize, t_beg/1000.0 );
	Log_App_Event( 0 , msg );

	return downloadsize;
}



long CWidgieXML::DownloadXMLManifest()
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;


//	CString xmlfile = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalManifestName;
//	CString lastfile = xmlfile + ".last";
//	myDeleteFile( lastfile );
//	MoveFile( xmlfile.GetBuffer(0), lastfile.GetBuffer(0) );

	// MANIFEST data
	downloadsize +=
	DownloadText( CFG->cfgManifestFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalManifestName);

	t_beg = timeGetTime()-t_beg;

	CString msg;
	msg.Format( "Downloaded XML Manifest, %d bytes in %.1f seconds", downloadsize, t_beg/1000.0 );
	Log_App_Event( 0 , msg );

	return downloadsize;
}




















/**
 *  Parse all of the XML files
 *
 *  This function is not Used by the MCCA at all
 *  it is an event handler for a button click that 
 *  was developed in conjunction with a GUI for testing
 *  XML parsing
 * 

We have multiple playlists in memory at once for each language
When we first start, we load in only one, the current or preset languag type.
From then on we load each new language as it comes...


 */
void CWidgieXML::ParseAll() 
{
	// parse current news xml file
	ParseNews();

	// parse all language's loop sequences...
	ParseLoops();

	ParseConfigDefaults();

	ParseManifest();
}






#include <afxtempl.h>

int GetDirectoryList( char *szTempFile, CList <CString,CString> &fileList )
{
	WIN32_FIND_DATA fd; 
	HANDLE hFind; 
	int nNext = 0;

	if ( szTempFile )
	{
		int 	fFound=FALSE;

		hFind = FindFirstFile(szTempFile,&fd); 
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;

		while ( fFound )  
		{
			if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				fileList.AddTail( fd.cFileName );
			}
			fFound = FindNextFile( (HANDLE)hFind, &fd );
		}
		FindClose(hFind);
		return nNext;
	}
	return nNext;  
} //





// go thru the directory list, and compare against manifest and delete all those manifest type files that we dont use anymore
int CWidgieXML::DeleteUnusedContent() 
{
	ManifestData mi;
		
	DWORD t_beg = timeGetTime();

	// Do dir list to CString list.
	CList<CString,CString> dirList;

	CString dirPath = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "\\*";

	GetDirectoryList( dirPath.GetBuffer(0), dirList );

	long downloadsize = 0;

	// loop all manifest entries
	for( POSITION manifpos = fileList.GetHeadPosition(); manifpos != NULL; )
	{
		mi = fileList.GetNext(manifpos);

		POSITION dirPos;

		//fileName = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + mi.manifImageFile);
		if( dirPos = dirList.Find( mi.manifImageFile ) )
		{
			dirList.RemoveAt( dirPos );
			// now remove the bmp from the dir list
			mi.manifImageFile.Replace( ".jpg", ".bmp" );
			if( dirPos = dirList.Find( mi.manifImageFile ) )
				dirList.RemoveAt( dirPos );
		}
            
		if( dirPos = dirList.Find( mi.manifMultimediaFile ) )
			dirList.RemoveAt( dirPos );

		if( dirPos = dirList.Find( mi.manifPrintFile ) )
			dirList.RemoveAt( dirPos );

		//fileName = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + mi.manifMultimediaFile);

		//fileName = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + mi.manifPrintFile);
	}



	for( POSITION dirpos = dirList.GetHeadPosition(); dirpos != NULL; )
	{
		CString file = dirList.GetNext( dirpos );
		char *p = file.GetBuffer(0);

		if( file.Find( "_" )>0 && isdigit(*p) )
		{
			CString fullFN = "content\\" + file;
			myDeleteFile( fullFN );
		}

	}


	return downloadsize;
}








/**
 *  Downloads all content that is refered to in the
 *  manifest and loopsequence XML files
 *
 *  This function is not Used by the MCCA at all
 *  it is an event handler for a button click that 
 *  was developed in conjunction with a GUI for testing
 *  content doneload
 * 
 */

long CWidgieXML::DownloadContent() 
{
	// TODO: Add your control notification handler code here
    
    /* TO DO MAKE SURE THAT content directory exists */
    BOOL contentDirCreateResult;

	CString lpPathName = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "\0";
	
	contentDirCreateResult = CreateDirectory(lpPathName , NULL);
	
	LPVOID lpMsgBuf;

	if (contentDirCreateResult == FALSE)
	{				
		DWORD createError = GetLastError();

		if (createError != ERROR_ALREADY_EXISTS)
		{
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				createError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);

			OutDebugs("Could not create content directory: %s" , lpPathName);

			LocalFree( lpMsgBuf );
		}
		else
		{
			OutDebugs("Content directory (%s) created successfully", lpPathName );
		}
	}

	ManifestData mi;
		
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	if( CFG->cfgIPandPort.IsEmpty() == false )
	{
		// only download files which do not exist, as we never ovewrite media files, but only use new names....
		for( POSITION manifpos = fileList.GetHeadPosition(); manifpos != NULL; )
		{
			mi = fileList.GetNext(manifpos);

			downloadsize +=
			DownloadManifestBin(CFG->cfgServerContentPath + mi.manifImageFile,
						CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + mi.manifImageFile);
	            
			downloadsize +=
			DownloadManifestBin(CFG->cfgServerContentPath + mi.manifMultimediaFile,
						CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + mi.manifMultimediaFile);

			downloadsize +=
			DownloadManifestBin(CFG->cfgServerContentPath + mi.manifPrintFile,
						CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + mi.manifPrintFile);
		}
	}

	CString msg;
	msg.Format( "Downloaded MediaFiles, %d bytes in %.1f seconds", downloadsize, (timeGetTime()-t_beg)/1000.0 );
	Log_App_Event( 0 , msg );


	return downloadsize;
}




long CWidgieXML::DownloadMissingContent() 
{
	long downloadsize=0;
	// TODO: Add your control notification handler code here
    
    /* TO DO MAKE SURE THAT content directory exists */
    BOOL contentDirCreateResult;

	CString lpPathName = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "\0";
	
	contentDirCreateResult = CreateDirectory(lpPathName , NULL);
	
	LPVOID lpMsgBuf;

	if (contentDirCreateResult == FALSE)
	{				
		DWORD createError = GetLastError();

		if (createError != ERROR_ALREADY_EXISTS)
		{
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				createError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);

			OutDebugs("Could not create content directory: %s" , lpPathName);

			LocalFree( lpMsgBuf );
		}
		else
		{
			OutDebugs("Content directory (%s) created successfully", lpPathName );
		}
	}

	PrintDebugText( "Downloading missing images in loop sequence..." );

	int imageCount = 0;
	POSITION newPos;
	ImpList *impList = GetImpressionList();
	// go through all impressions, and check to see if any files are missing, if so , then download them
	while( newPos = impList->FindIndex(imageCount++) )
	{
	    ImpressionData nextImpression = impList->GetAt(newPos);

		CString downloadFiles[4];

		downloadFiles[0] = nextImpression.m_image_file;
		downloadFiles[1] = nextImpression.m_multimedia_file;
		downloadFiles[2] = nextImpression.m_print_file;

		// loop thru the 3 different file types to download them if they are missing.
		for( int i=0 ; i<3; i++ )
		{
			struct stat sbuf;

			if( downloadFiles[i].Find( "http://" ) == 0 )
			{
				CString localFilename;
				localFilename.Format( "%s%08lx.jpg", CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir, CHashIt(downloadFiles[i]) );

				bool noFile = stat( localFilename.GetBuffer(0), &sbuf );
				// download the remote http data file...
				{
					OutDebugs( "Downloading file - %s", downloadFiles[i] );

					if( downloadsize+=DownloadBin(downloadFiles[i], localFilename.GetBuffer(0) ) == 0 )
					{
						PrintDebugText( "\n\nFAILED - file does not exist on server" );
						Sleep( 500 );
					}
				}
			} else
			if( CFG->cfgIPandPort.IsEmpty() == false )
			{
				CString localFilename = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + downloadFiles[i];
				// if the xml file doesnt exist, download it...
				if( stat( localFilename.GetBuffer(0), &sbuf ) )
				{
					CString txt = "\nDownloading file - ";
					txt += downloadFiles[i];
					PrintDebugText( txt.GetBuffer(0) );

					if( downloadsize+=DownloadBin(CFG->cfgServerContentPath + downloadFiles[i], localFilename.GetBuffer(0) ) == 0 )
					{
						PrintDebugText( "\n\nFAILED - file does not exist on server" );
						Sleep( 500 );
					}
				}
			}
		}
	}
	return downloadsize;
}













/**
 *  Downloads all content that is refered to in the
 *  config xml file for logos/images and stuff
 * 
 * 
 * 
 * 
 */

long CWidgieXML::DownloadSupportImages( CString type ) 
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	if( type.CompareNoCase( "itvlogo" )==0 )
	{
		// main logo
		if( CFG->cfgMainLogoFileName.IsEmpty() == FALSE ){
			downloadsize +=
			DownloadBin(CFG->cfgServerContentPath + CFG->cfgMainLogoFileName,
						CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + DEFAULT_LOGOFILE );
		}
	} else
	if( type.CompareNoCase( "newslogo" )==0 )
	{

		// news corner logo
		if( CFG->cfgNewsLogoFileName.IsEmpty() == FALSE ){
			downloadsize +=
			DownloadBin(CFG->cfgServerContentPath + CFG->cfgNewsLogoFileName,
						CFG->cfgLocalBaseDir + NEWSLOGO_FN );
		}
	} else
	if( type.CompareNoCase( "scrolllogo" )==0 )
	{
		// news scroll bar seperator
		if( CFG->cfgScrollLogoFileName.IsEmpty() == FALSE ){
			downloadsize +=
			DownloadBin(CFG->cfgServerContentPath + CFG->cfgScrollLogoFileName,
						CFG->cfgLocalBaseDir + SCROLLNEWSLOGO_FN );
		}
	}


	CString msg;
	msg.Format( "Downloaded Widgie Images, %d bytes in %.1f seconds", downloadsize, (timeGetTime()-t_beg)/1000.0 );
	Log_App_Event( 0 , msg );


	return downloadsize;
}


// this will download a new exe and store it as Widgie.exe, but also mark a 0 byte file for the version requirements ie Widgie_04.exe
long CWidgieXML::DownloadNewExecuteable() 
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	// news scroll bar seperator
	if( CFG->cfgUpdateExe.IsEmpty() == FALSE )
	{
		CString newfilenum = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + CFG->cfgUpdateExe;

		// if the version # doesnt exist, ie "content/Widgie_16.exe" then its ok to proceed.
		if( FileExists( newfilenum ) == FALSE )
		{
			CString newfile = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "Widgie.exe";

			// delete the previous/old exe that was downloaded.
			DeleteFile( newfile.GetBuffer(0) );

			downloadsize +=
			DownloadBin(CFG->cfgServerContentPath + CFG->cfgUpdateExe, 	newfile );

			// on success
			if( downloadsize > 32000 )
			{
				CFile tmpfile;

				tmpfile.Open( newfilenum, CFile::modeCreate | CFile::modeWrite );
				tmpfile.Close();
				//CFG->cfgExitWhenIdle = TRUE;

				CString msg;
				msg.Format( "Downloaded new Widgie.exe Binary, %d bytes in %.1f seconds", downloadsize, (timeGetTime()-t_beg)/1000.0 );
				Log_App_Event( 0 , msg );
			}
		}
	}
	return downloadsize;
}






















//
// check if we can talk to the server or not...
//
BOOL CWidgieXML::IsServerAlive( void )
{
	BOOL foundStatus = FALSE;

	if( CFG->cfgIPandPort.IsEmpty() == false && InternetAttemptConnect( 0 ) == ERROR_SUCCESS )
	{
		//OutDebugs( "InternetAttemptConnect( 0 ) == ERROR_SUCCESS" );


		// TODO: Add your control notification handler code here
		if( myInternetSession == NULL )
			myInternetSession = new CInternetSession(	NULL,
                            							1,
                            							INTERNET_OPEN_TYPE_PRECONFIG,
                            							NULL,
                            							NULL,
                            							INTERNET_FLAG_DONT_CACHE);
		CHttpConnection* myHTTPConnection = NULL;

		myHTTPConnection = myInternetSession->GetHttpConnection(CFG->cfgIPandPort);
		if( myHTTPConnection )
		{
			CHttpFile *newsfile=NULL;

			newsfile = myHTTPConnection->OpenRequest("","/index.html",NULL,0,NULL,NULL,0);

			bool noConnection = false;
			BOOL success;

			try /* Test to see if the connection is up */
			{
				success = newsfile->SendRequest();
			}
			catch (CInternetException* pEx)
			{
				OutDebugs( "no connection" );
				pEx->Delete();
				noConnection = true;
			}

			if( noConnection )
				foundStatus = FALSE;
			else
				foundStatus = TRUE;

			newsfile->Close();
			delete newsfile;

			myHTTPConnection->Close(); 		
		}

		delete myHTTPConnection;
	}

	return foundStatus;
}




int globalCheckNetConnection(void);
int globalCheckNetAdaptors( CString *netip, CString *netname );




/**
 *  Downloads a TEXT file and saves it to disk
 *
 *  A wrapper function for using CInternetSession and
 *  CHttpConnection to download a text file and save it 
 *  to a file
 *
 *  TODO: Try to re-connect to the server if the connection is down
 *  
 *  
 *  @param  fileToGet   The full URL of the file to download
 *                      e.g. http://www2.itvme.com:80/get_loopsequence.asp
 *
 *  @param  fileToWrite The name of the file to write to disk
 *                      The file name must inlude the full path where 
 *                      the file is to be written
 *                      e.g. c:\content\loopsequence.xml
 */
long CWidgieXML::DownloadText( CString remoteFile, CString fileToWrite, CString uid )
{
	/* open a socket to the iTV server and download the newsflashes */
	long	downloadsize = 0;

	if( AFXDIALOG->shuttingdDown == TRUE )
	{
		OutDebugs( "Skipping Downloading of text during shutdown..." );
		// bugger we are shutting down, DONT DOWNLOAD NOW....
		return 0;
	}



	// TODO: Add your control notification handler code here
	if( myInternetSession == NULL )
		myInternetSession = new CInternetSession(	NULL,
                            	                    1,
                            	                    INTERNET_OPEN_TYPE_PRECONFIG,
                            	                    NULL,
                            	                    NULL,
                            	                    INTERNET_FLAG_DONT_CACHE);
	CHttpConnection* myHTTPConnection = NULL;

    CHttpFile *newsfile=NULL;

	CString fileToGet = remoteFile;

	BOOL ivaserver = FALSE;

	// if we have a full real url, rather than a filename, extract details, and connect as usual...
	if( fileToGet.Find( "http://", 0 ) == 0 )
	{
		char server[256], username[32], passwd[32], file[256];
		ExtractUserFromURL( fileToGet.GetBuffer(0), server, username, passwd,file );

		OutDebugs( "Downloading Text from URL '%s%s'", server, file );

		myHTTPConnection = myInternetSession->GetHttpConnection( server );
		newsfile = myHTTPConnection->OpenRequest("",file,NULL,0,NULL,NULL,0);
	} else 
	if( CFG->cfgIPandPort.IsEmpty() == false )
	{
		// add the ID of the client;
		fileToGet += "?id=";
		if( (uid && !uid.IsEmpty()) )
		{
			fileToGet += uid;
			if( CFG->cfgID.IsEmpty() == FALSE )
			{
				fileToGet += "&simid=";
				fileToGet += CFG->cfgID;
			}
		} else
		{
			if( CFG->cfgID.IsEmpty() == FALSE )
				fileToGet += CFG->cfgID;
			else
				fileToGet += "xxx";
		}

		OutDebugs( "Downloading Text from '%s%s'", CFG->cfgIPandPort.GetBuffer(0), fileToGet.GetBuffer(0) );

		myHTTPConnection = myInternetSession->GetHttpConnection(CFG->cfgIPandPort);
		newsfile = myHTTPConnection->OpenRequest("",fileToGet,NULL,0,NULL,NULL,0);

		ivaserver = TRUE;
	}

	bool noConnection = false;
    BOOL success;

    try /* Test to see if the connection is up */
    {
        success = newsfile->SendRequest();
    }
    catch (CInternetException* pEx)
	{
		OutDebugs( "ERROR: DownloadXML has no connection, remotefile = %s", remoteFile.GetBuffer(0) );
        pEx->Delete();
		noConnection = true;
    }
    
    /* If the connection is not up don't download the file */
    if ((noConnection == false) && (success == TRUE))
    {
        CString buff;
		CStdioFile localNewsfile; 

		currentDownloadFile = fileToGet;

		char buffMemory[1000];
		long dataToGet = (long)newsfile->GetLength(), statusCode=200;

		dataToGet = 0;
		long len = 32;
		if( newsfile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH , buffMemory, (LPDWORD)&len, NULL ) )
			dataToGet = atoi(buffMemory);

		len = 32;
		if( newsfile->QueryInfo( HTTP_QUERY_STATUS_CODE , buffMemory, (LPDWORD)&len, NULL ) )
			statusCode = atoi(buffMemory);

		if( statusCode >= 404 || dataToGet == 1 )
		{
			CString msg;
			msg.Format( "Downloader, Remote file %s does not exist, skipping download", fileToGet );
			Log_App_FileError( errMediaMissing, fileToGet, msg.GetBuffer(0) );
			OutDebugs( "ERROR: Statuscode=%d , Remote file %s does not exist, skipping download", statusCode, fileToGet.GetBuffer(0) );
		}
		else
		if( dataToGet < 32 && !uid && ivaserver )		// only try a new request on the IVA server...
		{
			OutDebugs( "ID failed, trying default ID instead" );
			downloadsize = DownloadText( remoteFile, fileToWrite, _T("1") );
		}
		else
		{
			CString tempFile = fileToWrite + ".tmp";

			if( localNewsfile.Open(tempFile, CFile::modeCreate | CFile::modeWrite) )
			{
				OutDebugs( "Saving download file to %s (%d size)", tempFile.GetBuffer(0), dataToGet );

				long t1 = (timeGetTime());

				while( newsfile->ReadString(buff)==1 && AFXDIALOG->shuttingdDown==FALSE )
				{
					downloadsize += buff.GetLength();
					//Debug(buff);
					/* Write it to file */
					localNewsfile.WriteString(buff);
				
					/* the \n is put in to retain the formatting of the file
						after it is downloaded. This is not important for our
						application. It leaves the files in ahuman readable form. */

					localNewsfile.WriteString("\n");

				}
				localNewsfile.Close();

				t1 = (timeGetTime()-t1);
				if( t1 )
					OutDebugs( "Download Completed in %.1f seconds, %d bytes (%d bps)", t1/1000.0, downloadsize, 8000*downloadsize/t1 );

				if( downloadsize )
				{
					myDeleteFile( fileToWrite );		// delete original
					MoveFile( tempFile.GetBuffer(0), fileToWrite.GetBuffer(0) );		// rename temp to original
				}

			} else {
				CString msg;
				msg.Format( "Downloader cannot write to file - %s", GetLastErrorString() );
				Log_App_FileError( errMediaMissing, tempFile, msg.GetBuffer(0) );
			}
		}
    }
    

//	currentDownloadFile = "";

	newsfile->Close();
    myHTTPConnection->Close(); 		
//	myInternetSession->Close();

	delete newsfile;
    delete myHTTPConnection;
    //delete myInternetSession;

	return downloadsize;
}

/**
 *  Downloads a BINARY file and saves it to disk
 *
 *  A wrapper function for using CInternetSession and
 *  CHttpConnection to download a BINARY file and save it 
 *  to a file
 *
 *  TODO: Try to re-connect to the server if the connection is down
 *
 *  
 *  @param  fileToGet   The full URL of the file to download
 *                      e.g. http://www2.itvme.com:80/get_loopsequence.asp
 *
 *  @param  fileToWrite The name of the file to write to disk
 *                      The file name must inlude the full path where 
 *                      the file is to be written
 *                      e.g. c:\content\loopsequence.xml
 */
long CWidgieXML::DownloadManifestBin(CString remoteFile, CString fileToWrite, CString uid )
{
	long downloadsize = 0, localFileSize = 0;
	/* open a socket to the iTV server and download the newsflashes */

	struct stat sbuf;
	if( stat( fileToWrite.GetBuffer(0), &sbuf ) )
	{
		downloadsize += DownloadBin( remoteFile, fileToWrite, uid );
	}

	return downloadsize;
}



long CWidgieXML::DownloadBin(CString remoteFile, CString fileToWrite, CString uid )
{
	long downloadsize = 0, localFileSize = 0;
	/* open a socket to the iTV server and download the newsflashes */
	m_currentDownloaded = 0;


	if( AFXDIALOG->shuttingdDown == TRUE )
	{
		OutDebugs( "Skipping Downloading of text during shutdown..." );
		// bugger we are shutting down, DONT DOWNLOAD NOW....
		return 0;
	}


	CFile localBinFile;
	if( localBinFile.Open(fileToWrite, CFile::modeRead) ) 
	{
		localFileSize = (long)localBinFile.GetLength();
		localBinFile.Close();
	}


	if( remoteFile && remoteFile.IsEmpty() == FALSE )
	{
		CString httpServerStr, fileToGet;
		if( remoteFile.Find( "http://", 0 ) == 0 )
		{
			char server[256], username[32], passwd[32], file[256];
			ExtractUserFromURL( remoteFile.GetBuffer(0), server, username, passwd,file );
			httpServerStr = server;
			fileToGet = file;
		} else
		{
			httpServerStr = CFG->cfgIPandPort;
			fileToGet = remoteFile;

			// add the ID of the client;
			fileToGet += "?id=";
			if( (uid && !uid.IsEmpty()) )
			{
				fileToGet += uid;
				if( CFG->cfgID.IsEmpty() == FALSE )
				{
					fileToGet += "&simid=";
					fileToGet += CFG->cfgID;
				}
			} else
			{
				if( CFG->cfgID.IsEmpty() == FALSE )
					fileToGet += CFG->cfgID;
				else
					fileToGet += "xxx";
			}
		}

		// TODO: Add your control notification handler code here
		if( myInternetSession == NULL )
			myInternetSession = new CInternetSession(	NULL,
                            							1,
                            							INTERNET_OPEN_TYPE_PRECONFIG,
                            							NULL,
                            							NULL,
                            							INTERNET_FLAG_DONT_CACHE);
	
		CHttpConnection* myHTTPConnection = NULL;

		myHTTPConnection = myInternetSession->GetHttpConnection(httpServerStr);

		OutDebugs( "Downloading Binary from '%s%s'", httpServerStr.GetBuffer(0), fileToGet.GetBuffer(0) );

		CHttpFile *newsfile = newsfile = myHTTPConnection->OpenRequest("",fileToGet,NULL,0,NULL,NULL,0);

        bool noConnection = false;
        
        try /* Test to see if the connection is up */
        {
            newsfile->SendRequest();
        }
        catch (CInternetException* pEx)
	    {
            pEx->Delete();
		    noConnection = true;
        }
        
        /* If the connection is not up don't download the file */
        if (noConnection == false)
        {
		    char* buff;
            char buffMemory[1000];
            buff = &buffMemory[0];

		    CFile localBinFile;

			currentDownloadFile = fileToGet;

			// Lets fine out if the file is already on the local disc...
			long statusCode = 200;
			long dataToGet = (long)newsfile->GetLength();

			dataToGet = 0;
			long len = 32;
			if( newsfile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH , buffMemory, (LPDWORD)&len, NULL ) )
				dataToGet = atoi(buffMemory);

			if( dataToGet < 2048 )
			{
				long len = 32;
				if( newsfile->QueryInfo( HTTP_QUERY_STATUS_CODE , buffMemory, (LPDWORD)&len, NULL ) )
					statusCode = atoi(buffMemory);
			}
			
			OutDebugs( "Remote file length = %ld, statusCode = %d", dataToGet ,statusCode );

			if( statusCode >= 400 )
			{
				Log_App_FileError( errMediaMissing, fileToGet, "Remote file does not exist, skipping download" );
			}
			else
			if( localFileSize!=0 && localFileSize == dataToGet )
			{
				//Log_App_FileError( errMediaMissing, fileToWrite, "File already exists of the same size or less" );
				OutDebugs( "File %s already exists of the same size (%d), download aborted", fileToWrite.GetBuffer(0), localFileSize );
			}
			else
			if( dataToGet < 32 && !uid )
			{
				OutDebugs( "ID failed, trying default ID instead" );
				downloadsize = DownloadBin( remoteFile, fileToWrite, _T("1") );
			}
			else
			{
				long t1 = timeGetTime();
				CString tempFile = fileToWrite + ".tmp";
				m_currentDownloaded = 0;

				// download file into tempFile, then delete original, rename temp to file
				if(	localBinFile.Open( tempFile, CFile::modeCreate | CFile::modeWrite) )
				{
					OutDebugs( "Downloading now..." );

					int buffSize = sizeof(buffMemory);

					int newDataSize = 1;

					try
					{
						CFileStatus status;
						// read 1000 bytes at a time from server and save to local file....
						while(	newDataSize != 0 && 
								newsfile && 
								newsfile->GetStatus( status ) &&
								AFXDIALOG->shuttingdDown==FALSE )
						{
							newDataSize = newsfile->Read(buff, buffSize);
							if( newDataSize )
								localBinFile.Write(buff, newDataSize);
							downloadsize += newDataSize;
							m_currentDownloaded = downloadsize;
							//m_currentDownloadSpeed = ((8000*downloadsize)/((timeGetTime()-t1)));
						}
					}
					catch (CInternetException* pEx)
					{
						pEx->Delete();
					}

					localBinFile.Close();

					t1 = (timeGetTime()-t1);

					if( downloadsize>256 && (downloadsize == dataToGet || statusCode==200) )
					{
						myDeleteFile( fileToWrite );
						MoveFile( tempFile.GetBuffer(0), fileToWrite.GetBuffer(0) );

						if( downloadsize && t1>0 )
						{
							OutDebugs( "Download Completed in %.1f seconds, %d bytes (%d bps)", t1/1000.0, downloadsize, (8000*downloadsize)/(t1) );
						}

						// Now since we downloaded a new jpeg, lets delete the old BMP so as not to have the wrong image
						if( downloadsize && fileToWrite.Find( ".jpg" )>0 )
						{
							if( fileToWrite.Replace( ".jpg", ".bmp" )>0 ){
								myDeleteFile( fileToWrite );
								OutDebugs( "Deleted old file %s", fileToWrite.GetBuffer(0) );
							}
						}
					} else 
					{
						// download incomplete, delete file and return 0
						OutDebugs( "Download incomplete, %d out of %d done, deleting file now", downloadsize, dataToGet );
						myDeleteFile( tempFile );
						downloadsize = 0;
					}
				} else {
					CString msg;
					msg.Format( "Cannot write to file - %s", GetLastErrorString() );
					Log_App_FileError( errMediaMissing, tempFile, msg.GetBuffer(0) );
				}
			}

			if( newsfile )
				newsfile->Close();

        } else
			OutDebugs( "Cannot make net connection" );

		//currentDownloadFile = "";

		myHTTPConnection->Close(); 
		delete myHTTPConnection;

		//myInternetSession->Close();
		//delete myInternetSession;

		delete newsfile;
	}
	return downloadsize;
}


















































/**
 *  Parse a loop sequence XML file
 *
 *  Parses a file that meets the specification for loop sequence
 *  file as described int the iTV In-vehicle Appliance  
 *  Series 2 Development Manual.
 *
 *  TODO: at the moment strict validation is applied
 *        an outcome of this is that if an XML file is
 *        misformed (i.e. contains an error) parsing fails.
 *        This is not appropriate if we want to implement 
 *        degraded performance due to an error.
 *
 *        Action: Must remove validation and implementt error
 *        handling for misformed XML file
 *  
 *  @return     true    if the Loop Sequence is current
 *                      i.e. the system date (time) lies
 *                      between the start and end dates
 *                      for the loop sequence
 *
 *              false   if the Loop Sequence is not current
 *
 */

bool CWidgieXML::ParseLoop( ImpList *impList, CString languageToUse )
{
	// use the language passed as a parameter, otherwise default to the currently global language setting.
	if( !languageToUse )
		languageToUse = StrLanguage;
	else
	if( languageToUse.IsEmpty() )
		languageToUse = StrLanguage;

	if( !impList )
		impList = GetImpressionList(0);

	try {
		 XMLPlatformUtils::Initialize();
	}
    catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
        
        Debug ("Error during initialization! :\n");
        Debug ("Exception message is: \n");
           
		CString messageString;
		message = messageString.GetBuffer(100);
		Debug (messageString + "\n");

        Log_App_FileError(errXML, "The Parser!", "Error during initialization! : " + messageString);

		messageString.ReleaseBuffer();
        XMLString::release(&message);
	}
     
    SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
    parser->setFeature(XMLUni::fgSAX2CoreValidation, true);   // optional
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);   // optional
    parser->setFeature(XMLUni::fgXercesContinueAfterFatalError, true);

	static LoopSeqXML_Handler* defaultHandler = NULL;
	if( defaultHandler == NULL )
		defaultHandler = new LoopSeqXML_Handler(&m_OutputEdit, impList, NULL, languageToUse );
	else
		defaultHandler->Reset_Handler(&m_OutputEdit, impList, NULL, languageToUse );

    parser->setContentHandler(defaultHandler);
    parser->setErrorHandler(defaultHandler);

	try 
	{
		CString loopPath = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName;
		char* xmlFile = loopPath.GetBuffer(0); 

#ifdef _DEBUG2
		struct stat sbuf;
		// if the xml file doesnt exist, download it...
		if( stat( xmlFile, &sbuf ) ){
			DownloadText(CFG->cfgLoopsequenceFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName);
		}
#endif
		OutDebugs( "Parsing Loop XML file - %s",  xmlFile );
		parser->parse(xmlFile);

		BOOL currentLoop = defaultHandler->IsCurrent();
		delete parser;
		//delete defaultHandler;		defaultHandler = NULL;
		return currentLoop;
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
	//delete defaultHandler;	defaultHandler = NULL;

	return true;
}




bool CWidgieXML::ParseLoops( void )
{
	ImpList *impList;

	impList = GetImpressionList();
	ParseLoop( impList );

	// default to currently selected laugnage list, (ie. the last one choosen as the selected language)
	impList = GetImpressionList();

	for( POSITION Imppos = impList->GetHeadPosition(); Imppos != NULL; )
	{
		CString DebugString;
		ImpressionData ImpData;
		ImpData = impList->GetNext(Imppos);
	
	}
	return true;
}





bool CWidgieXML::ParseConfigDefaults( void )
{
	try {
		 XMLPlatformUtils::Initialize();
	}
    catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
        
        Debug ("Error during initialization! :\n");
        Debug ("Exception message is: \n");
           
		CString messageString;
		message = messageString.GetBuffer(100);
		Debug (messageString + "\n");

        Log_App_FileError(errXML, "The Parser!", "Error during initialization! : " + messageString);

		messageString.ReleaseBuffer();
        XMLString::release(&message);
	}
     
    SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
    parser->setFeature(XMLUni::fgSAX2CoreValidation, true);   // optional
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);   // optional
    parser->setFeature(XMLUni::fgXercesContinueAfterFatalError, true);

	static LoopSeqXML_Handler* defaultHandler = NULL;
	if( defaultHandler == NULL )
		defaultHandler = new LoopSeqXML_Handler(&m_OutputEdit, NULL, NULL, NULL );
	else
		defaultHandler->Reset_Handler(&m_OutputEdit, NULL, NULL, NULL );

    parser->setContentHandler(defaultHandler);
    parser->setErrorHandler(defaultHandler);

	try 
	{
		CString Path = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + DEFURL_CONFIG;
		char* xmlFile = Path.GetBuffer(0); 

#ifdef _DEBUG2
		struct stat sbuf;
		// if the xml file doesnt exist, download it...
		if( stat( xmlFile, &sbuf ) ){
			DownloadText( CFG->cfgConfigFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + DEFURL_CONFIG );
		}
#endif
		OutDebugs( "Parsing Config XML file - %s",  xmlFile );
		parser->parse(xmlFile);
		OutDebugs( "Parsing Config XML file - Complete" );
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
	delete defaultHandler;	defaultHandler = NULL;

	return true;
}






/**
 *  Parse a Newsflash XML file
 *
 *  Parses a file that meets the specification for newsflash
 *  file as described int the iTV In-vehicle Appliance  
 *  Series 2 Development Manual.
 *
 *  TODO: at the moment strict validation is applied
 *        an outcome of this is that if an XML file is
 *        misformed (i.e. contains an error) parsing fails.
 *        This is not appropriate if we want to implement 
 *        degraded performance due to an error.
 *
 *        Action: Must remove validation and implement error
 *        handling for misformed XML file
 *
 */

void CWidgieXML::ParseNews()
{

	try {
		 XMLPlatformUtils::Initialize();
	}
    catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
        Debug ("Error during initialization! :\n");
        Debug ("Exception message is: \n");
           
		CString messageString;
		message = messageString.GetBuffer(BUFFER_LENGTH);
		Debug ( messageString + "\n");

        Log_App_FileError(errXML, "The Parser!", "Error during initialization! : " + messageString);
		messageString.ReleaseBuffer();
        XMLString::release(&message);
			
	}

    SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
    parser->setFeature(XMLUni::fgSAX2CoreValidation, true);   // optional
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);   // optional
    
    /*  Keep going even if the XML is misformed we want to pick up as many valid and wellformed news captions as possible */
    parser->setFeature(XMLUni::fgXercesContinueAfterFatalError, true);

    NewsXML_Handler* defaultHandler = NULL;
	//if( defaultHandler == NULL )
	defaultHandler = new NewsXML_Handler( &m_OutputEdit, &newsflashList );

	parser->setContentHandler(defaultHandler);
    parser->setErrorHandler(defaultHandler);

    try 
	{
		//		char* xmlFile = newsPath.GetBuffer(newsPath.GetLength()); /* "C:\\xml\\news.xml"; */   
		CString newsPath = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalNewsName;

#ifdef _DEBUG2
		struct stat sbuf;
		// if the xml file doesnt exist, download it...
		if( stat( newsPath.GetBuffer(0), &sbuf ) ){
			DownloadText(CFG->cfgNewsFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalNewsName);
		}
#endif
		parser->parse( newsPath.GetBuffer(0) );

		// download external news sources in RSS format.....
		if( CFG->cfgUseSevenTicker ){
			newsPath = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + XMLFN_NATIONALNEWS;
			OutDebugs( "Parsing SEVEN NEWS XML file %s", newsPath.GetBuffer(0) );
			parser->parse( newsPath.GetBuffer(0) );
		}

		if( CFG->cfgUseYahooTicker ){
			newsPath = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + XMLFN_YAHOOWORLDNEWS;
			OutDebugs( "Parsing YAHOO NEWS XML file %s", newsPath.GetBuffer(0) );
			parser->parse( newsPath.GetBuffer(0) );
		}

		if( CFG->cfgUseBBCTicker ){
			newsPath = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + XMLFN_BBCWORLDNEWS;
			OutDebugs( "Parsing BBC NEWS XML file %s", newsPath.GetBuffer(0) );
			parser->parse( newsPath.GetBuffer(0) );
		}

		/////////
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
	catch (...) {
		Debug("Unexpected Exception Dude");
		Log_App_FileError(errXML, "Unexpected Exception ok", " ");
	}

	delete parser;
	delete defaultHandler;


	// debug stuff...
	for( POSITION pos = newsflashList.GetHeadPosition(); pos != NULL; )
	{
		NewsflashInfo nfi;
		nfi.data = newsflashList.GetNext(pos);
		nfi.SetDebug(&(this->m_OutputEdit));
		if (!nfi.Expired())
		{
			Debug(nfi.data.text);
		}
	}
}






/**
 *  Parse a Manifest XML file
 *
 *  Parses a file that meets the specification for manifest
 *  file as described int the iTV In-vehicle Appliance  
 *  Series 2 Development Manual.
 *
 *  TODO: at the moment strict validation is applied
 *        an outcome of this is that if an XML file is
 *        misformed (i.e. contains an error) parsing fails.
 *        This is not appropriate if we want to implement 
 *        degraded performance due to an error.
 *
 *        Action: Must remove validation and implement error
 *        handling for misformed XML file
 *
 */

void CWidgieXML::ParseManifest()
{
	OutDebugs( "Parsing Manifest XML data" );

	try {
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
		Debug ("Error during initialization! :\n");
		Debug ("Exception message is: \n");
	        
		CString messageString;
		message = messageString.GetBuffer(100);
		Debug ( messageString + "\n");
		Log_App_FileError(errXML, "The Parser!", "Error during initialization! : " + messageString);
		messageString.ReleaseBuffer();
		XMLString::release(&message);
	}

    
	SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreValidation, true);   // optional
	parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, true);   // optional

    /*  Keep going even if the XML is misformed we want to pick up as many valid and wellformed news captions as possible */
    parser->setFeature(XMLUni::fgXercesContinueAfterFatalError, true);

	ManifestXML_Handler* defaultHandler = NULL;
	defaultHandler = new ManifestXML_Handler(&m_OutputEdit, &fileList);
	parser->setContentHandler(defaultHandler);
	parser->setErrorHandler(defaultHandler);

	try 
	{
		CString manifestPath = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalManifestName;
		char* xmlFile = manifestPath.GetBuffer(0); /* "C:\\xml\\manifest.xml"; */

#ifdef _DEBUG2
		struct stat sbuf;
		// if the xml file doesnt exist, download it...
		if( stat( xmlFile, &sbuf ) ){
			DownloadText(CFG->cfgManifestFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalManifestName);
		}
#endif
		OutDebugs( "Starting Manifest XML decoder" );
		parser->parse(xmlFile);
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
	catch (...) {
		OutDebugs("Unexpected Exception Dude");
		Log_App_FileError(errXML, "Unexpected Exception ok", " ");
	}
	delete parser;
	delete defaultHandler;
}

/**
 *  Deletes all XML and Content the files 
 *
 *  NOTE: This function is not used at all my the MCCA
 *        it was create for development purposes
 *
 *  Deletes all file is the directory specified by:
 *          cfgLocalBaseDir + cfgLocalXML_Dir
 *
 *  Deletes all file is the directory specified by:
 *          cfgLocalBaseDir + cfgLocalContent_Dir
 *
 *
 */
void CWidgieXML::DeleteFiles() 
{
	// TODO: Add your control notification handler code here
    
    WIN32_FIND_DATA findData;
    HANDLE fileToDelete;
    BOOL foundFile = TRUE; 
    
    fileToDelete = FindFirstFile(CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + "*.???", &findData);
    
    while ((fileToDelete != INVALID_HANDLE_VALUE) && (foundFile != FALSE))
    {
        if ((findData.cFileName != ".") && (findData.cFileName != ".."))
        {
            int delResult = DeleteFile(CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + findData.cFileName);
        }
        
        foundFile = FindNextFile(fileToDelete, &findData); 
    }

    foundFile = TRUE;
    fileToDelete = FindFirstFile(CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "*.???", &findData);
    
    while ((fileToDelete != INVALID_HANDLE_VALUE) && (foundFile != FALSE))
    {
        if ((findData.cFileName != ".") && (findData.cFileName != ".."))
        {
            int delResult = DeleteFile(CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + findData.cFileName);
        }
        
        foundFile = FindNextFile(fileToDelete, &findData); 
    }
}

/**
 *  Deletes thel XML and Content directories
 *
 *  NOTE: This function is not used at all my the MCCA
 *        it was create for development purposes
 *
 *  Deletes the directory specified by:
 *          cfgLocalBaseDir + cfgLocalXML_Dir
 *
 *  Deletes the directory specified by:
 *          cfgLocalBaseDir + cfgLocalContent_Dir
 *
 *
 */
void CWidgieXML::DeleteDir() 
{
	// TODO: Add your control notification handler code here
    BOOL removeDirResult = RemoveDirectory(CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + "\0");

	LPVOID lpMsgBuf;

	if (removeDirResult == FALSE)
	{				
		DWORD deleteError = GetLastError();
		
    	FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			deleteError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);

		OutDebugs("ERROR: Could not remove XML directory %s",lpMsgBuf);

		LocalFree( lpMsgBuf );
    }
	else
    {
		Debug("XML directory removed successfully");
	}
    
    removeDirResult = RemoveDirectory(CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "\0");

	if (removeDirResult == FALSE)
	{				
		DWORD deleteError = GetLastError();
		
    	FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			deleteError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);

		OutDebugs("ERROR: Could not remove Content directory %s", lpMsgBuf );

		LocalFree( lpMsgBuf );
    }
	else
    {
		Debug("Content directory removed successfully");
	}

}






/**
 *  Writes a debug string to a file or an Edit box
 *
 *  For speed purposes this code is #define out.
 *  
 *  Uncomment the appropriate #define section when debug
 *  is required
 */
void CWidgieXML::Debug(CString debugMessage)
{
//#define DEBUG_TO_EDIT 
#ifdef DEBUG_TO_EDIT

    if (m_OutputEdit)
    {
        CString tempString;
	    this->m_OutputEdit.GetWindowText(tempString);
	    this->m_OutputEdit.SetWindowText(tempString + debugMessage + "\r\n");
        this->m_OutputEdit.LineScroll(this->m_OutputEdit.GetLineCount());
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
	delete errorFile;
#endif
}






CImage *CWidgieXML::GetImpressionImage(int pos)
{
	CImage *image;
	ImpList *impList = GetImpressionList();
	POSITION newPos = impList->FindIndex(pos);
	if( newPos )
	{
		ImpressionData thisImpression = impList->GetAt( newPos );

		image = (CImage*)thisImpression.imageData;

		if( image )
		{
			thisImpression.imageData = NULL;
			impList->SetAt(newPos, thisImpression);
		}

		return image;
	} else
		return NULL;
}

int CWidgieXML::FreeImpressionImage(int pos)
{
	CImage *image;

	if( image = GetImpressionImage(pos) )
	{
		image->Destroy();
	}
	return 0;
}












// ###################################  UPLOAD ERRORS LOG, ROTATE LOGS  #####################################
#include "zlib.h"
// This now supports gzip or bzip2 to compress with.
long GZipCompressFiles( char *sourcefile  )
{
	long	dataread = 0, dataout;
	long    datatotal= 0;
	char	newlogname[512];
	void *outfp;
	char *ram;
	long blocksize = 1024*32;

	if ( ram = (char*)malloc( blocksize ) )
	{
		CFile input;

		if( input.Open( sourcefile, CFile::modeRead ) )
		{
			sprintf( newlogname, "%s.gz", sourcefile );
			outfp = gzopen( newlogname, "wb6" );

			dataout = 0;
			{
				dataread = 1;
				while( dataread>0 )
				{
					dataread = input.Read( ram, blocksize );
					if ( dataread>0 )
					{
						datatotal += dataread;
						gzwrite( outfp, ram , dataread );
					}
				}
				OutDebugs( "gzclose" );
				gzclose( outfp );
				OutDebugs( "gzclose done" );
			}
			input.Close();
		}
		free( ram );
	}
	return datatotal;
}




#include "zipmake.h"
// Compress a file on disc into ZIP format.
int zipUpFileX( CString sourcefile, CString zfilename, CString filename )
{
	CFile input;
	int totalread = 0;

	if( input.Open( sourcefile.GetBuffer(0), CFile::modeRead ) )
	{
		zipFile zipData;

		//OutDebugs( "zip open" );
		zipData = zipOpen( (const char *)zfilename.GetBuffer(0), 0 );
		if( zipData )
		{
			static char data[1032];
			static zip_fileinfo zinfo;
			int readsize = 0;

			//OutDebugs( "memset" );
			memset( &zinfo, 0, sizeof(zip_fileinfo) );

			//OutDebugs( "zipOpenNewFileInZip" );
			zipOpenNewFileInZip( zipData, filename.GetBuffer(0), &zinfo, 0, 0, 0, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION );

			//OutDebugs( "input.Read" );
			while( readsize = input.Read( data, 1000 ) )
			{
				totalread += readsize;
				//OutDebugs( "zipWriteInFileInZip %d bytes", readsize );
				zipWriteInFileInZip( zipData, data , readsize );
			}

			//OutDebugs( "zipCloseFileInZip %d bytes", totalread );
			zipCloseFileInZip( zipData );

			//OutDebugs( "zipClose" );
			zipClose( zipData, NULL );
		}
	}
	return totalread;
}


#include "winnet_io.h"


