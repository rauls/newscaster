// NewsflashInfo.cpp: implementation of the NewsflashInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewsflashInfo.h"
#include "utils.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/**
 *  NewsflashInfo Class
 *
 *
 *
 */


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/**
 *  NewsflashInfo constructor
 *
 *  does nothing
 *
 */

NewsflashInfo::NewsflashInfo()
{

}

/**
 *  NewsflashInfo destructor
 *
 *  does nothing
 *
 */

NewsflashInfo::~NewsflashInfo()
{

}


/**
 *  Gives the class a reference to an Edit box that can be
 *  used for debug output
 *
 *  @param debugEdit    A reference to an Edit box that it 
 *                      ready to receive dubg Strings
 *
 */

void NewsflashInfo::SetDebug(CEdit* debugEdit)
{
	m_OutputEdit = debugEdit;

}




/**
 *  Compares the time and date for the NewsflashData structure 
 *  to the current time
 *
 *  @return     True    If the time in the NewsflashData structure
 *                      is later that the current system time
 *
 *              False   If the time in the NewsflashData structure
 *                      is earlier that the current system time
 */

bool NewsflashInfo::Expired()
{
	bool expired = false;
	
	CTime currentTime = CTime::GetCurrentTime();
	
	CTime newsArchiveTime = CTime(this->data.nYear,
									   this->data.nMonth,
									   this->data.nDay,
									   this->data.nHour,
									   this->data.nMin,
									   this->data.nSec);
	
	if (newsArchiveTime < currentTime && newsArchiveTime.GetCurrentTime()>0 )
	{
		expired = true;
		//OutDebugs( "News has expired" );
	}	

	//delete newsArchiveTime;
	return(expired);
}

/**
 *  = operator
 *
 *  Used for asigning the dontents of one 
 *  NewsflashInfo Object to an other
 *
 */
NewsflashInfo NewsflashInfo::operator = (NewsflashInfo infoToCopy)
{
	return(infoToCopy);
}


/**
 *  Writes a debug string to a file or an Edit box
 *
 *  For speed purposes this code is #define out.
 *  
 *  Uncomment the appropriate #define section when debug
 *  is required
 */
void NewsflashInfo::Debug(CString debugMessage)
{
//#define DEBUG_TO_EDIT 
#ifdef DEBUG_TO_EDIT

    CString tempString;
	this->m_OutputEdit->GetWindowText(tempString);
	this->m_OutputEdit->SetWindowText(tempString + debugMessage + "\r\n");

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