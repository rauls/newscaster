// LoopSeqXML_Handler.cpp: implementation of the LoopSeqXML_Handler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoopSeqXML_Handler.h"
#include "Widgie.h"
#include "WidgieDlg.h"
#include "utils.h"

#include <xercesc/sax2/Attributes.hpp>

#include <shlwapi.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/**
  * Constructor for LoopSequence Handler.
  *
  * Initialised the data mambers that will be used to collect
  * loop sequence information.
  *
  * Initialises the state machines that are used by the event
  * handlers that retrieve XML data.
  *
  * @param debugEdit        a pointer to an edit box used for debug
  *                         this can be NULL but only if #define DEBUG_TO_EDIT is
  *                         commented out in teh Debug function for this class
  *
  * @param impressionList   a pointer to a list to place all impression data that
  *                         is found during parsing
  *
  * @param defaults         a pointer to a structure used to store the data found
  *                         in the defaults section of of the loopsequence file.
  *
  */

LoopSeqXML_Handler::LoopSeqXML_Handler(CEdit* debugEdit,
                                       CList <ImpressionData, ImpressionData>* impressionList,
                                       CList <CString, CString>* LanguageList,
                                       CString language)
{
	m_OutputEdit = debugEdit;
	LoopXML_State = FINDING_IMPRESSION;
	XML_CharState = IMPRESSION_INVALID;

	if( impressionList )
	{
		pImpressionList = impressionList;

		startDateString = "2003-12-31";
		endDateString = "2012-12-31";

		InitCurrentImpression( impressionList->GetCount() );
		//InitLanguageList(LanguageList);

		//contentLanguage = language;

		//pFlagList = NULL;
	} else {
		//pFlagList = LanguageList;
		//pFlagList->RemoveAll();
	}
}


void LoopSeqXML_Handler::Reset_Handler(CEdit* debugEdit,
                                       CList <ImpressionData, ImpressionData>* impressionList,
                                       CList <CString, CString>* LanguageList,
                                       CString language)
{
	m_OutputEdit = debugEdit;
	LoopXML_State = FINDING_IMPRESSION;
	XML_CharState = IMPRESSION_INVALID;

	if( impressionList )
	{
		pImpressionList = impressionList;

		startDateString = "2003-12-31";
		endDateString = "2012-12-31";

		InitCurrentImpression(0);
//		InitLanguageList(LanguageList);
//		contentLanguage = language;
//		pFlagList = NULL;
	} else {
//		pFlagList = LanguageList;
//		pFlagList->RemoveAll();
	}

}




LoopSeqXML_Handler::~LoopSeqXML_Handler()
{
	OutDebugs( "LoopSeqXML_Handler::~LoopSeqXML_Handler()" );
}


/**
  * Event handler that is invoked when text is found between tags.
  *
  * A State machine that populates the current impression member
  * and the defaults structure with string text found 
  * between start and end tags i.e <tag>String Text</tag>
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class
  */


void LoopSeqXML_Handler::characters(const   XMLCh* const    chars,
								    const unsigned int    length)
{
	char* inText = XMLString::transcode(chars);
	CWidgieApp *cfg;

	cfg = CFG;
	//CString StrText = inText;
	if( inText && inText != "" && inText != "na" && inText != "NA" )

	switch (XML_CharState)
	{
        case START_DATE :
            startDateString = inText;
	        break;

        case END_DATE :
            endDateString = inText;
	        break;

        case IMPRESSION_ID :
			currentImpression.m_id = atoi(inText);
			break;

		case IMPRESSION_ORDER :
			currentImpression.m_order = atoi(inText);
			break;

		case IMPRESSION_PLAYLENGTH :
			if( inText)
			{
				if( char *p=strchr( inText, '=' ) )			// find "Duration=4"
				{
					p++;
					if( *p == ' ' )		// handle "Duration = 54"
						p++;
					cfg->cfgDefaultPlayLength =
					currentImpression.m_playlength = atoi(p);
				} else
				if( isdigit( *inText ) )
				{
					cfg->cfgDefaultPlayLength =
					currentImpression.m_playlength = atoi(inText);		// or use the value directly
				}
			}
			break;

		case IMPRESSION_DURATION :
			if( inText)
			{
				if( isdigit( *inText ) )
				{
					currentImpression.m_playlength = atoi(inText);		// or use the value directly
				}
			}
			break;
	
		case IMPRESSION_VOLUME :
			currentImpression.m_volume = atoi(inText);
			if( currentImpression.m_volume > 10 )
				currentImpression.m_volume = 10;
			break;

		case IMPRESSION_IMAGE :
			currentImpression.m_image_file = inText;
			break;

		case IMPRESSION_REFERAL :
			currentImpression.m_referal = inText;
			break;

		case IMPRESSION_IMAGEURL :			// MDID support for direct URL
			if( char *lf = strchr(inText,10) ) {
				*lf = 0;
			}
			currentImpression.m_image_url = inText;
			currentImpression.m_image_url.Replace( "&amp;", "&" );			// fix html codes
			break;

		case IMPRESSION_URLPARAMS :
			currentImpression.m_url_params = inText;
			break;

		case IMPRESSION_IMAGE_ID :
			currentImpression.m_image_id = inText;
			break;

		case IMPRESSION_IMAGE_CID :
			currentImpression.m_image_cid = inText;
			break;

		case IMPRESSION_MULTIMEDIA :
			if( char *lf = strchr(inText,10) ) {
				*lf = 0;
			}
			if( currentImpression.m_multimedia_file.GetLength() == 0 ) {
				currentImpression.m_multimedia_file = inText;
			}
			break;

		case IMPRESSION_FIELD:
			if( inText && *inText != 10 )
				currentImpression.m_multimedia_file = inText;
			break;

		case IMPRESSION_TITLE:
			if( inText && *inText != 10 )
				currentImpression.m_title = inText;
			break;
			

		// ---------- OBSELETE ---------

		case IMPRESSION_PRINT :
			currentImpression.m_print_file = inText;
			break;

		case IMPRESSION_VOICE :
			currentImpression.m_voice_number = inText;
			break;
		case IMPRESSION_VOICESTATUS:
			currentImpression.m_voice_status = inText;
			break;
		case IMPRESSION_VOICETIME1:
			{
				currentImpression.m_voice_time1 = inText;
				CTimeSpan t = TimeDateToCTimeSpan( currentImpression.m_voice_time1, 0 );
				currentImpression.m_time1 = (time_t)t.GetTotalSeconds();
			}
			break;
		case IMPRESSION_VOICETIME2:
			{
				currentImpression.m_voice_time2 = inText;
				CTimeSpan t = TimeDateToCTimeSpan( currentImpression.m_voice_time2, 0 );
				currentImpression.m_time2 = (time_t)t.GetTotalSeconds();
			}
			break;

		case IMPRESSION_VOICEDAYS:
			{
				char *p = inText;
				currentImpression.m_voice_days = 0;
				while( p && *p )
				{
					currentImpression.m_voice_days |= (1<< (atoi(p)+1) );
					p = strchr( p, ',' );
					if( p )
					{
						//skip comma
						p++;
						//skip space
						if( *p == ' ' )
							p++;
					}
				}
			}
			break;

		case IMPRESSION_VOICEDURATION:
			{
				int value = atoi(inText);
				if( value>0 )
					currentImpression.m_voice_duration = value;
			}
			break;

		case IMPRESSION_INVALID :
			break;

		case DEFAULT_CONFIGFILE:
			cfg->cfgConfigVersion = inText;
            WritePrivateProfileString("LOCAL MACHINE", "ConfigVersion", inText, INI_FILE);
			break;

		// ---------------------------------- HANDLE DEFAULT CONFIG SETTINGS ------------------------------------
		case DEFAULT_NAME :
			//cfg->cfgConfigNameLocation = inText;
			break;

		case DEFAULT_LANGUAGE :
			currentImpression.m_language = inText;
			break;
	
		case DEFAULT_HTTP_SERVER :
			// if we are located locally near the server, then we shouldnt
			// change to another one, but stick with what we have, only
			// change servers if we are 'outside'
			if( cfg->cfgServerIsLocal == FALSE )
			{
				cfg->cfgIPandPort = inText;
				if( cfg->cfgIPandPort == "null" )
				{
					cfg->cfgIPandPort.Empty();
				}
				OutDebugs( "CONFIG: Server address reset to : %s", inText );
			}
			break;
		
		case DEFAULT_LOCAL_SERVER :
			// <localserver>Display_112233=server1</>

			{
				CString element = inText;
				int pos = element.Find( '=' );
				if( pos>0 )
				{
					CString macid = element.Mid( 0, pos );
					if( macid.CompareNoCase( cfg->cfgID ) == 0 )
					{
						cfg->cfgLocalServer = element.Mid( pos+1, element.GetLength()-pos );
					}
				} else
					cfg->cfgLocalServer = inText;			// add the local server definition
			}

			OutDebugs( "CONFIG: Local Server address reset to : %s", cfg->cfgLocalServer );
			break;

		case DEFAULT_FTP_SERVER :
            // Implement this if its going to be kept
			break;

		case DEFAULT_RESET :
            // Implement this if its going to be kept
			break;
		
		case DEFAULT_NEW_CONFIG :
            // Implement this if its going to be kept
			break;

		case DEFAULT_NEWS_BACKGROUND :
            StrToIntEx(inText, STIF_SUPPORT_HEX, &cfg->cfgBackgroundColor);
			break;

		case DEFAULT_NEWS_FONT :
            cfg->cfgFontFace = inText;
			break;

		case DEFAULT_TEXT_COLOR :
            StrToIntEx(inText, STIF_SUPPORT_HEX, &cfg->cfgTextColor);
			break;

		case DEFAULT_FONT_HEIGHT :
			if( strstr( inText, "auto" ) )
			{
				cfg->cfgFontHeight = (int)(cfg->cfgBottom * 0.07);
			} else
			{
				cfg->cfgFontHeight = atoi(inText);
			}
			#ifdef _DEBUGHALF
			cfg->cfgFontHeight /= 2;
			#endif
			break;

		case DEFAULT_BAR_HEIGHT :
			if( strstr( inText, "auto" ) )
			{
				cfg->cfgTempBarHeight = (int)(cfg->cfgBottom * 0.08);
			} else
			{
	            cfg->cfgTempBarHeight = atoi(inText);
			}
			#ifdef _DEBUGHALF
			cfg->cfgTempBarHeight /= 2;
			cfg->cfgBarHeight = atoi(inText)/2;
			#endif
			break;

		case DEFAULT_BAR_DELAY :
			{
				cfg->cfgNewsBarDelay = atoi(inText);
				if ((cfg->cfgNewsBarDelay < 1)
					|| (cfg->cfgNewsBarDelay > 50))

				{
					cfg->cfgNewsBarDelay = 20;
				}
	            itoa(cfg->cfgNewsBarDelay, inText, 10);
			}
			break;



		case DEFAULT_NEWS_INTERVAL :
			{
				cfg->cfgNewsInterval = atoi(inText);
				if ((cfg->cfgNewsInterval < 1)
					|| (cfg->cfgNewsInterval > 50))

				{
					cfg->cfgNewsInterval = 4;
				}

				itoa(cfg->cfgNewsInterval, inText, 10);
			}
			break;

		case DEFAULT_SCROLL_SPEED :
			{
				cfg->cfgScrollSpeed = atoi(inText);
				if ((cfg->cfgScrollSpeed < 1)
					|| (cfg->cfgScrollSpeed > 50))

				{
					cfg->cfgScrollSpeed = 10;
				}

				itoa(cfg->cfgScrollSpeed, inText, 10);
			}
			break;

		case DEFAULT_STEP_SPEED :
			{
				cfg->cfgStepSpeed = atoi(inText);

				if ((cfg->cfgStepSpeed < 1)
					|| (cfg->cfgStepSpeed > 15))

				{
					cfg->cfgStepSpeed = 2;
				}

				itoa(cfg->cfgStepSpeed, inText, 10);
			}
			break;

		case DEFAULT_CONTENT_PATH :
			cfg->cfgServerContentPath = inText;
			break;

		case DEFAULT_NEWS_FILENAME :
            cfg->cfgNewsFileName = inText;
			break;

		case DEFAULT_MANIFEST_FILENAME :
            cfg->cfgManifestFileName = inText;
			break;

		case DEFAULT_LOOP_SEQUENCE_FILENAME :
            cfg->cfgLoopsequenceFileName = inText;
			break;

		// ## these filenames are the ones used as bmps for onscreen logos, the filenames represent the names on the server
		// which get renamed to a different local disc filename thats more basic...  news_logo_01.bmp to newslogo.bmp
		case DEFAULT_LOGOIMAGE:
            cfg->cfgMainLogoFileName = inText;
			break;
			
		case DEFAULT_NEWSIMAGE:
			if( cfg->cfgNewsLogoFileName.GetLength()>0 && 
				cfg->cfgNewsLogoFileName.CompareNoCase( inText ) )  // if the logo name changes, then download it.
			{
				AFXDIALOG->downloadConfigNewsLogo = TRUE;
			}

			cfg->cfgNewsLogoFileName = inText;
			break;

		case DEFAULT_SCROLLIMAGE:
			if( cfg->cfgScrollLogoFileName.GetLength()>0 && 
				cfg->cfgScrollLogoFileName.CompareNoCase( inText ) )  // if the logo name changes, then download it.
			{
				AFXDIALOG->downloadConfigNewsScrollLogo = TRUE;
			}

			cfg->cfgScrollLogoFileName = inText;
			break;

		case DEFAULT_NEWS_TIME1:
            cfg->cfgNewsDownloadTime1 = inText;
			if( cfg->cfgNewsNextDownloadTime1 == 0 )
				cfg->cfgNewsNextDownloadTime1 = TodaysNewTime( inText );
			break;
		case DEFAULT_NEWS_TIME2:
            cfg->cfgNewsDownloadTime2 = inText;
			if( cfg->cfgNewsNextDownloadTime2 == 0 )
				cfg->cfgNewsNextDownloadTime2 = TodaysNewTime( inText );
			break;
		case DEFAULT_NEWS_TIME3:
            cfg->cfgNewsDownloadTime3 = inText;
			if( cfg->cfgNewsNextDownloadTime3 == 0 )
				cfg->cfgNewsNextDownloadTime3 = TodaysNewTime( inText );
			break;

		case DEFAULT_UPDATE_EXE:
			if( cfg->cfgUpdateExe.GetLength()>0 && cfg->cfgUpdateExe.CompareNoCase( inText ) )
				// if the exe name changes, then download it.
			{
				AFXDIALOG->downloadConfigExe = TRUE;
			}
			cfg->cfgUpdateExe = inText;
			break;

		case DEFAULT_VIDEODOWNLOADALWAYS:
			cfg->cfgVideoDownloadAlways = atoi( inText );
			break;

		case DEFAULT_IMAGESDOWNLOADALWAYS:
			cfg->cfgImagesDownloadAlways = atoi( inText );
			break;

		case DEFAULT_PARAM01:
			if ( strstr( inText, "rssnews" ) )
				cfg->cfgUseRSSNews =  TRUE;
			break;
	

		// #############################################################
		case DEFAULT_GET_XML_TIME :
            WritePrivateProfileString("DOWNLOAD", "XMLDownloadIntervalTime", inText, INI_FILE);
            cfg->cfgDownloadIntervalTime = inText;
            gotXML_Time = TRUE;
            if (gotXML_Day == TRUE)
            {
                /* format the time and day together*/
                cfg->cfgXMLDownloadInterval = 
                                TimeDateToCTimeSpan(cfg->cfgDownloadIntervalTime,
                                                    cfg->cfgDownloadIntervalDay);
				gotXML_Day = FALSE;
            }
			break;

		case DEFAULT_GET_XML_DAY :
            WritePrivateProfileString("DOWNLOAD", "XMLDownloadIntervalDays", inText, INI_FILE);
            cfg->cfgDownloadIntervalDay = atoi(inText);
            gotXML_Day = TRUE;
            if (gotXML_Time == TRUE)
            {
                /* format the time and day together*/
                cfg->cfgXMLDownloadInterval = 
                                      TimeDateToCTimeSpan(cfg->cfgDownloadIntervalTime,
                                                          cfg->cfgDownloadIntervalDay);
				gotXML_Time = FALSE;
            }
			break;

		case DEFAULT_GET_NEWS_TIME :
            WritePrivateProfileString("DOWNLOAD", "NewsDownloadIntervalTime", inText, INI_FILE);
            cfg->cfgDownloadIntervalTime = inText;
            gotNEWS_Time = TRUE;
            if (gotNEWS_Day == TRUE)
            {
                /* format the time and day together*/
                cfg->cfgNewsDownloadInterval = 
                                TimeDateToCTimeSpan(cfg->cfgDownloadIntervalTime,
                                                    cfg->cfgDownloadIntervalDay);
				gotXML_Day = FALSE;
            }
			break;

		case DEFAULT_GET_NEWS_DAY :
            WritePrivateProfileString("DOWNLOAD", "NewsDownloadIntervalDays", inText, INI_FILE);
            cfg->cfgDownloadIntervalDay = atoi(inText);
            gotNEWS_Day = TRUE;
            if (gotNEWS_Time == TRUE)
            {
                /* format the time and day together*/
                cfg->cfgNewsDownloadInterval = 
                                      TimeDateToCTimeSpan(cfg->cfgDownloadIntervalTime,
                                                          cfg->cfgDownloadIntervalDay);
				gotXML_Time = FALSE;
            }
			break;


		case DEFAULT_UPLOAD_INTERVAL:
			if( strchr( inText, ':' ) )
			{
				cfg->cfgUploadInterval = TimeDateToCTimeSpan( inText, 0 );
			} else
			{
				cfg->cfgUploadInterval = atoi(inText);
				if ((cfg->cfgUploadInterval < 1) || (cfg->cfgUploadInterval > (88640) ))

				{
					cfg->cfgUploadInterval = 60*60;
				}
			}
			break;

		case DEFAULT_SLEEP_START:
			cfg->cfgSleepStartTime = inText;
			break;

		case DEFAULT_SLEEP_STOP:
			cfg->cfgSleepStopTime = inText;
			break;

		default: 
			break;
	}
}




#define CHECKELEMENT(string,value)		if (elementString.CompareNoCase(string) == 0)		XML_CharState = value



/**
  * Event handler that is invoked when a start tag is found
  *
  * A State machine that populates the current impression member
  * and the defaults structure when an open tag is found 
  * and also controls the state of the charactes stage machibe
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class

  */


void LoopSeqXML_Handler::startElement(const   XMLCh* const    uri,
                            const   XMLCh* const    localname,
                            const   XMLCh* const    qname,
                            const   Attributes&     attrs)
{
	char* element = XMLString::transcode(qname);
	CString elementString = element;
	XMLString::release(&element);
	
    /* variables used to extact attibute strings */
    unsigned int len = attrs.getLength();

	Debug(elementString);

	switch (LoopXML_State)
	{
		case FINDING_IMPRESSION :
			if (elementString.CompareNoCase("impression") == 0 || elementString.CompareNoCase("slide") == 0 )
				LoopXML_State = FOUND_IMPRESSION;
			else
			if (elementString.CompareNoCase("defaults") == 0)
				LoopXML_State = FOUND_DEFAULTS;
			else
   			if (elementString.CompareNoCase("start_date") == 0)
				XML_CharState = START_DATE;
			else
            if (elementString.CompareNoCase("end_date") == 0)
				XML_CharState = END_DATE;
			break;

		case FOUND_DEFAULTS :
			if (elementString.CompareNoCase("configversion") == 0)
				XML_CharState = DEFAULT_CONFIGFILE;
			else
			if (elementString.CompareNoCase("configfile") == 0)
				XML_CharState = DEFAULT_CONFIGFILE;
			else
			if (elementString.CompareNoCase("volume_default") == 0)
				XML_CharState = DEFAULT_VOLUME;
			else
			if (elementString.CompareNoCase("brightness") == 0)
				XML_CharState = DEFAULT_BRIGHTNESS;
			else
			if (elementString.CompareNoCase("bright_default") == 0)
				XML_CharState = DEFAULT_BRIGHTNESS;
			else
			if (elementString.CompareNoCase("dialup_number") == 0)
				XML_CharState = DEFAULT_DIALUP_NUMBER;
			else
			if (elementString.CompareNoCase("http_server") == 0)
				XML_CharState = DEFAULT_HTTP_SERVER;
			else
            if (elementString.CompareNoCase("news_logo") == 0)
				XML_CharState = DEFAULT_NEWS_LOGO;
			else
            if (elementString.CompareNoCase("news_background") == 0)
				XML_CharState = DEFAULT_NEWS_BACKGROUND;
			else
            if (elementString.CompareNoCase("news_font") == 0)
				XML_CharState = DEFAULT_NEWS_FONT;
			else
            if (elementString.CompareNoCase("text_color") == 0)
				XML_CharState = DEFAULT_TEXT_COLOR;
			else
            if (elementString.CompareNoCase("font_height") == 0)
				XML_CharState = DEFAULT_FONT_HEIGHT;
			else
            if (elementString.CompareNoCase("bar_height") == 0)
				XML_CharState = DEFAULT_BAR_HEIGHT;
			else
            if (elementString.CompareNoCase("newsbar_delay") == 0)
				XML_CharState = DEFAULT_BAR_DELAY;
			else
            if (elementString.CompareNoCase("news_interval") == 0)
				XML_CharState = DEFAULT_NEWS_INTERVAL;
			else
            if (elementString.CompareNoCase("scroll_speed") == 0)
				XML_CharState = DEFAULT_SCROLL_SPEED;
			else
            if (elementString.CompareNoCase("step_speed") == 0)
				XML_CharState = DEFAULT_STEP_SPEED;
			else
            if (elementString.CompareNoCase("server_content_path") == 0)
				XML_CharState = DEFAULT_CONTENT_PATH;
			else
            if (elementString.CompareNoCase("news_filename") == 0)
				XML_CharState = DEFAULT_NEWS_FILENAME;
			else
            if (elementString.CompareNoCase("manifest_filename") == 0)
				XML_CharState = DEFAULT_MANIFEST_FILENAME;
			else
            if (elementString.CompareNoCase("loopsequence_filename") == 0)
				XML_CharState = DEFAULT_LOOP_SEQUENCE_FILENAME;
			else
            if (elementString.CompareNoCase("xml_download_interval_time") == 0)
				XML_CharState = DEFAULT_GET_XML_TIME;
			else
            if (elementString.CompareNoCase("xml_download_interval_day") == 0)
				XML_CharState = DEFAULT_GET_XML_DAY;
			else
			if (elementString.CompareNoCase("news_download_interval_time") == 0)
				XML_CharState = DEFAULT_GET_NEWS_TIME;
			else
            if (elementString.CompareNoCase("news_download_interval_day") == 0)
				XML_CharState = DEFAULT_GET_NEWS_DAY;
			else

            CHECKELEMENT( "upload_interval",		DEFAULT_UPLOAD_INTERVAL ); else

            CHECKELEMENT( "name",					DEFAULT_NAME ); else

            CHECKELEMENT( "BackgroundColor",		DEFAULT_BGCOLOR ); else

            CHECKELEMENT( "sleep_start",			DEFAULT_SLEEP_START ); else
            CHECKELEMENT( "sleep_stop",				DEFAULT_SLEEP_STOP ); else

            CHECKELEMENT( "videodownloadalways",	DEFAULT_VIDEODOWNLOADALWAYS ); else
            CHECKELEMENT( "imagesdownloadalways", 	DEFAULT_IMAGESDOWNLOADALWAYS ); else

			CHECKELEMENT( "download_exe",			DEFAULT_UPDATE_EXE ); else
			CHECKELEMENT( "ExeApp",					DEFAULT_UPDATE_EXE ); else

			//
			// <localserver>Display_112233=server1</>
			CHECKELEMENT( "localserver",			DEFAULT_LOCAL_SERVER ); else
			
			CHECKELEMENT( "load_image", 			DEFAULT_LOGOIMAGE ); else
            CHECKELEMENT( "news_image", 			DEFAULT_NEWSIMAGE ); else
            CHECKELEMENT( "news_scroll_image", 		DEFAULT_SCROLLIMAGE );
			break;

		case FOUND_IMPRESSION :
			if (elementString.Compare("id") == 0 )
			{
				XML_CharState = IMPRESSION_ID;
				gotID = TRUE;
			} else
			if (elementString.Compare("order") == 0)
			{
				XML_CharState = IMPRESSION_ORDER;
				gotOrder = TRUE;
			} else
			if (elementString.Compare("playlength") == 0)
			{
				XML_CharState = IMPRESSION_PLAYLENGTH;
				gotPlaylength = TRUE;
			} else
			if (elementString.Compare("duration") == 0)
			{
				XML_CharState = IMPRESSION_DURATION;
				gotPlaylength = TRUE;
			} else
			if (elementString.Compare("volume") == 0)
			{
				XML_CharState = IMPRESSION_VOLUME;
				gotVolume = TRUE;
			} else
			if (elementString.Compare("image_file") == 0 )
			{
				XML_CharState = IMPRESSION_IMAGE;
				gotImage = TRUE;
			} else
			if (elementString.Compare("image_dir") == 0 )
			{
				XML_CharState = IMPRESSION_IMAGEDIR;
				gotImage = TRUE;
			} else
			if (elementString.Compare("referal") == 0 )
			{
				XML_CharState = IMPRESSION_REFERAL;
			} else
			if (elementString.Compare("multimedia_file") == 0 || elementString.Compare("media_file") == 0 )
			{
				XML_CharState = IMPRESSION_MULTIMEDIA;
				gotMultimedia = TRUE;
			} else
			if (elementString.Compare("multimedia_dir") == 0 || elementString.Compare("media_dir") == 0 )
			{
				XML_CharState = IMPRESSION_MULTIMEDIADIR;
				gotMultimedia = TRUE;
			} else
			if (elementString.Compare("multimedia_url") == 0 || elementString.Compare("media_url") == 0 )
			{
				XML_CharState = IMPRESSION_MULTIMEDIAURL;
				gotMultimedia = TRUE;
			} else
			if (elementString.Compare("url_params") == 0 )
			{
				XML_CharState = IMPRESSION_URLPARAMS;
			} else
			if (elementString.Compare("include") == 0 )
			{
				XML_CharState = IMPRESSION_INCLUDE;
			} else
			// ============= MDID SPECIFIC ELEMENTS TO DETECT ==========
			if (elementString.CompareNoCase("slideid") == 0)
			{
				XML_CharState = IMPRESSION_ID;
				gotID = TRUE;
			} else
			if (elementString.CompareNoCase("imageresource") == 0 )
			{
				XML_CharState = IMPRESSION_IMAGEURL;
				gotImage = TRUE;
			} else
			if (elementString.CompareNoCase("imagefilename") == 0 )
			{
				XML_CharState = IMPRESSION_IMAGE;
				gotImage = TRUE;
			} else
			if (elementString.CompareNoCase("image_id") == 0 || elementString.CompareNoCase("imageid") == 0 )
			{
				XML_CharState = IMPRESSION_IMAGE_ID;
			} else
			if (elementString.CompareNoCase("image_cid") == 0 || elementString.CompareNoCase("collectionid") == 0  )
			{
				XML_CharState = IMPRESSION_IMAGE_CID;
			} else
			if (elementString.Compare("field") == 0 )		// check for video image field
			{
				if( attrs.getLength() >0 )
				{
					char *QName, *Value;
					unsigned int attrCount = 0;

					QName = XMLString::transcode(attrs.getQName((short) attrCount));
					CString QNameString = QName;
		            XMLString::release(&QName);
	            
		            Value = XMLString::transcode(attrs.getValue((short) attrCount));
					CString QValueString = Value;

                    if (QNameString.CompareNoCase("name") == 0 && QValueString.CompareNoCase("Source") == 0 )
				    {
						XML_CharState = IMPRESSION_MULTIMEDIA;
						gotMultimedia = TRUE;
				    } else
                    if (QNameString.CompareNoCase("name") == 0 && QValueString.CompareNoCase("Video") == 0 )
				    {
						XML_CharState = IMPRESSION_MULTIMEDIA;
						gotMultimedia = TRUE;
				    } else
                    if (QNameString.CompareNoCase("name") == 0 && QValueString.CompareNoCase("Title") == 0 )
				    {
						XML_CharState = IMPRESSION_TITLE;
				    } else
                    if (QNameString.CompareNoCase("name") == 0 && QValueString.CompareNoCase("Duration") == 0 )
				    {
						XML_CharState = IMPRESSION_PLAYLENGTH;
				    } else
                    if (QNameString.CompareNoCase("name") == 0 && QValueString.CompareNoCase("Coverage") == 0 )
				    {
						XML_CharState = IMPRESSION_PLAYLENGTH;
				    }

				}
			}
			break;
	}
	elementString.Empty();
}
/*
Added Support for MDID xml files ;
    <Slide>
      <imageresource>getimage.aspx?id=15&amp;cid=1&amp;res=15.jpg&amp;format=F&amp;token=user_681a0f2f-4507-4d9a-8706-927aebd48a34&amp;ssid=8</imageresource>
      <slideid>26</slideid>
      <imageid>15</imageid>
      <collectionid>1</collectionid>
      <imagefilename>1_15.jpg</imagefilename>
      <imagenotes></imagenotes>
      <catalogdata>
        <fields>
          <field name="Collection">Personal Images</field>
          <field name="Title">test</field>
          <field name="Source">video3.mpg</field>
		  </fields>
      </catalogdata>
    </Slide>
	
*/

















/**
  * Event handler that is invoked when an end tag is found
  *
  * A State machine that controls the state of the charactes state machine
  *
  * 
  * @params parametes are as specified by the SAX2 interface
  *         of a DefaultHandler class
  */

void LoopSeqXML_Handler::endElement(const XMLCh* const uri,
								   const XMLCh* const localname,
								   const XMLCh* const qname)
{
	char* element = XMLString::transcode(qname);
	CString elementString = element;
	XMLString::release(&element);
	
	Debug(elementString);
	/* Make sure we are not processing the character input*/
	XML_CharState = IMPRESSION_INVALID;

    
	switch (LoopXML_State)
	{
		case FOUND_DEFAULTS :
			if (elementString.Compare("defaults") == 0)
			{
				Debug("FOUND /defaults");
				LoopXML_State = FINDING_IMPRESSION;
			}
			break;

		case FOUND_IMPRESSION :
			if (elementString.Compare("impression") == 0 || elementString.Compare("Slide") == 0)
			{	
				if ( (gotID == TRUE) &&	(gotImage == TRUE || gotMultimedia == TRUE)	)
				{
					InsertSorted();
					//OutDebugs( "Adding impression slideid-%d, imageid-%s", currentImpression.m_id, currentImpression.m_image_id );
				}

				// reset for next item
                InitLanguageSpecificImpression();
                InitCurrentImpression( pImpressionList->GetCount() );

				LoopXML_State = FINDING_IMPRESSION;
            }
			break;

		case FINDING_IMPRESSION :
			InitCurrentImpression( pImpressionList->GetCount() );
			break;

		default :
			Debug("The stage machine for 'endElement' of LOOP SEQUENCE XML");
			Debug("parser is broken it is in default state and it should");
			Debug("never have gotten here");
	}
	elementString.Empty();
}

/** 
 *  If the XML file is misformed then this event handler gets called
 *
 *  Just does debug for now
 *
 *  TODO: Check what happens if Validation is switched off and there
 *        is an error in the XML.
 *
 */

void LoopSeqXML_Handler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());

    CString errorMessage;
    long i = exception.getLineNumber();
    errorMessage.Format("Fatal Error:  %s at line: %i", message, i);

    Debug(errorMessage);
}

/** 
 *  Initialises all the local variables of the Current impression
 *  and the flags that signify if a certain element has been found
 *
 */
void LoopSeqXML_Handler::InitCurrentImpression( int id )
{
	gotID = FALSE;
	gotOrder = FALSE;
	gotPlaylength = FALSE;
	gotVolume = FALSE;				
	gotLanguage = FALSE;
	gotImage = FALSE;
	gotImageType = FALSE;
	gotMultimedia = FALSE;
	gotMultimediaType = FALSE;

    gotXML_Time = FALSE;
    gotXML_Day = FALSE;

    gotNEWS_Time = FALSE;
    gotNEWS_Day = FALSE;

	currentImpression.m_id = 0;
	currentImpression.m_image_id = "";
	currentImpression.m_image_cid = "";

	currentImpression.m_image_file = "";
	currentImpression.m_image_url = "";
	currentImpression.m_image_file_type = "";
	currentImpression.m_language = "";
	currentImpression.m_multimedia_file = "";
	currentImpression.m_multimedia_file_type = "";
	currentImpression.m_order = 0;
	currentImpression.m_playlength = CFG->cfgDefaultPlayLength;					// default play length to 3 seconds
	currentImpression.m_volume = 7;						// 70%
	currentImpression.m_brightness = 8;					// 80%
	currentImpression.m_hitCount = 0;
	currentImpression.m_viewCount = 0;

	currentImpression.imageData = NULL;
}

/** 
 *  
 *  
 *
 */
void LoopSeqXML_Handler::InitLanguageSpecificImpression()
{
	gotImage = FALSE;
	gotImageType = FALSE;
	gotLanguage = FALSE;
	gotMultimedia = FALSE;
	gotMultimediaType = FALSE;
    gotXML_Time = FALSE;
    gotXML_Day = FALSE;

	currentImpression.m_image_file = "";
	currentImpression.m_image_url = "";
	currentImpression.m_image_file_type = "";
	currentImpression.m_language = "";
	currentImpression.m_multimedia_file = "";
	currentImpression.m_multimedia_file_type = "";
}




/**
 *  Initialise the defaults structure
 *
 */
void LoopSeqXML_Handler::InitLanguageList(CList <CString, CString>* languages)
{
    pLanguageList = languages;
}

/**
 *  Insets the currentImpression into a List of Impression.
 *
 *  The current impression will be inserted in the POSITION
 *  of the list which mainteins an ordered list based on the
 *  order member of the Impression
 *
 */
void LoopSeqXML_Handler::InsertSorted()
{
    POSITION currentPos = 0;
    ImpressionData currentData;
    POSITION nextPos = 0;
    ImpressionData nextData;

	if( pImpressionList == NULL )
		return;
 
    int iTotal = pImpressionList->GetCount();

    for (int i = 0; i <= iTotal; i++)
    {
        if (i >= iTotal - 1)
        {
            if (iTotal == 1)
            {
                currentPos = pImpressionList->FindIndex(i);
                currentData = pImpressionList->GetAt(currentPos);

                if (currentImpression.m_order < currentData.m_order)
                {
                    pImpressionList->AddHead(currentImpression);
                    i = iTotal + 1;
                }
                else
                {
                    pImpressionList->AddTail(currentImpression);
                    i = iTotal + 1;
                }
            }
            else
            {
                pImpressionList->AddTail(currentImpression);
                i = iTotal + 1;
            }
        }
        else
        {
            currentPos = pImpressionList->FindIndex(i);
            currentData = pImpressionList->GetAt(currentPos);

			// if we have no pos , just add it to the end.
            if (currentImpression.m_order == 0 &&  currentData.m_order == 0 )
            {
                pImpressionList->AddTail(currentImpression);
                i = iTotal + 1;
            }
			else
            if (currentImpression.m_order < currentData.m_order)
            {
                pImpressionList->InsertBefore(currentPos, currentImpression);
                i = iTotal + 1;
            }
            else
            {
                nextPos = pImpressionList->FindIndex(i+1);
                nextData = pImpressionList->GetAt(nextPos);

				// is the CURRENT 
                if ((currentImpression.m_order >= currentData.m_order)
                    && (currentImpression.m_order <= nextData.m_order))
                {
                    pImpressionList->InsertAfter(currentPos, currentImpression);
                    i = iTotal + 1;
                }
            }
        }
    }
}

/**
 *  Checks if current loop sequence is current
 *
 *  If the current system date falls between the <start_date> and
 *  <end_date> found in the loop sequence XML file.
 *
 *  @return     TRUE    if the currrent date fall between the
 *                      start date and the end date
 *
 *              FALSE   if it doesn't
 *
 *  TODO: INPUT VALIDATION
 *
 */

BOOL LoopSeqXML_Handler::IsCurrent()
{
    
    /* Extract information from the startDate*/
    int startYear = 0;
    int startMonth = 0;
    int startDay = 0;

	if( startDateString && startDateString.GetLength() )
	    ExtractDate(&startDateString, &startYear, &startMonth, &startDay);

    /* Extract information from the endDate */
    int endYear = 0;
    int endMonth = 0;
    int endDay = 0;

	if( endDateString && endDateString.GetLength() )
	    ExtractDate(&endDateString, &endYear, &endMonth, &endDay);

    CTime currentTime = CTime::GetCurrentTime();

    
    /* Some crude validation so that CTime constructor doesn't throw an exception. */
    if ((startYear != 0) && (startMonth != 0) && (startDay != 0) &&
		(endYear != 0) && (endMonth != 0) && (endDay != 0))
    {
    
        CTime startTime = CTime::CTime(startYear, startMonth, startDay, 0, 0, 0);
        CTime endTime = CTime::CTime(endYear, endMonth, endDay, 0, 0, 0);

        if ((currentTime >= startTime) && (currentTime <= endTime))
        {
            return TRUE;
        }
	    else
        {
            return FALSE;
        }
    }
    else
    {
        return (FALSE);
    }
}
    


void LoopSeqXML_Handler::AddLanguage(CString languageStr)
{
    int numberOfLanguages = pLanguageList->GetCount();
    BOOL languageIsNew = TRUE;

    if (numberOfLanguages > 0)
    {
        for (int i = 0; i <= numberOfLanguages - 1; i++)
        {
            POSITION newPos = pLanguageList->FindIndex(i);
            CString existingLanguage = pLanguageList->GetAt(newPos);

            if (existingLanguage == languageStr)
            {
                languageIsNew = FALSE;
                break;
            }
        }
    }

    if (languageIsNew == TRUE)
    {
        pLanguageList->AddTail(languageStr);    
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
void LoopSeqXML_Handler::Debug(CString debugMessage)
{
  
//#define DEBUG_TO_EDIT 
#ifdef DEBUG_TO_EDIT
    
    if (m_OutputEdit->m_hWnd != 0)
    {
        CString tempString;
	    this->m_OutputEdit->GetWindowText(tempString);
	    this->m_OutputEdit->SetWindowText(tempString + debugMessage + "\r\n");
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
#endif
}
