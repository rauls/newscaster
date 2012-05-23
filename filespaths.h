/*	###################################################
	## RS: 9-04-2004
	Define all file path locations and relative files
*/

#ifndef __FILEPATHS__
#define __FILEPATHS__


// hard location of project, but first priority is for the code to try all files relative to EXE first
// we dont want to be limited to only one location - for testing reasons and moveability.
#define	BASEDRIVE				"C:\\"
#define	BASEDIR					"Dvorak"

#define	DEBUG_FN				"debug.txt"

#define CONTENT_DIR				"content\\"
#define	XML_DIR					"xml\\"
#define	NEWS_FN					"news.xml"
#define	LOOP_FN					"loop.xml"
#define	MANIFEST_FN				"manifest.xml"
#define	LOGO_FN					"iTV.jpg"

// Server Defaults
#define	HOST_LIVESERVER			"www.ivtmm.com.au:80"		// official default server
#define	HOST_TESTSERVER			"192.168.1.1:80"			// if running on devbed, use test server...as backup
#define	HOST_CONTENT			"/userfiles/"
#define	HOST_NEWSURL			"/get_newsflash.html"
#define	HOST_MANIFESTURL		"/get_manifest.html"
#define	HOST_LOOPURL			"/get_loopsequence.html"

// These should be in the application folder, but we'll leave em there for now.
#define HISTORY_FILE        	"C:\\history.log"
#define INI_FILE            	"C:\\Dvorak.ini"		
#define FLAG_DIRECTORY      	"C:\\Dvorak\\flags\\"   // gotta put this in .ini file
#define DEFUALT_LOOP        	"default\\loop.xml"     // gotta put this in .ini file
#define KEYPRESS_SOUND      	"C:\\dvorak\\button.wav"
#define TAXI_METER_BACKGROUND   "C:\\Dvorak\\taxi_meter.bmp"


#endif



