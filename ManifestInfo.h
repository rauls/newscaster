// ManifestInfo.h: interface for the ManifestInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MANIFESTINFO_H__A99DDCD2_A45C_41F5_B9D1_801508D0832D__INCLUDED_)
#define AFX_MANIFESTINFO_H__A99DDCD2_A45C_41F5_B9D1_801508D0832D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef struct ManifestData     // structure to store strings and intergers 
{                               // retrieved from manifest xml file
	CString manifMethod;
	CString manifDateRequired;
	CString manifImageFile;
	CString manifImageFileType;
	CString manifMultimediaFile;
	CString manifMultimediaFileType;
	CString manifPrintFile;
	CString manifPrintFileType;


} tagManifestData;


class ManifestInfo  
{
public:
	ManifestInfo();
	virtual ~ManifestInfo();

};

#endif // !defined(AFX_MANIFESTINFO_H__A99DDCD2_A45C_41F5_B9D1_801508D0832D__INCLUDED_)
