// LoopSeqInfo.h: interface for the LoopSeqInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LoopSeqInfo_H__A8545475_A3A9_4ED2_AC99_C26407D4FA10__INCLUDED_)
#define AFX_LoopSeqInfo_H__A8545475_A3A9_4ED2_AC99_C26407D4FA10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef struct ImpressionData   // structure to store strings and intergers 
{                               // retrieved from loop sequence impression in 
	int m_id;                   // the loopo sequence XML file
	int m_hitCount;
	int m_viewCount;
	int m_order;
	int m_playlength;			// default to 3
	int m_volume;
	int m_brightness;

	time_t	m_time1;			// time range to call out at.
	time_t	m_time2;
	long	m_call_duration;

	CString m_language;

	CString	m_title;
	CString	m_image_id;			// MDID image ID
	CString	m_image_cid;		// MDID image collections ID

	CString m_image_file;		// image filename "beach.jpg" or just an ascii number of its ID "1_28.jpg"
	CString m_image_url;		// MDID image URL to use.
	CString m_image_file_type;	// jpeg 
	CString m_multimedia_file;	// optional video file.
	CString m_multimedia_file_type;	// we can figure this out
	CString m_url_params;
	CString	m_referal;

	CString m_scheduleStart;
	CString m_scheduleEnd;

	// ======== OBSELETE
	CString m_print_file;		// optional print file, obselete
	CString m_print_file_type;
	CString m_voice_number;
	CString m_voice_status;
	long	m_voice_duration;
	long	m_voice_days;		// bits 0 to 6 define which days of week to call at are ok.
	CString	m_voice_time1;		// time range to call out at.
	CString	m_voice_time2;
	// =================

	// image data would be stored locally per impression to improve speed, and be loaded and kept in ram.
	// only really needed on slow computers, ie 300mhz/5mbsHD
	void *imageData;
	time_t t_lastImageLoad;

} tagImpressioData;

typedef struct LoopDefaults     // structure to store strings and intergers 
{                               // retrieved from loop sequence defaults section
	CString		defaultLanguage;
	int 		defaultVolume;
	int 		defaultBrightness;
	CString 	defaultDialupNumber;
	CString 	defaultHTTP_Server;
	CString 	defaultFTP_Server;
	CString 	defaultReset;
	CString 	defaultNewConfig;

	int			defaultNewsBackground;
	CString		defaultNewsFont;
	int 		defaultTextColor;
	int 		defaultFontHeight;
	int 		defaultBarHeight;
	int 		defaultBarDelay;
	int 		defaultNewsInterval;
	int 		defaultScrollSpeed;
	int 		defaultStepSpeed;
	CString 	defaultContentPath;
	CString 	defaultNewsFilename;
	CString 	defaultManifestFilename;
	CString 	defaultLoopSequenceFilename;
    CString 	defaultGetXML_Time;
    int			defaultGetXML_Day;


} tagLoopDefaults;




class LoopSeqInfo 
{
public:
	LoopSeqInfo();
	virtual ~LoopSeqInfo();

	ImpressionData data;        // instance of ImpressionData that is used to
                                // store XML information during parsing
};

#endif // !defined(AFX_LoopSeqInfo_H__A8545475_A3A9_4ED2_AC99_C26407D4FA10__INCLUDED_)
