#ifndef	__MYANSI_H
#define	__MYANSI_H


#ifdef __cplusplus
extern "C" {
#else
// define bool for C - used to be: unsigned char bool;
enum c_bool { false, true };
typedef enum c_bool bool;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef QS_MAC
void	SysBeep( int _type );
#endif

#define	MACPATHSEP			':'
#define	WINPATHSEP			'\\'
#define	UNXPATHSEP			'/'

#define	PATHSEP				WINPATHSEP
#define	OTHER_OS1_PATHSEP	UNXPATHSEP
#define	OTHER_OS2_PATHSEP	MACPATHSEP
#define	PATHSEPSTR			"\\"
#define	PATHBACKDIR			"\\..\\" 


#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                (!FALSE)
#endif

#if	QS_MAC
#define	PROGRESS_IDLE	ProgressIdle();
#else
#define	PROGRESS_IDLE ;
#endif


#define 	myisspace(a) 	((a) == ' ' || (a) == '\t')
#define		myisdigit(c)	((c) >='0' && (c) <='9') ? 1:0
#define		myisupper(c)	((c) >='A' && (c) <='Z') ? 1:0
#define		myislower(c)	((c) >='a' && (c) <='z') ? 1:0
#define		mytoupper(c)	((c) >='a' && (c) <='z') ? 'A' + (c) - 'a' : (c)
#define		mytolower(c)	((c) >='A' && (c) <='Z') ? 'a' + (c) - 'A' : (c)
#define		myisalnum(c)	((c) >=0x32 && (c) <=0x7F) ? 1:0

//#define		mycreator 		'FWEB' 
//#define 	myfileType 		'TEXT'



// if int64 doesnt exist (UNIX) then type define the UNIX longlongint into int64
//#if QS_MAC
//typedef long long __int64;
//#elif QS_UNIX
//#include <sys/types.h>
//#define	__int64		int64_t
//#endif


size_t			GetWeeksInYear(size_t nYear);
unsigned char	DaysToFirstSundayOfYear(size_t nYear);
size_t			DaysToYearSince1970(size_t nYear);
int				GetStartOfWeek(struct tm* ptmDest, size_t nWeekNum, size_t nYear);


long SwapEndian( long x );
void SleepSecs( long length );
void SleepTicks( long length );

long ErrorMsg( const char *txt, ... );
long CautionMsg( const char *txt, ... );
long NotifyMsg( const char *txt, ... );

int RunSelf( const char *param );

long HexStr2int( const char *str );

__int64 GetDiskFree( const char *path );

long IsFileInvalid( const char *fileName, char *format );

int	mydatecmp(const char* szDate1, const char* szDate2);

char *mystrstr(const char * str, const char * pat );

double timems( void );
void UserMsg( const char *txt );
void ErrorCode( const char *text, int err );
long MakeDirPath( const char *dir );
int MakeDir( const char *dir );

long ReadLong( unsigned char *data );
void UprString (char *txt, int flag);

long IsFilePKZIP( const char *fileName );
char *CopyLine( char *d, char *s );

unsigned long ipstringtolong( const char *ipstr );
// non standard wierd mac bits...

const char* revstrrchr( const char *s, const char *e, int c );
//void checkpath(char *s1,char delimiter);
void TrimPathForClassicMacOS (char *pathString);

int breakatx( char *s2, char brkch, int words );

#if QS_MAC
short	pstrlen (unsigned char * string);
void	pstrcpy (unsigned char * dstString, unsigned char * srcString);
void	pstrcat (unsigned char * dstString,unsigned char * srcString);
void	strcpyc2p (unsigned char * dst, char *src);
void	strcpyp2c (char *dst, unsigned char * src)	;
void	strcatc2p (unsigned char * dst, char *src);
#endif

void swaplong( long *l1, long *l2 );
long iplongtoipstring( long ip, char *str );

int				strcpyuntil( char *s1, char *s2, char until );
size_t			strncpyuntil(char* sz, const char* csz, size_t n, char c);

long			GetTimeFromFilename( char *filename );
void 			getsub		(char *src, char *dst);
void 			strip		(char *src, char *dst);
void			stripxchar(char *instring, char x);
char *			strstrn(const char * str, const char * pat, int n);

char *			strstri(const char * str, const char * pat);
long 			myCrackNum 	(char **x);
long			mystrcpylower( char *dst, const char *src );	
unsigned long 	myatoint( const char *text, long *num );
unsigned long 	myatoi( const char *text );
void			*mymemset (void *_s, int c, int n);
size_t 			mystrlen ( const char *str );				// c-string length function
long			mystrcpy (char *dst, const char *src);	// c-string copy function
long			mystrncpy( char *dst, const char *src, int len );	// c-string copy function
long			mystrncpyNull( char *dst, const char *src, int len );	// c-string copy function that makes sure we null terminate
long			mystrncpylower( char *dst, const char *src, int len );
void			mystrcat( char *dst, const char *src );	// concatenate src with dst c-string
long			mystrncatNull( char *dst, const char *src, int dstSize );
char *			mystrrnchr(const char *s, char c, int n);
char *			mystrchr(const char *s, char c);
char *			mystrrchr(const char *s, int c);

// Finds the last character in the src string which is part of the srchChars string and
// copies the string up to this last character found into the dest string.
char			*mystrrchrs( char *dest, const char *src, char *srchChars );

int 			mystrcmpi(const char *str1, const char *str2);
int				mystrncmpi(const char* s1, const char* s2, int iLen);

int 			strcmpd(const char * str1, const char * str2);
short 			strcmpiPart(char *str1, char *str2);
short 			strcmpDom(char *str1, char *str2);
size_t 			strcpyx( char *s1, char *s2, char brkch, size_t words );
size_t 			strncpyx( char *s1, char *s2, char brkch, size_t words, size_t maxbuf );
int 			strcpywn( char *s1, char *s2, int words );
int 			strcpyw( char *s1, char *s2 );
void 			CopyFilenameUsingPath( char *dest, char *pathfile, char *filename );
int 			PathFromFullPath( char *fullpath, char *path );
int 			PathFromURL( char *url, char *path );
char	*		FileFromPath( const char *fullpath, char *outfile );
int 			Int2Str		(long num, char *str);
char	*		Long2Str	( long val, long td );
char	*		Long2HexStr	( long val );
void 			Ptr2HexStr	(void *ptr, char *str);
void	*		HexStr2Ptr	(char *str);
char			HexToChar (char *txt);
char	*		StrAdd		( char *d, char *s1, char *s2 );


void trimString(char *instring, int size);
void trimLine(char *line);			// trims trailing returns & newlines
void trimLineWhite(char *line);		// trims trailing spaces, tabs, returns & newlines

long GetGzFileLength( char *fileName );
int getVersion( char *s1, char *s2, int max);
int strcpybr( char *s1, char *s2);
void stripURL(char *instring);
int stripType(char *dest, char *src);
__int64  GetRealFileLength( char *fileName );
__int64 GetFileLength( char *fileName );

long FormatNum( char *txt, long x);
long FormatLongNum( long n, char *txt );
long FormatDoubleNum( double n, char *txt );
long ULong2CStr( unsigned long x, char *dest );
long Double2CStr( double x, char *dest, short type );

void stripReferURL(char *instring, char *outstring);
char *Path2FilePath( char *path);
long ReplaceStr( char *string, char *newstring, const char *key, const char *newkey, short wordflag );
short countchar(char *buffer,char ch);
long strcmpMany( char *string, char **comps );
long IsGzFP( FILE *fp );
long IsGzFile( char *fileName );
long IsBzFP( FILE *fp );
long IsBzFile( char *fileName );
long GetGzFPLength( FILE *fp );
long GetFPLength( FILE *fp );
long MemToFile( char *filename, void *data, long limitLen );
long FileToMem( const char *filename, void *data, long limitLen );
int strcpybrk( char *s1, const char *s2, char brkch, int words );
void createURL(char *dest, char *src);

void SetFilePrefix (char *srcFile, char *prefix);
void SetFileExtension (char *srcFile, const char *extn);
short stripPort(char * str, short len, char chr);
void makepath( char *path, char *oldpath);
void EqualToTab(char *instring);
void TabToEqual(char *instring);
char *CheckWWWprefix( char *host );
int strcpydomain( char *d, char *s );
void ColonToSlash(char *instring);
void strcpybreak( char *dst, char *src, char brk, int len);	// c-string copy function

unsigned long IPStrToIPnum( const char *ipstr );
char *IPnumToIPStr( long ip, char *ipstr );

void ClearLongs( long *p, long len );

void MemSwap( register char *sc, register char *dc, register long len );

typedef long (*SortCmpFunction) (void *, void *, long *);

#define TOP_SORT	1
#define BOTTOM_SORT	0
long FastQSort (void *base, long n, long size, SortCmpFunction cmp, short sortOrder );
void ClearRam( long *p, long len );

#ifndef	strcmpi
#define	strcmpi	mystrcmpi
#endif

#define	safestrcpy( dest, source )		mystrncpy( dest, source, sizeof( dest ) )
#define MemClr(r,s) ClearRam( (long*)r, (long)s )

void GetBuildDate (char *out);

#ifdef __cplusplus
}
#endif

#endif
