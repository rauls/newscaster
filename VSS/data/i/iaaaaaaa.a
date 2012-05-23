#if !defined(_HISTORY__H_)
#define _HISTORY__H_

/* The functions defined in this file are used to log the history 
   if the IVA. It will be used to collect statistical information 
   of the operation of the IVA and any error conditions that are
   encountered*/

#define	FN_OPERATIONS	"Logs\\operations.log"
#define	FN_ERRORS		"Logs\\errors.log"
#define	FN_DEBUG		"Logs\\debug.log"

#define HISTORY_FILE        "Logs\\history.log"
#define DELIMITER           ","


typedef enum Buttons
{                
    btnInformation,
    btnForward,
    btnBackward,
    btnLanguage

} tag_App_Button;


typedef enum App_Error   
{                
    errXML,
    errBadMedia,
    errMediaMissing,
    errNoNews,
    errNoLoopSequence,
    errMediaExists

} tag_App_Error;

BOOL Log_App_FileCat( char *filename, char *txt );

BOOL Log_App_FileWrite( char *filename, char *txt );

void Log_App_Status( CString filename, int impressionID, CString eventMessage);

void Log_App_Event(int impressionID, CString eventMessage);

void Log_App_CellInfo( CString eventMessage, CString name );

void Log_App_FileError(App_Error error, CString filename, CString details);

void Log_App_Error( char * details);

void Log_App_Debug( CString details );



#endif // #define _HISTORY__H_