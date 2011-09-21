// Widgie XMLDlg.cpp : implementation file
/*

Future Enhancements :
1. save slideshows to slideshow_nn.xml instead of just one xml file.















*/

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


#define __HTTP_VERB_GET	"GET"
#define __HTTP_VERB_POST "POST"
#define __HTTP_ACCEPT_TYPE "*/*"
#define __HTTP_ACCEPT "Accept: */*\r\n"


/////////////////////////////////////////////////////////////////////////////
// CWidgieXML

static int AreWeOnline( void )
{
	if ( InternetAttemptConnect( 0 ) == ERROR_SUCCESS )
		return 1;
	else
		return 0;

}

static BOOL myDeleteFile( CString file )
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


















// ------------ CLASS BEGIN




CWidgieXML::CWidgieXML( CString language, CEdit* debugEdit )
{
    StrLanguage = language;
	IntLanguage = 0;

	m_slideshowID = -1;
	m_slideshowElement = "";
    
	myInternetSession = NULL;

	m_downloadNewsCount =
	m_downloadConfigCount =
	m_downloadSlideshowCount =
	m_downloadLoginCount =
	m_downloadMediaCount = 0;
	
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

bool VerifyDestinationDir( CString pPathName )
{
	bool Result = CreateDirectory(pPathName , NULL);
	
	LPVOID lpMsgBuf;

	if (Result == FALSE)
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

			OutDebugs( "Could not create directory: %s", pPathName );
			LocalFree( lpMsgBuf );
		}
		else
		{
			OutDebugs("XML directory (%s) created successfully", pPathName );
		}
	}
	return Result;
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
	/* check if the Content directory exists  if it doesn't create it */

	CString lpPathName = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + "\0";
	
	BOOL xmlDirCreateResult = VerifyDestinationDir(lpPathName);
	
	long downloadsize = 0;
	DWORD t_beg = timeGetTime();

	OutDebug( "Downloading XML news" );
	downloadsize +=
	DownloadXMLNews();

	OutDebugs( "Downloading XML loop" );
	downloadsize +=
	DownloadXMLLoop();

	/* OBSELETE, manifest no longer needed as we just download missing files from the slide show
	OutDebugs( "Downloading XML manifest" );
	downloadsize +=
	DownloadXMLManifest();
	*/

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

	long downloadsize = 0, totaldownload = 0;

	if( CFG->cfgUseRSSNews )
	{
		// download all our news RSS feeds.
		for( int i=0; i< CFG->cfgNewsRSSFeeds.GetCount();i++)
		{
			CString destFile;
			destFile.Format( "%s\\news%d.rss", CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir, i );

			downloadsize = DownloadText( NULL, CFG->cfgNewsRSSFeeds.GetAt( CFG->cfgNewsRSSFeeds.FindIndex(i) ), destFile );
			if( downloadsize < 1 )
				OutDebugs( "ERROR: News Failed (%d) to download %s", downloadsize, CFG->cfgNewsRSSFeeds.GetAt( CFG->cfgNewsRSSFeeds.FindIndex(i) ) );
			else
				totaldownload += downloadsize;
		}
	}

	// local news XML
	if( CFG->cfgIPandPort.GetLength()>0 )
	{
		downloadsize = DownloadText( CFG->cfgIPandPort, CFG->cfgNewsFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalNewsName);
		if( downloadsize < 1 )
			OutDebugs( "ERROR: News Failed to download %s%s", CFG->cfgIPandPort, CFG->cfgNewsFileName );
		else
			totaldownload += downloadsize;
	}


	if( totaldownload >0 )
	{
		last_good_download_t = time(0);

		CString msg;
		msg.Format( "Downloaded XMLNews, %d bytes in %.1f seconds", totaldownload, (timeGetTime()-t_beg)/1000.0 );
		Log_App_Event( 0 , msg );

	}

	m_downloadNewsCount++;

	return totaldownload;
}



long CWidgieXML::DownloadFile( CString server, CString url )
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	// check that the config version in the loop sequence is newer than the one that was preloaded in the global settings.
	// Config XML
	downloadsize =
	DownloadText( server, url, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + "tmp.data" );

	return downloadsize;
}



// returns a download size if downloaded, else
// returns 0 if it decided NOT to download it or if it failed to download it.
long CWidgieXML::DownloadXMLConfig()
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	/*
	// Parameters!!
	CString params;
		params += "?username=";
		if( !CFG->cfgUsername.IsEmpty() )
			params += CFG->cfgUsername;
		else
			params += "guest";
		params += "&password=";
		params += CFG->cfgUserpass;
		params += "&mid=";				// machine id based of the MAC ADDR
		params += CFG->cfgID;
	*/


	// check that the config version in the loop sequence is newer than the one that was preloaded in the global settings.
	// Config XML
	downloadsize =
	DownloadText( CFG->cfgIPandPort, CFG->cfgConfigFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + DEFURL_CONFIG, NULL, &m_configDateStamp );

	// only log it if we did download a new config, else it probably hasnt been updated, so no need to download it
	if( downloadsize >0 )
	{
		CString msg;
		msg.Format( "Downloaded Config XML, %d bytes in %.1f seconds", downloadsize, t_beg/1000.0 );
		Log_App_Event( 0 , msg );
	}

	m_downloadConfigCount++;

	return downloadsize;
}

//
//
// Download the slideshow xml file
//
//
long CWidgieXML::DownloadXMLLoop()
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	downloadsize =
	MDID_DownloadSlideShow( CFG->cfgIPandPort, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName );

	// if it failed because of wrong session, try again once
	if( downloadsize == -1 )
		downloadsize =
		MDID_DownloadSlideShow( CFG->cfgIPandPort, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName );

	t_beg = timeGetTime()-t_beg;

	if( downloadsize >0 )
	{
		CString msg;
		msg.Format( "Downloaded XML Play list %s, %d bytes in %.1f seconds", 
			CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName,
			downloadsize, t_beg/1000.0 );
		Log_App_Event( 0 , msg );
	} else
	if( downloadsize <0 )
	{
		OutDebugs( "ERROR: failed to download xml file %s for some reason when tried (%d)",
			CFG->cfgLocalXML_Dir.GetBuffer(0),
			downloadsize );
	}

	m_downloadSlideshowCount++;

	return downloadsize;
}



//
// download the manifest list which describes which raw files to download (obslete)
//
// == OBSELETE ==
//
long CWidgieXML::DownloadXMLManifest()
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	// MANIFEST data
	downloadsize =
	DownloadText( CFG->cfgIPandPort, CFG->cfgManifestFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalManifestName);

	t_beg = timeGetTime()-t_beg;

	if( downloadsize >0 )
	{
		CString msg;
		msg.Format( "Downloaded XML Manifest, %d bytes in %.1f seconds", downloadsize, t_beg/1000.0 );
		Log_App_Event( 0 , msg );
	}

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
	OutDebugs( "DEBUG: Parsing News" );
	// parse current news xml file
	ParseNews();

	OutDebugs( "DEBUG: Slideshow" );
	// parse all language's loop sequences...
	ParseLoops();

	OutDebugs( "DEBUG: Config" );
	ParseConfigDefaults();

	OutDebugs( "DEBUG: Manifest" );
	ParseManifest();

	OutDebugs( "DEBUG: Done." );
}







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
 *  manifest XML files
 *
 *  This function is not Used by the MCCA at all
 *  it is an event handler for a button click that 
 *  was developed in conjunction with a GUI for testing
 *  content doneload
 * 
 */

long CWidgieXML::DownloadContent() 
{
	CString lpPathName = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "\0";
	
    BOOL contentDirCreateResult;
	contentDirCreateResult = VerifyDestinationDir(lpPathName);

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





















// ######################    MDID SUPPORT SECTION    ########################




CString CWidgieXML::GetElement( CString p_xml, CString p_elementName )
{
	CString result;
	if( p_xml.IsEmpty() == FALSE )
	{
		int pos;
		// ok now find <SessionToken> and store it for future access rights.
		//<sessiontoken>user_e8cc4a69-960c-4685-959c-2db6a419f0ae</sessiontoken>
		if( (pos = p_xml.Find( p_elementName, 0 )) >= 1 )
		{
			if( (pos = p_xml.Find( '>', pos )) >= 1 ) 
			{
				int pos2 = p_xml.Find( '<', pos );
				result = p_xml.Mid( pos+1, pos2-(pos+1) );
			}
		}
	}
	return result;
}


//
// Extract the xml tags parameter value from its name
//				<slideshow id="8" archived="false" createdondate="8/02/2005" modifiedondate="8/02/2005">Display_2969B9</slideshow>
//						   ^^^^^^
//
//
CString CWidgieXML::GetElementParam( CString p_xml, CString p_paramName )
{
	CString result;
	if( p_xml.IsEmpty() == FALSE )
	{
		int pos = p_xml.Find( p_paramName );
		if( pos >= 1 )
		{
			if( (pos = p_xml.Find( '\"', pos )) >= 1 )		// search for "
			{
				pos++;
				int pos2 = p_xml.Find( '\"', pos );
				result = p_xml.Mid( pos, pos2-(pos) );
			}
		}
	}
	return result;
}


//
//
//
//
// Using form POST, login to MDID and setup a cookie for subsequent logins
//
//
//
//
long CWidgieXML::MDID_Login( CString server, CString remoteFile, CString parameters ) 
{
	bool noConnection = false;
    BOOL success;
	long downloadsize = 0;

	if( AFXDIALOG->shuttingDown == TRUE )
	{
		OutDebugs( "Skipping Downloading of text during shutdown..." );
		// bugger we are shutting down, DONT DOWNLOAD NOW....
		return 0;
	}

	m_downloadLoginCount++;

	// default the server to the normal config
	if( !server || (server && server.IsEmpty()) )
		server = CFG->cfgIPandPort;

	if( myInternetSession == NULL )
		myInternetSession = new CInternetSession(	"Display Agent",
                            	                    1,
													INTERNET_OPEN_TYPE_PRECONFIG,
													//INTERNET_OPEN_TYPE_PROXY,
													NULL,
                            	                    NULL,
                            	                    INTERNET_FLAG_DONT_CACHE );
	CHttpConnection	*myHTTPConnection = NULL;
    CHttpFile		*httpFile=NULL;
	CString			fileToGet = remoteFile;

	// configure the POST arguments to the CGI
	if( server.IsEmpty() == false )
	{
		if( parameters && !parameters.IsEmpty() )
		{
			fileToGet += parameters;
		}
		// ---------------------------
		static LPCTSTR szAcceptType=__HTTP_ACCEPT_TYPE;
		static LPCTSTR szContentType="Content-Type: application/x-www-form-urlencoded\r\n";

		OutDebugs( "MDID LOGIN USER=%s , url='%s%s'", CFG->cfgUsername.GetBuffer(0), server.GetBuffer(0), fileToGet.GetBuffer(0) );

		long port;
		server = URLGetPort( server, &port );

		myHTTPConnection = myInternetSession->GetHttpConnection(server, (INTERNET_PORT)port, NULL, NULL);
		httpFile = myHTTPConnection->OpenRequest( "POST",fileToGet,NULL,0,NULL,NULL,0 );
		httpFile->AddRequestHeaders( __HTTP_ACCEPT, HTTP_ADDREQ_FLAG_REPLACE, strlen(__HTTP_ACCEPT) );

		CString postArgs;
		postArgs.Format( "userID=%s&password=%s", CFG->cfgUsername, CFG->cfgUserpass );

		try /* Test to see if the connection is up */
		{
			success = httpFile->SendRequest( szContentType, strlen(szContentType), (LPVOID)postArgs.GetBuffer(0), postArgs.GetLength() );
		}
		catch (CInternetException* pEx)
		{
			OutDebugs( "ERROR: DownloadXML has no connection, remotefile = %s%s", server.GetBuffer(0), remoteFile.GetBuffer(0) );
			pEx->Delete();
			noConnection = true;
		}
	}

	CString dataResult;

    
    /* If the connection is not up don't download the file */
    if ((noConnection == false) && (success == TRUE))
    {
		CStdioFile localSavedFile; 

		currentDownloadFile = server + fileToGet;

		char buffMemory[1000];
		long dataToGet = (long)httpFile->GetLength(), statusCode=200;

		dataToGet = 0;
		long len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH , buffMemory, (LPDWORD)&len, NULL ) )
			dataToGet = atoi(buffMemory);

		len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_STATUS_CODE , buffMemory, (LPDWORD)&len, NULL ) )
			statusCode = atoi(buffMemory);

		if( (statusCode >= 404 && statusCode < 500)|| dataToGet == 1 )
		{
			OutDebugs( "ERROR: LoginMDID (user=%s), Statuscode=%d , Remote file %s%s does not exist, skipping download", 
				CFG->cfgUsername.GetBuffer(0), statusCode, server.GetBuffer(0), fileToGet.GetBuffer(0) );
		}
		else
		{
			OutDebugs( "STATUS: Logging into MDID as %s / %s", CFG->cfgUsername.GetBuffer(0), server.GetBuffer(0) );

			long t1 = (timeGetTime());

			CString tempFile = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + "\\login.xml";
			CStdioFile localSavedFile;

			// save slideshow to harddisk
			if( localSavedFile.Open(tempFile, CFile::modeCreate | CFile::modeWrite) )
			{
		        CString buff;
				// READ XML FILE FROM SERVER
				while( httpFile->ReadString(buff)==1 && AFXDIALOG->shuttingDown==FALSE )
				{
					buff.Append( "\n" );
					dataResult.Append( buff );
					localSavedFile.WriteString( buff );
					downloadsize += buff.GetLength();
				}
				localSavedFile.Close();

				t1 = (timeGetTime()-t1);
				if( t1 )
					OutDebugs( "HTTP Result = %s", dataResult );
			} else {
				CString msg;
				msg.Format( "Downloader cannot write to '%s' bcoz %s", tempFile, GetLastErrorString() );
				Log_App_FileError( errMediaMissing, tempFile, msg.GetBuffer(0) );
			}
		}
    }
    
	// free net stuff
	if( httpFile )
	{
		httpFile->Close();
		delete httpFile;
	}
	if( myHTTPConnection )
	{
		myHTTPConnection->Close(); 		
	    delete myHTTPConnection;
	}

	// if failed to download for some reason, load xml file from disk
	if( downloadsize <= 0 )
	{
		dataResult = LoadFileintoCString( CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + "\\login.xml" );
		downloadsize = dataResult.GetLength();
	}


	// Process the data returned.....
	if( dataResult.GetLength() >0 )
		MDID_ProcessLogin( dataResult );

	return downloadsize;
}




/* We should have gotton a result of - 

	<?xml version="1.0" encoding="utf-8"?>
	<LoginResponseInfo xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns="http://mdid.jmu.edu/webservices">
	<resultcode>SUCCESS</resultcode>
	<errormessage>n/a</errormessage>
	<sessiontoken>user_a475c01e-5c1f-46cd-845f-77480225e452</sessiontoken>
	<slideshows>
	<folder name="[MAIN]">
	<slideshow id="7" archived="false" createdondate="8/02/2005" modifiedondate="8/02/2005">Display_262B67</slideshow>
	<slideshow id="9" archived="false" createdondate="8/02/2005" modifiedondate="8/02/2005">Display_296911</slideshow>
	<slideshow id="8" archived="false" createdondate="8/02/2005" modifiedondate="8/02/2005">Display_2969B9</slideshow>
	<slideshow id="10" archived="false" createdondate="8/02/2005" modifiedondate="8/02/2005">Display_Demo</slideshow>
	<slideshow id="6" archived="false" createdondate="8/02/2005" modifiedondate="8/02/2005">Display_Demo3</slideshow>
	</folder>
	</slideshows>
	</Login>


Date Ranges for schedule filter.

1. Time Range 			; two HH:MM values to describe in inclusive time range
	"12:00 - 15:00"		noon to 3pm every day.
	"12:00 to 15:00"		is also valid 
	"13:00"					1pm onwards no matter what day.


2. Specific Day 			; specify the day of the week to play back on 
	"Friday"					play on that day only
	"Monday to Wednesday"

3.	Date Ranges 			; specify a range of valid days to play back at.
	"15/04/2005"			only play on that day
	"15/04/2005 to 20/04/2005"
	"15/04/2005 for 3"	start on the 15th, but only for 3 days, ie to the 17th

4. Months					; a wider time range
	"July"					play every day during july

*/
long CWidgieXML::MDID_ProcessLogin( CString loginXML ) 
{
	// vars
	CString buff,
			slideshowDemo;
	int		curPos = 0;
	BOOL	loginSuccess = FALSE;


	if( loginXML.GetLength() >0 )
	{
		CString folderName;
		m_slideshowList.RemoveAll();

		// code
		buff = loginXML.Tokenize("\n",curPos);

		while( buff != "" )
		{
			if( loginSuccess == FALSE && GetElement( buff, "<resultcode>" ) == "SUCCESS" )
				loginSuccess = TRUE;

			if( buff.Find( "<sessiontoken>" ) >= 0 )
			{
				m_sessionToken = GetElement( buff, "<sessiontoken>" );
				m_sessionStartTime = CTime::GetCurrentTime();
				OutDebugs( "Login token = %s", m_sessionToken.GetBuffer(0) );
			}
			// check folder names for Schedule times to filter out the slideshows.
			if( buff.Find( "<folder" ) >= 0 )
			{
				folderName = GetElementParam( buff, "name" );
				// check for date range.
				//if( isdigit( folderName.GetAt( 0 ) )
			} else
			// if we are in the slideshow element, check for our ID
			if( buff.Find( "<slideshow" ) >= 0 )
			{
				CString slideshowName = GetElement( buff, "<slideshow" );

				if( slideshowName.GetLength()>0 )
				{
					m_slideshowList.AddTail( slideshowName );
					// if we find our ID
					if( slideshowName.CompareNoCase( CFG->cfgID ) == 0 )
					{
						m_slideshowElement = buff;
					} else
					if( slideshowName.CompareNoCase( "Display_Demo" ) == 0 )
					{
						slideshowDemo = buff;
					}
				}
			}

			buff = loginXML.Tokenize("\n",curPos);
		}

		if( loginSuccess )
		{
			// if we cannot find our ID, then choose the demo default one, its better than no slideshow at all
			if( m_slideshowElement.IsEmpty() )
				m_slideshowElement = slideshowDemo;

			// retrieve the slideshow ID that matches us from the list of slideshows
			if( !m_slideshowElement.IsEmpty() )
			{
				CString id = GetElementParam( m_slideshowElement, "id" );
				m_slideshowID = atoi( id.GetBuffer(0) );
				m_slideshowDate = GetElementParam( m_slideshowElement, "modifiedondate" );
				OutDebugs( "Using Slideshow ID=%d last=%s", m_slideshowID, m_slideshowDate.GetBuffer(0) );
			}
		}
	}

	return loginSuccess;
}	


//
//
//
//
//
//
//
//
//
// MDID - using webservices and previous login token, download the whole slideshow xml file
//
// Using form POST, Download Appropriate Slideshow based on the ID/NAME
//
//
//
long CWidgieXML::MDID_DownloadSlideShow( CString server, CString fileToWrite ) 
{
	long	downloadsize = 0;

	if( AFXDIALOG->shuttingDown == TRUE )
	{
		OutDebugs( "Skipping Downloading of text during shutdown..." );
		// bugger we are shutting down, DONT DOWNLOAD NOW....
		return 0;
	}

	// default the server to the normal config
	if( !server || (server && server.IsEmpty()) )
		server = CFG->cfgIPandPort;

	// ### LOGIN into MDID to get access to its data
	// but only login if not done so for the last 15mins, as its not likely slideshows will
	// be renamed, but we must recheck every 15mins
	if( m_sessionToken.IsEmpty() || 
		(CTime::GetCurrentTime() - m_sessionStartTime)>(15*60) )			//m_sessionToken.IsEmpty() ||
	{
		MDID_Login( server, URL_MDID_LOGIN, "" );
	}

	if( myInternetSession == NULL )
		myInternetSession = new CInternetSession(	"Display Agent",
                            	                    1,
													INTERNET_OPEN_TYPE_PRECONFIG,
													//INTERNET_OPEN_TYPE_PROXY,
													NULL,
                            	                    NULL,
                            	                    INTERNET_FLAG_DONT_CACHE);
	CHttpConnection* myHTTPConnection = NULL;

    CHttpFile *httpFile=NULL;

	CString fileToGet = URL_MDID_GETSLIDESHOW;

	bool	noConnection = false;
    BOOL	success;
	long	dataToGet;
	long	downloadSum = 0;
	CString dataResult;
	CString tempFile = fileToWrite + ".tmp";


	{
		static LPCTSTR szAcceptType=__HTTP_ACCEPT_TYPE;
		static LPCTSTR szContentType="Content-Type: application/x-www-form-urlencoded\r\n";

		long port;
		server = URLGetPort( server, &port );

		myHTTPConnection = myInternetSession->GetHttpConnection(server, (INTERNET_PORT)port, NULL, NULL);
		httpFile = myHTTPConnection->OpenRequest( "POST",fileToGet,NULL,0,NULL,NULL,0 );
		httpFile->AddRequestHeaders( __HTTP_ACCEPT, HTTP_ADDREQ_FLAG_REPLACE, strlen(__HTTP_ACCEPT) );

		CString postArgs;
		postArgs.Format( "slideshowID=%d&sessiontoken=%s", m_slideshowID, m_sessionToken );

		OutDebugs( "MDID GETSLIDESHOW '%s%s' args=%s", server.GetBuffer(0), fileToGet.GetBuffer(0), postArgs.GetBuffer(0) );

		// add a machine ID string for the proxy, but it would get ignored by our MDID server.
		fileToGet.AppendFormat( "macid=%s", CFG->cfgID );

		try /* Test to see if the connection is up */
		{
			success = httpFile->SendRequest( szContentType, strlen(szContentType), (LPVOID)postArgs.GetBuffer(0), postArgs.GetLength() );
		}
		catch (CInternetException* pEx)
		{
			OutDebugs( "ERROR: DownloadSlideshow has no connection for %s%s", server.GetBuffer(0), fileToGet.GetBuffer(0) );
			pEx->Delete();
			noConnection = true;
		}
	}

    
    /* If the connection is not up don't download the file */
    if ((noConnection == false) && (success == TRUE))
    {
		CStdioFile localSavedFile; 
		char buffMemory[1000];
		long statusCode=200;

		currentDownloadFile = server + fileToGet;
		dataToGet = (long)httpFile->GetLength();

		// get some header details
		long len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH , buffMemory, (LPDWORD)&len, NULL ) )
			dataToGet = atoi(buffMemory);

		len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_STATUS_CODE , buffMemory, (LPDWORD)&len, NULL ) )
			statusCode = atoi(buffMemory);

		if( (statusCode >= 404 && statusCode < 500)|| dataToGet == 1 )
		{
			OutDebugs( "ERROR: Statuscode=%d , Remote file %s%s does not exist, skipping download", statusCode, server.GetBuffer(0), fileToGet.GetBuffer(0) );
		}
		else
		{
			CStdioFile localSavedFile;

			// save slideshow to harddisk
			if( localSavedFile.Open(tempFile, CFile::modeCreate | CFile::modeWrite) )
			{
				long	t1 = (timeGetTime());
				BOOL	resultCode = FALSE;
				CString buff;

				OutDebugs( "Saving to : %s (%d size)", tempFile.GetBuffer(0), dataToGet );

				// read slideshow from webservice and save to file
				while( httpFile->ReadString(buff)==1 && AFXDIALOG->shuttingDown==FALSE )
				{
					downloadSum = HashIt( buff.GetBuffer(0), buff.GetLength(), downloadSum );

					buff.Append( "\n" );
					dataResult.Append( buff );
					localSavedFile.WriteString( buff );
					downloadsize += (buff.GetLength());
				}
				localSavedFile.Close();
				t1 = (timeGetTime()-t1);
				if( downloadsize && t1>0 )
				{
					OutDebugs( "Download Completed in %.1f seconds, %d bytes (%d Kbps)", t1/1000.0, downloadsize, (8*downloadsize/t1) );
				}
				// ----------------------------------------------------
			} else {
				CString msg;
				msg.Format( "Downloader cannot write to '%s' bcoz %s", tempFile, GetLastErrorString() );
				Log_App_FileError( errMediaMissing, tempFile, msg.GetBuffer(0) );
			}
		}
    }

	// free net stuff
	if( httpFile )
	{
		httpFile->Close();
		delete httpFile;
	}
	if( myHTTPConnection )
	{
		myHTTPConnection->Close(); 		
	    delete myHTTPConnection;
	}

	// if failed to download because we cannot connect, load last saved.
	if( noConnection )
	{
		CString newfile = fileToWrite;
		newfile.Replace( ".xml", "_" ); 
		newfile.AppendFormat( "%s.xml", CFG->cfgID );

		dataResult = LoadFileintoCString( newfile );
		downloadsize = dataResult.GetLength();
	}

	// process our XML slideshow data
	if( downloadsize>0 )
	{
		int resultCode = MDID_ProcessSlideShow( dataResult );

		// Ok , now that we downloaded the file into a temp file, rename to original file.
		if( resultCode>0 )
		{
			// only if we really did download something, check the files.
			if( downloadsize>0 && dataToGet>0 )
			{
				// get the old files sum
				long fileSum = CHashFile( fileToWrite );

				// if file is the same, just dump it and not use it.
				if( downloadSum == fileSum )
				{
					OutDebugs( "Slideshow is the same, no need to use it" );
					downloadsize = 0;
				} 
				else
				// if the downloaded file is really different... then use it
				{
					myDeleteFile( fileToWrite );											// delete original .xml
					MoveFile( tempFile.GetBuffer(0), fileToWrite.GetBuffer(0) );			// rename .tmp to .xml

					// also make sure we have a copy of it file named "slideshow_Display_121212.xml" also
					CString newfile = fileToWrite;
					newfile.Replace( ".xml", "_" ); 
					newfile.AppendFormat( "%s.xml", CFG->cfgID );

					CopyFile( fileToWrite.GetBuffer(0), newfile.GetBuffer(0), FALSE );
				}
			} else 
			if( downloadsize != -1 )
			{

				// download incomplete, delete file and return 0
				OutDebugs( "Download failed - %d/%d bytes done, deleting file now",  downloadsize, dataToGet );
				myDeleteFile( tempFile );
				downloadsize = 0;
			}
		} else
		{
			m_sessionToken.Empty();			// clear our login session id
			downloadsize = resultCode;
		}
	}


	return downloadsize;
}


//
// Process the MDID slideshow headers.
// Return 1 for success
//        0 for no slideshow found
//		 -1 for invalid session
//
long CWidgieXML::MDID_ProcessSlideShow( CString slideshowXML ) 
{
	// vars
	CString buff,
			slideshowDemo;
	int		curPos = 0;
	BOOL	resultCode = 0;

	// code
	if( slideshowXML.GetLength() >0 )
	{
		buff = slideshowXML.Tokenize("\n",curPos);

		while( buff != "" )
		{
			if( !resultCode )
			{
				//<resultcode>SUCCESS</resultcode>
				// check for <resultcode>INVALIDSESSION , and break out of the download
				if ( buff.Find( "<resultcode>SUCCESS" ) >=0 )
				{
					resultCode = 1;
					break;
				} else
				if ( buff.Find( "<resultcode>INVALIDSESSION" ) >=0 )
				{
					OutDebugs( "ERROR: Slideshow download failed - invalid session %s", m_sessionToken );
					resultCode = -1;
					break; 
				} else
				if ( buff.Find( "<resultcode>SLIDESHOWNOTFOUND" ) >=0 )
				{ 
					OutDebugs( "ERROR: Slideshow for '%s' not found on server", CFG->cfgID );
					resultCode = 0; 
					break; 
				}
			}
			buff = slideshowXML.Tokenize("\n",curPos);
		}
	}
	return resultCode;
}










//
//
//
//
// Using form POST, login to MDID and setup a cookie for subsequent logins
//
//
//
//
long CWidgieXML::ServerPostAction( CString server, CString remoteFile, CString postArgs ) 
{
	BOOL loginSuccess = FALSE;
	bool noConnection = false;
    BOOL success;
	long downloadsize = 0;

	if( AFXDIALOG->shuttingDown == TRUE )
	{
		OutDebugs( "Skipping Downloading of text during shutdown..." );
		// bugger we are shutting down, DONT DOWNLOAD NOW....
		return 0;
	}

	// default the server to the normal config
	if( !server || (server && server.IsEmpty()) )
		server = CFG->cfgIPandPort;

	if( myInternetSession == NULL )
		myInternetSession = new CInternetSession(	"Display Agent",
                            	                    1,
													INTERNET_OPEN_TYPE_PRECONFIG,
													//INTERNET_OPEN_TYPE_PROXY,
													NULL,
                            	                    NULL,
                            	                    INTERNET_FLAG_DONT_CACHE );
	CHttpConnection	*myHTTPConnection = NULL;
    CHttpFile		*httpFile=NULL;
	CString			fileToGet = remoteFile;

	{

		// ---------------------------
		static LPCTSTR szAcceptType=__HTTP_ACCEPT_TYPE;
		static LPCTSTR szContentType="Content-Type: application/x-www-form-urlencoded\r\n";

		OutDebugs( "POST ACTION , url='%s%s'", server.GetBuffer(0), fileToGet.GetBuffer(0) );

		long port;
		server = URLGetPort( server, &port );

		myHTTPConnection = myInternetSession->GetHttpConnection(server, (INTERNET_PORT)port, NULL, NULL);
		httpFile = myHTTPConnection->OpenRequest( "POST",fileToGet,NULL,0,NULL,NULL,0 );
		httpFile->AddRequestHeaders( __HTTP_ACCEPT, HTTP_ADDREQ_FLAG_REPLACE, strlen(__HTTP_ACCEPT) );

		try /* Test to see if the connection is up */
		{
			success = httpFile->SendRequest( szContentType, strlen(szContentType), (LPVOID)postArgs.GetBuffer(0), postArgs.GetLength() );
		}
		catch (CInternetException* pEx)
		{
			OutDebugs( "ERROR: ServerPostAction has no connection, remotefile = %s%s", server.GetBuffer(0), remoteFile.GetBuffer(0) );
			pEx->Delete();
			noConnection = true;
		}
	}


    
    /* If the connection is not up don't download the file */
    if ((noConnection == false) && (success == TRUE))
    {
        CString buff;
		CStdioFile localSavedFile; 

		currentDownloadFile = server + fileToGet;

		char buffMemory[1000];
		long dataToGet = (long)httpFile->GetLength(), statusCode=200;

		dataToGet = 0;
		long len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH , buffMemory, (LPDWORD)&len, NULL ) )
			dataToGet = atoi(buffMemory);

		len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_STATUS_CODE , buffMemory, (LPDWORD)&len, NULL ) )
			statusCode = atoi(buffMemory);

		if( (statusCode >= 404 && statusCode < 500)|| dataToGet == 1 )
		{
			OutDebugs( "ERROR: Statuscode=%d , Remote file %s%s does not exist, skipping download", 
				statusCode, server.GetBuffer(0), fileToGet.GetBuffer(0) );
		}
		else
		{
			CString dataResult,slideshowDemo;

			long t1 = (timeGetTime());

			CString tempFile = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + "\\post_result.html";
			CStdioFile localSavedFile;

			// save slideshow to harddisk
			if( localSavedFile.Open(tempFile, CFile::modeCreate | CFile::modeWrite) )
			{
				// READ XML FILE FROM SERVER
				while( httpFile->ReadString(buff)==1 && AFXDIALOG->shuttingDown==FALSE )
				{
					dataResult.Append( buff );
					localSavedFile.WriteString( buff );
					localSavedFile.WriteString( "\n" );
					downloadsize += buff.GetLength();
				}
				localSavedFile.Close();

				t1 = (timeGetTime()-t1);
				if( t1 )
					OutDebugs( "HTTP Result = %s", dataResult );
			} else {
				CString msg;
				msg.Format( "Downloader cannot write to '%s' , reason is %s", tempFile, GetLastErrorString() );
				Log_App_FileError( errMediaMissing, tempFile, msg.GetBuffer(0) );
			}
		}
    }
    
	if( httpFile )
	{
		httpFile->Close();
		delete httpFile;
	}
	if( myHTTPConnection )
	{
		myHTTPConnection->Close(); 		
	    delete myHTTPConnection;
	}

	return downloadsize;
}




//
//
//
//
//
//
// ############################################
// Using the impressions (ie slideshow) information
// download all missing media data, images/video if they do not exist
//
//
//
//
//
long CWidgieXML::DownloadMissingContent( long *pfilesTotal, long *pfilesDone ) 
{
	long downloadsize = 0;

	CString lpPathName = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "\0";
	
    BOOL contentDirCreateResult;
	contentDirCreateResult = VerifyDestinationDir(lpPathName);

	PrintDebugText( "Downloading missing images in loop sequence..." );

	m_downloadMediaCount++;

	// ### LOGIN into MDID to get access to its data if we have a server defined.
	if( CFG->cfgIPandPort.IsEmpty() == false )
	{
		if( m_sessionToken.IsEmpty() ||
			(CTime::GetCurrentTime() - m_sessionStartTime)>(59*60) )
		{
			MDID_Login( CFG->cfgIPandPort, URL_MDID_LOGIN, "" );
		}
	}


	long l_filesTotal, l_filesDone;

	if( !pfilesTotal )
		pfilesTotal = &l_filesTotal;

	if( !pfilesDone )
		pfilesDone = &l_filesDone;

	*pfilesTotal = 0;
	*pfilesDone = 0;

	int imageCount = 0;
	POSITION newPos;
	ImpList *impList = GetImpressionList();

	// go through all impressions, and check to see if any files are missing, if so , then download them
	while( newPos = impList->FindIndex(imageCount++) )
	{
	    ImpressionData nextImpression = impList->GetAt(newPos);
		CString localFilename;
		struct stat sbuf;

		// #################################################
		// DOWNLOAD NEW IMAGES
		// These are downloaded via the webservice using dynamic server calls
		if( nextImpression.m_image_file.IsEmpty() == FALSE )
		{
			localFilename = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + nextImpression.m_image_file;
			// filename would be stored as "1_34.jpg"  where 1=cid, 34=id

			if( nextImpression.m_image_file.Find( "http://" ) == 0 )
			{
				localFilename.Format( "%s%08lx.jpg", CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir, CHashIt(nextImpression.m_image_file) );
				// download the remote http data file...
				{
					OutDebugs( "Downloading file - %s", nextImpression.m_image_file );
					if( downloadsize+=DownloadBin(CFG->cfgIPandPort, nextImpression.m_image_file, localFilename.GetBuffer(0), "" ) == 0 )
					{
						PrintDebugText( "\n\nFAILED - file does not exist on server" );
						Sleep( 500 );
					}
				}
			} else
			// only if the local file doesnt exist download a new remote file.
			if( (CFG->cfgImagesDownloadAlways==FALSE && stat( localFilename.GetBuffer(0), &sbuf )) ||
				CFG->cfgImagesDownloadAlways )		// else always try to download overwriting local files
			{
				CString txt = "\nDownloading image - ";
				txt += nextImpression.m_image_file;
				PrintDebugText( txt.GetBuffer(0) );

				// recalculate the correct ID/CID from the filename "1_25.jpg"
				// if for some reason we just dont have the data, just in case....
				if( nextImpression.m_image_id.IsEmpty() ) 
				{
					int first_ = nextImpression.m_image_file.Find( '_', 0);
					if( first_ >= 0 )
					{
						nextImpression.m_image_cid = nextImpression.m_image_file.Left( first_ );
						int dot_ = nextImpression.m_image_file.Find( '.', 0);
						nextImpression.m_image_id = nextImpression.m_image_file.Mid( first_+1, dot_-(first_+1) );
					}
				}
				
				CString param;
				param.Append( "?id=" );
				param.Append( nextImpression.m_image_id );
				param.Append( "&cid=" );
				param.Append( nextImpression.m_image_cid );
				param.Append( "&format=F" );
				param.Append( "&token=" );
				param.Append( m_sessionToken );			// use the login token
				
				(*pfilesTotal)++;

				int dlsize;
				dlsize = DownloadBin( CFG->cfgIPandPort, CFG->cfgGetImageFileName, localFilename.GetBuffer(0), param );
				if( dlsize>0 && dlsize < 8000 )
				{
					PrintDebugText( "\n\nFAILED - file is only an error msg file" );
					DeleteFile ( localFilename );
				} else
				if( dlsize>0 )
				{
					downloadsize += dlsize;
					(*pfilesDone)++;
				} else
				if( dlsize<=0 )
				{
					PrintDebugText( "\n\nFAILED - file does not exist on server" );
				}
			}
		}
		// ##########################################################
		// download video file if defined in the slideshow item
		// these are downloaded directly through the server via static files.
		if( nextImpression.m_multimedia_file.IsEmpty() == FALSE )
		{
			//==== Download video file
			localFilename = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + nextImpression.m_multimedia_file;

			// if the local video file doesnt not exist, download it now.
			if( (CFG->cfgVideoDownloadAlways==FALSE && stat( localFilename.GetBuffer(0), &sbuf )) ||
				 CFG->cfgVideoDownloadAlways		// else always try to download overwriting local files
				 )
			{
				CString txt = "\nDownloading video - ";
				txt += nextImpression.m_multimedia_file;
				PrintDebugText( txt.GetBuffer(0) );

				CString url = CFG->cfgServerVideoPath + nextImpression.m_multimedia_file;

				(*pfilesTotal)++;

				long dlsize = DownloadBin( CFG->cfgIPandPort, url, localFilename.GetBuffer(0) );
				if( dlsize > 0 )
				{
					downloadsize += dlsize;
					(*pfilesDone)++;
				} else
				if( dlsize < 0 )
				{
					PrintDebugText( "\n\nFAILED - file does not exist on server" );
					Sleep( 100 );
				}
			}
		}
	}

	return downloadsize;
}



// ####################################   END MDID SUPPORT CODE   ##################################






























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

	long downloadsize = 0, totalsize = 0;

	if( type.CompareNoCase( "splashlogo" )==0 )
	{
		// main logo
		if( CFG->cfgMainLogoFileName.IsEmpty() == FALSE )
		{
			CString filename = CFG->cfgLocalBaseDir + CFG->cfgLocalImagesDir + CFG->cfgMainLogoFileName;
			if( FileExists( filename ) == FALSE )
			{
				downloadsize =
				DownloadBin( CFG->cfgIPandPort, CFG->cfgServerContentPath + CFG->cfgMainLogoFileName,
							 filename );
				if( downloadsize >0 )
				{
					totalsize += downloadsize;
				}
			}
		}
	} else
	if( type.CompareNoCase( "newslogo" )==0 )
	{
		// news corner logo
		if( CFG->cfgNewsLogoFileName.IsEmpty() == FALSE )
		{
			CString filename = CFG->cfgLocalBaseDir + CFG->cfgLocalImagesDir + CFG->cfgNewsLogoFileName;
			if( FileExists( filename ) == FALSE )
			{
				downloadsize =
				DownloadBin( CFG->cfgIPandPort, CFG->cfgServerContentPath + CFG->cfgNewsLogoFileName, 
							 filename );
				if( downloadsize >0 )
				{
					totalsize += downloadsize;
					TouchFile( filename );
				}
			}
		}
	} else
	if( type.CompareNoCase( "scrolllogo" )==0 )
	{
		// news scroll bar seperator
		if( CFG->cfgScrollLogoFileName.IsEmpty() == FALSE )
		{
			CString filename = CFG->cfgLocalBaseDir + CFG->cfgLocalImagesDir + CFG->cfgScrollLogoFileName;
			if( FileExists( filename ) == FALSE )
			{
				downloadsize =
				DownloadBin( CFG->cfgIPandPort, CFG->cfgServerContentPath + CFG->cfgScrollLogoFileName,
							 filename );
				if( downloadsize >0 )
				{
					totalsize += downloadsize;
					TouchFile( filename );
				}
			}
		}
	}


	if( totalsize>0 )
	{
		CString msg;
		msg.Format( "Downloaded Support Images, %d bytes in %.1f seconds", downloadsize, (timeGetTime()-t_beg)/1000.0 );
		Log_App_Event( 0 , msg );
	}


	return totalsize;
}


// this will download a new exe and store it as Widgie.exe, but also mark a 0 byte file for the version requirements ie Widgie_04.exe
long CWidgieXML::DownloadNewExecuteable() 
{
	DWORD t_beg = timeGetTime();

	long downloadsize = 0;

	CString exeFile = CFG->cfgUpdateExe;

	// news scroll bar seperator
	if( exeFile.IsEmpty() == FALSE )
	{
		CString newfilenum = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + exeFile;

		// if the version # doesnt exist, ie "content/Widgie_16.exe" then its ok to proceed.
		if( FileExists( newfilenum ) == FALSE )
		{
			CString newfile = CFG->cfgLocalBaseDir + CFG->cfgLocalContentDir + "Widgie.exe";

			// delete the previous/old exe that was downloaded.
			DeleteFile( newfile.GetBuffer(0) );

			downloadsize +=
			DownloadBin( CFG->cfgIPandPort, CFG->cfgServerContentPath + exeFile, 	newfile );

			// on success
			if( downloadsize > 32000 )
			{
				TouchFile( newfilenum );
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

	if( InternetAttemptConnect( 0 ) == ERROR_SUCCESS )
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

		long port;

		CString	server = URLGetPort( CFG->cfgIPandPort, &port );

		if( server.IsEmpty() == false )
		{
			CHttpConnection* myHTTPConnection = myInternetSession->GetHttpConnection( server, (INTERNET_PORT)port, NULL, NULL);

			if( myHTTPConnection )
			{
				CHttpFile *httpFile=NULL;

				httpFile = myHTTPConnection->OpenRequest("","/index.html",NULL,0,NULL,NULL,0);

				bool noConnection = false;
				BOOL success;

				try /* Test to see if the connection is up */
				{
					success = httpFile->SendRequest();
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

				httpFile->Close();
				delete httpFile;

				myHTTPConnection->Close(); 		
				delete myHTTPConnection;
			}
		}
	}

	return foundStatus;
}




int globalCheckNetConnection(void);
int globalCheckNetAdaptors( CString *netip, CString *netname );


enum {
	DOWNLOADERROR_NOFILE	= 0,
	DOWNLOADERROR_SAME		= -1,
	DOWNLOADERROR_NOSERVER  = -2
};


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
 *                      e.g. http://www2.blahserver.com:80/get_loopsequence.asp
 *
 *  @param  fileToWrite The name of the file to write to disk
 *                      The file name must inlude the full path where 
 *                      the file is to be written
 *                      e.g. c:\content\loopsequence.xml
 *
 *  @param  parameters  CGI extra parameters to add, can be null
 *
 *  @lastDownloadDAte   Optionally pass last files date to match
 *						against the servers date, if they are equal then do not download the file
 *						and skip it like normal, returning 0
 *   
 *
 *
 *
 *
 */
long CWidgieXML::DownloadText( CString server, 
							   CString remoteFile, 
							   CString fileToWrite, 
							   CString parameters, 
							   CString *lastDownloadDate )
{
	// ---------------- setup
	CWidgieApp *cfg = CFG;
	long	downloadsize = 0;

	if( AFXDIALOG->shuttingDown == TRUE )
	{
		OutDebugs( "Skipping Downloading of text during shutdown..." );
		// bugger we are shutting down, DONT DOWNLOAD NOW....
		return 0;
	}

	// default the server to the normal config
	if( !server || (server && server.IsEmpty()) )
		server = cfg->cfgIPandPort;

	// try the local proxy server first if it exists and if we arent already trying it in this call ...
	if( cfg->cfgLocalServer.GetLength() >0 && 
		server.CompareNoCase(cfg->cfgLocalServer) &&		// avoid infinite recursion
		cfg->cfgLocalServerReachable )
	{
		downloadsize = DownloadText( cfg->cfgLocalServer, remoteFile, fileToWrite, parameters, lastDownloadDate );
		if( downloadsize >0 )
			return downloadsize;
	}


	// -------- ok lets start downloading...................

	// TODO: Add your control notification handler code here
	if( myInternetSession == NULL )
		myInternetSession = new CInternetSession(	NULL,
                            	                    1,
                            	                    INTERNET_OPEN_TYPE_PRECONFIG,
                            	                    NULL,
                            	                    NULL,
                            	                    INTERNET_FLAG_DONT_CACHE );		//INTERNET_FLAG_DONT_CACHE
	CHttpConnection* myHTTPConnection = NULL;
    CHttpFile *httpFile=NULL;
	CString fileToGet = remoteFile;

	long port;

	// if we have a full real url, rather than a filename, extract details, and connect as usual...
	if( fileToGet.Find( "http://", 0 ) == 0 )
	{
		char domain[256], username[32], passwd[32], file[256];
		ExtractUserFromURL( fileToGet.GetBuffer(0), domain, username, passwd,file );

		server = URLGetPort( domain, &port );

		OutDebugs( "Downloading Text from URL '%s%s'", domain, file );

		myHTTPConnection = myInternetSession->GetHttpConnection( server, (INTERNET_PORT)port, NULL, NULL );
		httpFile = myHTTPConnection->OpenRequest("",file,NULL,0,NULL,NULL,0);
	} else 
	{
		// add the ID of the client to login to MDID;
		if( parameters && !parameters.IsEmpty() )
		{
			fileToGet += parameters;
		}
		// ---------------------------
		OutDebugs( "Downloading Text from '%s%s'", server.GetBuffer(0), fileToGet.GetBuffer(0) );

		server = URLGetPort( server, &port );

		myHTTPConnection = myInternetSession->GetHttpConnection(server, (INTERNET_PORT)port, NULL, NULL);
		httpFile = myHTTPConnection->OpenRequest("",fileToGet,NULL,0,NULL,NULL,0);
	}

	bool noConnection = false;
    BOOL success;

    try /* Test to see if the connection is up */
    {
        success = httpFile->SendRequest();
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
        CString buff, fileDate;

		currentDownloadFile = server + fileToGet;

		char buffMemory[1000];
		long dataToGet = (long)httpFile->GetLength(), statusCode=200;

		long len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH , buffMemory, (LPDWORD)&len, NULL ) )
			dataToGet = atoi(buffMemory);

		len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_STATUS_CODE , buffMemory, (LPDWORD)&len, NULL ) )
			statusCode = atoi(buffMemory);

		len = 32;
		if( httpFile->QueryInfo( HTTP_QUERY_LAST_MODIFIED , buffMemory, (LPDWORD)&len, NULL ) )
			fileDate = buffMemory;

		if( statusCode >= 404 || dataToGet == 1 )
		{
			OutDebugs( "ERROR: Statuscode=%d , Remote file %s%s does not exist, skipping download", statusCode, server.GetBuffer(0), fileToGet.GetBuffer(0) );
			currentDownloadFile.Append( " - Failed, doesnt exist" );
		}
		else
 		if( lastDownloadDate && !fileDate.IsEmpty() && fileDate.Compare( lastDownloadDate->GetBuffer(0) )==0 )
		{
			OutDebugs( "Remote file has the same date '%s', skip download ", fileDate.GetBuffer(0) );
			downloadsize = -1;
		}
		else
		// Ok its safe to download the file now...
		{
			CString tempFile = fileToWrite + ".tmp";
			CStdioFile localSavedFile;

			// store the files date for future reference
			if( lastDownloadDate )
			{
				lastDownloadDate->SetString( fileDate );
			}

			if( localSavedFile.Open(tempFile, CFile::modeCreate | CFile::modeWrite) )
			{
				long t1 = (timeGetTime());
				long downloadSum = 0;

				OutDebugs( "Saving to : %s (%d size)", tempFile.GetBuffer(0), dataToGet );

				while( AFXDIALOG->shuttingDown==FALSE && httpFile->ReadString(buff) )
				{
					downloadSum = HashIt( buff.GetBuffer(0), buff.GetLength(), downloadSum );			// compute hash, but we ignore all LFs
					buff.Append( "\n" );
					downloadsize += (buff.GetLength());
					downloadsize++;
					m_currentDownloaded = downloadsize;
					/* Write it to file */
					localSavedFile.WriteString(buff);
				}

				localSavedFile.Close();

				t1 = (timeGetTime()-t1);
				if( t1 )
					OutDebugs( "Download Completed in %.1f seconds, %d bytes (%d Kbps)", t1/1000.0, downloadsize, (8*downloadsize/t1) );

				// did we download something?
				if( downloadsize >0 )
				{
					// get the old files sum
					long fileSum = CHashFile( fileToWrite );			// computer hash for file, but ignoring all LFs

					// if the downloaded file is really different... then use it
					// otherwise, just dump it and not use it.
					if( downloadSum != fileSum )
					{
						myDeleteFile( fileToWrite );		// delete original
						MoveFile( tempFile.GetBuffer(0), fileToWrite.GetBuffer(0) );		// rename temp to original
					} else 
					{
						OutDebugs( "Download file checksums are the same, no data change" );
						downloadsize = -2;
					}
				}

			} else {
				CString msg;
				msg.Format( "Downloader cannot write to file - %s", GetLastErrorString() );
				Log_App_FileError( errMediaMissing, tempFile, msg.GetBuffer(0) );
			}
		}
    }

	if( httpFile )
	{
		httpFile->Close();
		delete httpFile;
	}
	if( myHTTPConnection )
	{
		myHTTPConnection->Close(); 		
	    delete myHTTPConnection;
	}

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
		downloadsize = DownloadBin( CFG->cfgIPandPort, remoteFile, fileToWrite );
	}

	return downloadsize;
}



//
// Download a binary file from the server.
//
//
//
//
//
long CWidgieXML::DownloadBin( CString server, CString remoteFile, CString fileToWrite, CString parameters, CString *lastDownloadDate )
{
	// ----------- setup
	CWidgieApp *cfg = CFG;
	long	downloadsize = 0, 
			localFileSize = 0;
	/* open a socket to the iTV server and download the newsflashes */
	m_currentDownloaded = 0;

	// ---- make sure we abort if quiting
	if( AFXDIALOG->shuttingDown == TRUE )
	{
		OutDebugs( "Skipping Downloading of text during shutdown..." );
		// bugger we are shutting down, DONT DOWNLOAD NOW....
		return 0;
	}

	// default the server to the normal config
	if( !server || (server && server.IsEmpty()) )
		server = cfg->cfgIPandPort;

	// try the local proxy server first if it exists...
	if( cfg->cfgLocalServer.GetLength() >0 && 
		server.CompareNoCase(cfg->cfgLocalServer) &&
		cfg->cfgLocalServerReachable )
	{
		downloadsize = DownloadBin( cfg->cfgLocalServer, remoteFile, fileToWrite, parameters, lastDownloadDate );
		if ( downloadsize >0 )
			return downloadsize;
	}


	// ----------------- ok lets go on with it.....


	CFile localBinFile;
	localFileSize = fileSize( fileToWrite.GetBuffer(0) );


	if( remoteFile && remoteFile.IsEmpty() == FALSE )
	{
		CString fileToGet;

		if( remoteFile.Find( "http://", 0 ) == 0 )
		{
			char host[256], username[32], passwd[32], file[256];
			ExtractUserFromURL( remoteFile.GetBuffer(0), host, username, passwd,file );
			server = host;
			fileToGet = file;
		} else
		{
			fileToGet = remoteFile;
		}

		if( server.IsEmpty() == true )
		{
			return -2;
		}

		// TODO: Add your control notification handler code here
		if( myInternetSession == NULL )
			myInternetSession = new CInternetSession(	NULL,
                            							1,
                            							INTERNET_OPEN_TYPE_PRECONFIG,
                            							NULL,
                            							NULL,
                            							INTERNET_FLAG_DONT_CACHE);
	
		long port;
		server = URLGetPort( server, &port );

		CHttpConnection* myHTTPConnection = myInternetSession->GetHttpConnection(server, (INTERNET_PORT)port, NULL, NULL);

		if( parameters && !parameters.IsEmpty() )
		{
			fileToGet += parameters;
		}

		OutDebugs( "DOWNLOADING BINARY : '%s%s'", server.GetBuffer(0), fileToGet.GetBuffer(0) );

		CHttpFile *httpFile = myHTTPConnection->OpenRequest("",fileToGet,NULL,0,NULL,NULL,0);

		//INTERNET_OPTION_RECEIVE_TIMEOUT 
		//Sets or retrieves an unsigned long integer value that contains the time-out value, in milliseconds, to receive a 
		// response to a request. If the response takes longer than this time-out value, the request is canceled. This option
		// can be used on any HINTERNET handle, including a NULL handle. It is used by InternetQueryOption and InternetSetOption. 
		myHTTPConnection->SetOption( INTERNET_OPTION_RECEIVE_TIMEOUT, 300000, 0 );

        bool noConnection = false;
        
        try /* Test to see if the connection is up */
        {
            httpFile->SendRequest();
        }
        catch (CInternetException* pEx)
	    {
            pEx->Delete();
		    noConnection = true;
        }
        
        /* If the connection is not up don't download the file */
        if (noConnection == false)
        {
            char buffMemory[1000];
			CString	fileDate;

			currentDownloadFile = server + fileToGet;

			// Lets fine out if the file is already on the local disc...
			long statusCode = 200;
			long dataToGet, len;

			len = 32;
			if( httpFile->QueryInfo( HTTP_QUERY_CONTENT_LENGTH , buffMemory, (LPDWORD)&len, NULL ) )
				dataToGet = atoi(buffMemory);

			len = 32;
			if( httpFile->QueryInfo( HTTP_QUERY_STATUS_CODE , buffMemory, (LPDWORD)&len, NULL ) )
				statusCode = atoi(buffMemory);

			len = 32;
			if( httpFile->QueryInfo( HTTP_QUERY_LAST_MODIFIED , buffMemory, (LPDWORD)&len, NULL ) )
				fileDate = buffMemory;
			
			OutDebugs( "Remote file length = %ld, statusCode = %d, lastmod = %s", dataToGet ,statusCode, fileDate.GetBuffer(0) );

			if( statusCode >= 400 )
			{
				OutDebugs( "ERROR: Statuscode=%d , Remote file %s%s does not exist, skipping download", statusCode, server.GetBuffer(0), fileToGet.GetBuffer(0) );
				currentDownloadFile.Append( " - Failed, doesnt exist" );
				downloadsize = -1;
			}
			else
			if( localFileSize>0 && localFileSize == dataToGet )
			{
				//Log_App_FileError( errMediaMissing, fileToWrite, "File already exists of the same size or less" );
				OutDebugs( "File %s already exists of the same size or less (%d), download aborted", fileToWrite.GetBuffer(0), localFileSize );
				currentDownloadFile.Append( " - same size" );
				downloadsize = 0;
			} else
			if( lastDownloadDate && !fileDate.IsEmpty() && (*lastDownloadDate == fileDate) )
			{
				OutDebugs( "Remote file has the same date '%s', skip download", fileDate.GetBuffer(0) );
				currentDownloadFile.Append( " - same date" );
				downloadsize = 0;
			}
			else
			{
				long t1 = timeGetTime();
				CString tempFile = fileToWrite + ".tmp";
				m_currentDownloaded = 0;

			    CFile localBinFile;

				// store the files date for future reference
				if( lastDownloadDate )
				{
					lastDownloadDate->SetString( fileDate );
				}

				// download file into tempFile, then delete original, rename temp to file
				if(	localBinFile.Open( tempFile, CFile::modeCreate | CFile::modeWrite) )
				{
					OutDebugs( "Downloading now..." );

					int buffSize = sizeof(buffMemory);

					int newDataSize = 1;

					try
					{
						CFileStatus status;
						char* buff;
						buff = &buffMemory[0];

						// read 1000 bytes at a time from server and save to local file....
						while(	newDataSize>0 && 
								httpFile && 
								AFXDIALOG->shuttingDown==FALSE )
						{
							newDataSize = httpFile->Read(buff, 256);
							if( newDataSize>0 )
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

					if( downloadsize && (downloadsize == dataToGet || statusCode==200) )
					{
						myDeleteFile( fileToWrite );
						MoveFile( tempFile.GetBuffer(0), fileToWrite.GetBuffer(0) );

						if( downloadsize && t1>0 )
						{
							OutDebugs( "Download Completed in %.1f seconds, %d bytes (%d Kbps)", t1/1000.0, downloadsize, (8*downloadsize/t1) );
						}

						// Now since we downloaded a new jpeg, lets delete the old BMP so as not to have the wrong image
						if( downloadsize && fileToWrite.Find( ".jpg" )>0 )
						{
							if( fileToWrite.Replace( ".jpg", ".bmp" )>0 )
							{
								if( FileExists( fileToWrite.GetBuffer(0) ) )
								{
									myDeleteFile( fileToWrite );
									OutDebugs( "Deleted old file %s", fileToWrite.GetBuffer(0) );
								}
							}
						}
					} else 
					{
						// download incomplete, delete file and return 0
						OutDebugs( "Download incomplete, %d out of %d done, deleting file now", downloadsize, dataToGet );
						myDeleteFile( tempFile );
						downloadsize = 0;
					}
				} else 
				{
					CString msg;
					msg.Format( "Cannot write to file - %s", GetLastErrorString() );
					Log_App_FileError( errMediaMissing, tempFile, msg.GetBuffer(0) );
				}
			}

			if( httpFile )
			{
				httpFile->Close();
				delete httpFile;
			}
        } else
		{
			OutDebugs( "Cannot make net connection" );
		}

		if( myHTTPConnection )
		{
			myHTTPConnection->Close(); 
			delete myHTTPConnection;
		}

		//myInternetSession->Close();
		//delete myInternetSession;

		m_currentDownloaded = 0;

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
		// first we must try "slideshow_Display_####.xml", if that fails open "slideshow.xml"
		CString defaultLoopFN = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName;
		CString currentLoopFN = defaultLoopFN;
		currentLoopFN.Replace( ".xml", "_" );
		currentLoopFN.AppendFormat( "%s.xml", CFG->cfgID );

		// if there are no files, download one.
		if( !FileExists(currentLoopFN) && !FileExists(defaultLoopFN) )
		{
			DownloadXMLLoop();
		}

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
		delete defaultHandler;		defaultHandler = NULL;
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
	
	delete defaultHandler;	defaultHandler = NULL;

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




/*
This will import the telentice style loop sequence...
*/
bool CWidgieXML::ImportTelenticeMPL( void )
{
	ImpList *impList = GetImpressionList();

	LoopSeqXML_Handler* ImpHandler = new LoopSeqXML_Handler(&m_OutputEdit, impList, NULL, "" );

	// src file name
	CString mplFN = CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalLoopName;
	if( FileExists( mplFN ) )		// yes it exists
	{
		CStdioFile localFile;
		// open the file.
		BOOL result = localFile.Open( mplFN, CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate );
		if( result )
		{
			CString line;
			// read each line of text.
			while( localFile.ReadString(line)==1 )
			{
				ImpressionData *ImpPtr = ImpHandler->GetCurrentImpressionPtr();
				// init a default item
				ImpHandler->InitCurrentImpression(0);	

				// if its QUOTED, then use  the value inside the quotes for the media file.
				if( line[0] == '\"' )
				{
					CString file;
					int endpos = line.Find( "\"", 1);
					file = line.Mid( 1, endpos );
					ImpPtr->m_multimedia_file = file;
					CString tmp = line.Mid( endpos+2 );
					ImpPtr->m_playlength = StrToInt(tmp);
					ImpHandler->InsertSorted();
				}
			}
			localFile.Close();
		}
	}

	delete ImpHandler;


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
			DownloadText( CFG->cfgIPandPort, CFG->cfgConfigFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + DEFURL_CONFIG );
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
			DownloadText( CFG->cfgIPandPort, CFG->cfghttpFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalNewsName);
		}
#endif
		OutDebugs( "Parsing RSS NEWS XML file %s", newsPath.GetBuffer(0) );
		parser->parse( newsPath.GetBuffer(0) );

		if( CFG->cfgUseRSSNews )
		{
			// parse all our downloaded news rss feeds.
			for( int i=0; i< CFG->cfgNewsRSSFeeds.GetCount();i++)
			{
				newsPath.Format( "%s\\news%d.rss", CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir, i );
				OutDebugs( "Parsing RSS NEWS XML file %s", newsPath.GetBuffer(0) );
				parser->parse( newsPath.GetBuffer(0) );
			}
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
 *  file as described int the  Series 2 Development Manual.
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
			DownloadText( CFG->cfgIPandPort, CFG->cfgManifestFileName, CFG->cfgLocalBaseDir + CFG->cfgLocalXML_Dir + CFG->cfgLocalManifestName);
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










//
// Upload event log via a POST using http
//
// ASP Web server script will read the values and
// add them to the database.
//
//
int CWidgieXML::PostEventLog( CList<CString,CString> *pEventCList )
{
	int result = 0;

	if( pEventCList )
	{
		CString mainURL, parameters;

		OutDebugs( "STATUS: Updating stats to server (%d events)", pEventCList->GetCount() );

		mainURL = "/submitstats.asp";

		parameters.AppendFormat( "displayID=%s", CFG->cfgID );

		parameters.AppendFormat( "&data=" );

		for( POSITION eventpos = pEventCList->GetHeadPosition(); eventpos != NULL; )
		{
			CString event;
			event = pEventCList->GetNext(eventpos);

			event.Replace( " ", "%20" );

			parameters.AppendFormat( "%s,", event );
		}

		parameters.TrimRight( "," );

		result =
		ServerPostAction( CFG->cfgOriginalServer, mainURL, parameters );
	}


	return result;
}







