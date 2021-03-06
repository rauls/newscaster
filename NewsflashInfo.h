// NewsflashInfo.h: interface for the NewsflashInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWSFLASHINFO_H__98829CF2_A347_41EA_BB75_7A3359E7ADDD__INCLUDED_)
#define AFX_NEWSFLASHINFO_H__98829CF2_A347_41EA_BB75_7A3359E7ADDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef struct NewsflashData    // structure to store strings and intergers 
{                               // retrieved from newsflash xml file
	CString text;
	int nYear;
	int nMonth;
	int nDay;
	int nHour;
	int nMin;
	int nSec;

} tagNewsflashData;

class NewsflashInfo  
{
public:
	
	NewsflashData data;         // instance of NewsflashData that is used to
                                // store XML information during parsing
    
    NewsflashInfo();                            // See C++ implementation
	virtual ~NewsflashInfo();                   // See C++ implementation

	bool Expired();                             // See C++ implementation
	NewsflashInfo operator = (NewsflashInfo infoToCopy); // See C++ implementation
    void SetDebug(CEdit* debugEdit);	        // See C++ implementation
	

	
private:
	CEdit* m_OutputEdit;                // reference to edit control to send debug text
	void Debug(CString debugMessage);   // See C++ implementation
};

#endif // !defined(AFX_NEWSFLASHINFO_H__98829CF2_A347_41EA_BB75_7A3359E7ADDD__INCLUDED_)
