#include <afx.h>
#include <afxwin.h>
#include <afxmt.h>      // for synchronization objects
#include <afxext.h>
#include <afxisapi.h>
#include <list>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <math.h>
#include <float.h>

#include "stdafx.h"
#include "history.h"
#include "utils.h"
#include "Widgie.h"





BOOL Log_App_FileCat( char *filename, char *txt )
{
	CFile localFile;
	BOOL result;

	result = localFile.Open( filename, CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate );

	if( !result )
	{
		CreateDirectory( "Logs" , NULL );
		result = localFile.Open( filename, CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate );
	}

	if( result && txt )
	{
		localFile.SeekToEnd();
		localFile.Write( txt, strlen(txt) );
		localFile.Close();
		return TRUE;
	}
	else
		return FALSE;
}



BOOL Log_App_FileWrite( char *filename, char *txt )
{
	if( CFG->cfgEnableLogs )
	{
		CFile localFile;
		BOOL result;

		result = localFile.Open( filename, CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate );

		if( !result )
		{
			CreateDirectory( "Logs" , NULL );
			result = localFile.Open( filename, CFile::modeCreate|CFile::modeReadWrite|CFile::modeNoTruncate );
		}

		if( result && txt )
		{
			localFile.SeekToEnd();
			localFile.Write( txt, strlen(txt) );
			localFile.Write( "\r\n", 2 );
			localFile.Close();
			return TRUE;
		}
	} else
		OutputDebugString( txt );

	return FALSE;
}


// log any details to any filename.... more for generic status logs
void Log_App_Status( CString filename, int impressionID, CString eventMessage)
{
    CTime currentTime = CTime::GetCurrentTime();

    CString dateString = currentTime.Format("%Y-%m-%d");
    CString timeString = currentTime.Format("%H:%M:%S");

	char *fName;

	if( !filename || (filename && filename.GetLength()==0) )
		fName = FN_OPERATIONS;
	else
		fName = filename.GetBuffer(0);

	CString txt;

	txt.Format( "%s %s - [id=%d] %s", dateString.GetBuffer(0), timeString.GetBuffer(0), impressionID, eventMessage );
	Log_App_FileWrite( fName, txt.GetBuffer(0) );

}

void Log_App_Event(int impressionID, CString eventMessage)
{
	Log_App_Status( FN_OPERATIONS, impressionID, eventMessage );
}




#include <shlwapi.h>


void Log_App_Error(char * details)
{
    CTime currentTime = CTime::GetCurrentTime();

    CString dateString = currentTime.Format("%Y-%m-%d");
    CString timeString = currentTime.Format("%H:%M:%S");

	CString txt;

	txt.Format( "%s %s - %s", dateString.GetBuffer(0), timeString.GetBuffer(0),  details );

	Log_App_FileWrite( FN_ERRORS, txt.GetBuffer(0) );
}


void Log_App_FileError(App_Error error, CString filename, CString details)
{
    /* This array must contain its elements in the same order 
       as the enumeration list in the header file history.h */
    CString Errors[] = {"bad xml", "bad media", "media missing", "no news", "no loop sequence"};

    CTime currentTime = CTime::GetCurrentTime();

    CString dateString = currentTime.Format("%Y-%m-%d");
    CString timeString = currentTime.Format("%H:%M:%S");

	CString txt;

	txt.Format( "%s %s - [%s] '%s' - %s", dateString.GetBuffer(0), timeString.GetBuffer(0), 
		Errors[error].GetBuffer(0), filename.GetBuffer(0), details.GetBuffer(0) );

	Log_App_FileWrite( FN_ERRORS, txt.GetBuffer(0) );
}


void Log_App_Debug( CString details )
{
    CTime currentTime = CTime::GetCurrentTime();

    CString dateString = currentTime.Format("%Y-%m-%d");
    CString timeString = currentTime.Format("%H:%M:%S");

	CString txt;

	txt.Format( "%s %s - %s", dateString.GetBuffer(0), timeString.GetBuffer(0), details.GetBuffer(0) );

	Log_App_FileWrite( FN_DEBUG, txt.GetBuffer(0) );
}
