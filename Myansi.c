#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>	// for size_t
#include <assert.h>
#include <stdarg.h>

#include "myansi.h"

#include <sys/stat.h>

#include <windows.h>
#include <winuser.h>   /* includes the common control header */

#include <time.h>

// use this from config.cpp
extern long OutDebugs( const char *txt, ... );


static char		tempReturnStr[32];

static int	DaysToYear[]	= {	// From 1970
	0,		365,	730,	1096,	1461,	1826,	2191,	2557,	2922,	3287,
	3652,	4018,	4383,	4748,	5113,	5479,	5844,	6209,	6574,	6940,
	7305,	7670,	8035,	8401,	8766,	9131,	9496,	9862,	10227,	10592,
	10957,	11323,	11688,	12053,	12418,	12784,	13149,	13514,	13879,	14245,
	14610,	14975,	15340,	15706,	16071,	16436,	16801,	17167,	17532,	17897,
	18262,	18628,	18993,	19358,	19723,	20089,	20454,	20819,	21184,	21550,
	21915,	22280,	22645,	23011,	23376,	23741,	0
};

size_t	DaysToYearSince1970(size_t nYear)
{
	QS_ASSERT(nYear>=1970);
	QS_ASSERT(nYear<=2035);

	return DaysToYear[nYear-1970];
}

size_t	DaystoPreviousSunday(size_t nWeekNum, size_t nYear)
{
	size_t		nDays		= 0;
	size_t		nDayOffset;

	nDays		+= DaysToYearSince1970(nYear);	// # days from 1970 to nYear.
	nDays		+= (nWeekNum * 7);

	nDayOffset = (nDays % 7);
	if (nDayOffset  > 3)				// the 1st sunday in 1970 is '3' days into the year!
	{
		nDays		-= nDayOffset;					// This may take us into the previous year!
		nDays		+= 3;
	}
	else
	{
		nDays		-= nDayOffset;					// This may take us into the previous year!
		nDays		-= 4;
	}

	return nDays;
}

// 0 = Success, -1 = Failure.
int
GetStartOfWeek(struct tm* ptmDest, size_t nWeekNum, size_t nYear)
{
	struct tm*	ptm			= NULL;
	time_t		tTimeSunday	= 0;
	size_t		nDays		= 0;

	nDays = DaystoPreviousSunday(nWeekNum, nYear);

	tTimeSunday += (ONEDAY * nDays);	

	ptm = localtime(&tTimeSunday);			// and create the struct tm.

	*ptmDest = *ptm;

	return 0;
}


//
//
// DO not EVER delete this function it is really needed
//
//
long SwapEndian( long x )
{
	long a;
	a = (x >> 16)&0xff;
	a |= x&0xff00;
	a |= (x&0xff) << 16;
	return a;
}


long ReadLong( unsigned char *data )
{
	long x;

	x = data[0];
	x = x<<8 | data[1];
	x = x<<8 | data[2];
	x = x<<8 | data[3];
	return x;
}

#ifndef	QS_MAC

void	SysBeep( int _type )
{
#if	QS_WINDOWS
	MessageBeep( _type );
#elif QS_UNIX
	printf( "\07" );
#endif
}

#endif



long NotifyMsg( const char *fmt, ... )
{
	char txt[4000];

	if ( fmt ){
		va_list		args;
		va_start( args, fmt );
		vsprintf( txt, fmt, args );
		va_end( args );
	}

#if	QS_WINDOWS
	return WinNotifyMsg( txt );
#elif QS_UNIX
	return UnixNotifyMsg( txt );
#elif QS_MAC
	NotifyUser (txt, 0);
	return (0);
#endif
}

long CautionMsg( const char *fmt, ... )
{
	char txt[4000];

	if ( fmt ){
		va_list		args;
		va_start( args, fmt );
		vsprintf( txt, fmt, args );
		va_end( args );
	}

#if	QS_WINDOWS
	return WinCautionMsg( txt );
#elif QS_UNIX
	return UnixCautionMsg( txt );
#elif QS_MAC
	//return MacCautionMsg( txt );
	NotifyUser (txt, 0);				// have to use NotifyUser for now, as CautionUser takes a constant, not a string
	return (0);
#endif
}

long ErrorMsg( const char *fmt, ... )
{
	char txt[4000];

	if ( fmt ){
		va_list		args;
		va_start( args, fmt );
		vsprintf( txt, fmt, args );
		va_end( args );
	}

#if	QS_WINDOWS
	return WinErrorMsg( txt );
#elif QS_UNIX
	return UnixErrorMsg( txt );
#elif QS_MAC
	//return MacErrorMsg( txt );
	//NotifyUser (txt, 0);				// have to use NotifyUser for now, as CautionUser takes a constant, not a string
	HandleProcessError (txt);
	return (0);
#endif
}


void UprString( char *txt, int flag )
{
	long len;
	if ( txt ) {
		len = strlen( (const char *)txt );
		while( *txt && len>0 ){
			if ( *txt >= 'a' && *txt <= 'z' )
				*txt -=  ('a'-'A');
			txt++; len--;
		}
	}
}


void SleepTicks( long length )
{
#if QS_WINDOWS
		Sleep( length * 17 );
#endif				
#if QS_UNIX		// UNIX sleep method
		usleep( length * 17 );
#endif
#if	QS_MAC
		MacSleep( length);
#endif
}


void SleepSecs( long length )
{
#if QS_WINDOWS
		Sleep( length * 1000 );
#endif				
#if QS_UNIX		// UNIX sleep method
		sleep( length );
#endif
#if	QS_MAC
		MacSleep( length * 60 );
#endif
}


#if QS_WINDOWS
#include <intshcut.h>
int RunSelf( const char *param )
{
	char dir[320] = "c:\\";
	char *appexe = __argv[0];
	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo; 
	long ret;

	dir[0] = appexe[0];
	//CreateProcessAsUser
	sprintf( dir, "\"%s\" %s", appexe, param );

	//return system( dir );
//	ShellExecute( NULL, oper, doc, param, dir, SW_SHOW );
	memset( &siStartInfo, 0, sizeof(STARTUPINFO ) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	ret = CreateProcess( appexe, dir, 
		NULL, NULL,
		TRUE,
		NORMAL_PRIORITY_CLASS, 
		NULL, NULL, 
		&siStartInfo, 
		&piProcInfo );

	while( WaitForInputIdle(piProcInfo.hProcess,1000) != -1 )
		Sleep(1);
	
	return ret;
}	
#else
int RunSelf( const char *param )
{
	char newparam[320];

	sprintf( newparam, "fweb %s", param );
	return system( newparam );
}
#endif


__int64 GetDiskFreeSpace95( const char *drive )
{
#if QS_WINDOWS
	DWORD	sec,byte,freeb,total;
	GetDiskFreeSpace( drive, &sec,&byte, &freeb, &total );
	return (__int64)(sec*byte*freeb);
#endif
	return 0;
}

__int64 GetDiskFree( const char *path )
{
	__int64	free = -1,tot,freedisk;
	double	freespace = -1;
	char	dir[256];

#if QS_WINDOWS
	if ( path[0] == '\\' )
		PathFromFullPath( (char*)path, dir );	// safe cast as dir is non-null.
	else {
		dir[0] = path[0];
		dir[1] = ':';
		dir[2] = 0;
	}
	GetDiskFreeSpaceEx( dir, (PULARGE_INTEGER)&free, (PULARGE_INTEGER)&tot, (PULARGE_INTEGER)&freedisk );
	if ( free < 0 )
		free = GetDiskFreeSpace95( dir );
#endif
	return free;
}


// search stringpointer 'string' for word 'key' and replace it with 'newkey' and put the result 
//		in 'newstring'
//
// if FLAG is true then test if the 'key' is a word on its own or not else just blindly 
// search/replace the key
//
// If the destination is NULL, then we just replace the source, assuming the source is big
//   enough to handle the result.... we hope... (that is the requirement)
//
long ReplaceStr( char *srcstring, char *newstring, const char *key, const char *newkey, short wordflag )
{
	char	*oldkey, *p, *string, *ram, prevC=0, nextC=0, doit=TRUE;
	long	oldlen, len, ret=0, rep=0;

	oldlen = strlen( key );
	if ( !newstring )
		p = ram = (char *)malloc( 10000 );
	else 
		p = newstring;
	string = srcstring;

	if ( !p ) return 0;
	
	while ( oldkey = strstr( (char*)string, key ) ){
		len = oldkey - string;
		if ( wordflag ){
			if ( len )	prevC = *(oldkey-1);
			nextC = oldkey[oldlen];
			if ( prevC== '\"' || (isalpha( prevC ) || isalpha( nextC )) || nextC=='/' || nextC=='.' )
				doit = FALSE;
			else doit = TRUE;
		}
		if ( doit ){
			mystrncpy( p, string, (short)len );
			p += len;
			p += mystrcpy( p, newkey );
			string = oldkey + oldlen;
			ret++; rep++;
		} else {
			mystrncpy( p, string, (short)len );
			p += len;
			p += mystrncpy( p, oldkey, (short)oldlen );
			string = oldkey + oldlen;
			rep++;
		}
	}
	if ( ret )
		p += mystrcpy( p, string );
	*p = 0;

	if ( !newstring )
	{
		if ( ret )
			mystrcpy( srcstring, ram );
		free( ram );
	}
	return ret;
}


// added in rev 8
char *CopyLine( char *d, char *s )
{
	char* s_orig=s;

	assert( d );
	assert( s );

	while( *s && *s!='\r' && *s!='\n' )
	{
		*d++ = *s++;
	}

	*d='\0';

	while( *s=='\r' || *s=='\n' )
	{
		++s;
	}

	if( *s )
	{
		return s;
	}

	// mimic somewhat strange behavior of previous version of this function
	if( s!=s_orig && *(s-1)=='\n' )
	{
		*(s-1)='\0';
	}

	return 0;
}


short countchar(char *buffer,char ch)
{
	long 	n;			
	short c=0;		

	n=mystrlen(buffer);
	while (n--) {
		if ( *buffer == ch) {
			c++;
		}
		*buffer++;
	}
	return c;
}


// get substring enclosed in inverted commas ""
void getsub( char *src, char *dst )	
{
	int s=0;
	char c;
	
	if ( src && dst){
		while( c=*src ) {
			if (c=='"') {
				if (s==0) {
					s=1;
					src++;
				} else {
					s=0;
					*dst=0; //last " found so terminate and return
					return;
				}
			}
			if (s==1)
				*dst++=*src;
			src++;
		}
	}
}


void strip(char *src, char *dst)
{
	int i=0;
	char c;
	
	if ( src && dst){
		while ( c=*src ) {
			if (i>3) *dst++=c;
			i++;
			src++;
		}
		*dst=0;   //terminate string
	}
}


/************************************************************************************/
short pstrlen( unsigned char * str )				// p-string length function
{
	return( str[0] );
}


/************************************************************************************/
void pstrcpy( unsigned char * dst, unsigned char * src )	// p-string copy function
{
	short	i = *src;
	do {  dst[i] = src[i];  } while (i--);
}


void strcpyc2p(unsigned char *dst, char *src)
{
	short len=0;
	unsigned char *p=dst;
	
	if ( src && dst ) {
		p++;
		while(( *p++ = *src++ ) != 0) { len++; };
	}
	dst[0]=(unsigned char)len;
}


void strcatc2p( unsigned char * dst, char *src)
{
	short len=0;
	unsigned char *p=dst+dst[0];
	
	if ( src && dst ) {
		p++;
		while(( *p++ = *src++ ) != 0) { len++; };
	}
	dst[0]=(unsigned char)len+dst[0];
}


void strcpyp2c(char *dst, unsigned char * src)	
{
	short i=1,len=src[0];
	
	while (i<=len) {
		dst[i-1] = src[i];
		i++;
	}
	dst[i-1]=0;
}


void swaplong( long *l1, long *l2 )
{
	long x;

	if ( l1 && l2 ){
		x = *l1;

		*l1 = *l2;
		*l2 = x;
	}
}


long GetTimeFromFilename( char *filename )
{
	char *p = filename,c, numbers[32], num=0;
	long jd=0;

	while( c=*p ){
		if ( isdigit( c ) ){
			numbers[num++] = c;
		}
		if ( num>=6 ){
			long dd,mm,yy;
			char txt[32];

			mystrncpy( txt, &numbers[0], 2 ); txt[2]=0; dd = myatoi( txt );
			mystrncpy( txt, &numbers[2], 2 ); txt[2]=0; mm = myatoi( txt );
			mystrncpy( txt, &numbers[4], 2 ); txt[2]=0; yy = myatoi( txt );

			if ( dd>31 && yy<=31 ) swaplong( &dd, &yy );
			if ( dd<=31 && mm>12 && yy<=12 ) swaplong( &mm, &yy );

			//year 2000 compliant
			if (yy<70)
				yy+=2000;
			else if (yy<1000)
				yy+=1900;

			sprintf( txt, "%02d/%02d/%04d", dd,mm,yy );
			jd = DateStringToCTime( txt );
		}
		p++;
	}
	return jd;
}


//basicly convert this Long to a String
char *Long2Str( long val, long totdigit )
{
	long		num, digits=0, val2;
	unsigned char	temp[17], digit;

	val2 = val;

	if ( val < 0  )
		val = -val;

	//convert long into backword text
	for ( num=0; num<15; ) 
	{
		digits++; 
		digit = (unsigned char)(val%10);
		temp[num++] = digit+0x30;

		val = val / 10;
		if ( val == 0 )
			break;
	}
	// fill in preceding 0's
	if ( digits < totdigit ){
		for( ;digits<totdigit;){
			digits++;
			temp[num++] = '0';
		}
	}
	if ( val2 < 0 ) {
		digits++;
		temp[num++] = '-';
	}

	//copy backword text digits to output text...
	for ( num=0; num<digits; num++){
		tempReturnStr[num] = temp[ digits-(num+1) ];
	}
	tempReturnStr[num] = 0;

	return tempReturnStr;
}


//basicly convert this 64Long to a String
char *LLong2Str( __int64 *valPtr, long totdigit )
{
	__int64		val,val2;
	long		num, digits=0;
	short		digit;
	char		temp[32];

	val = *valPtr;
	val2 = val;
	if ( val < 0  ) val = -val;

	//convert long into backword text
	for ( num=0; num<20; ) {
		digits++; 
		digit = (short)(val %10);
		temp[num++] = digit+'0';

		val = val / 10;
		if ( val == 0 )
			break;
	}
	// fill in preceding 0's
	if ( digits < totdigit ){
		for( ;digits<totdigit;){
			digits++;
			temp[num++] = '0';
		}
	}
	if ( val2 < 0 ) {
		digits++;
		temp[num++] = '-';
	}

	//copy backword text digits to output text...
	for ( num=0; num<digits; num++){
		tempReturnStr[num] = temp[ digits-(num+1) ];
	}
	tempReturnStr[num] = 0;

	return tempReturnStr;
}



/* 1,234,567,890,123 */
long FormatNum( char *txt, long x)
{
	char		newStr[128], *p, *dot;
	int			intlen, len,count,idx,newlen=0;
	
	len=mystrcpy( newStr, txt );
	dot = strrchr( (char*)newStr, '.' );

	if ( dot ) intlen = dot-newStr; else intlen = len;

	p = txt;
	for( count=0; count < len ; count++ )
	{
		idx = (intlen - count);
		if ( count>0 && idx>0 )
		{
			if ( idx%3 == 0 )
			{
				*p++ = ',';
				newlen++;
			}
		}
		*p++ = newStr[count];
		newlen++;
		*(p+1)=0;
	}	
	return newlen;
}


long FormatLongNum( long n, char *txt )
{
	strcpy(txt,Long2Str( n, 0 ));
	return FormatNum(txt, 0);
}


long FormatLong64Num( __int64 n, char *txt )
{
	strcpy( txt,LLong2Str( &n, 0 ));
	return FormatNum(txt, 0);
}


long FormatDoubleNum( double n, char *txt )
{
	Double2CStr( n, txt,1);
	return FormatNum(txt, 0);
}


long ULong2CStr( unsigned long x, char *dest )
{
	if ( dest )
		return mystrcpy(dest,Long2Str( x, 0 ));
	return 0;
}


long Double2CStr( double x, char *dest, short type )
{
	if ( dest )
	{
		if( type )
			return sprintf( dest, "%.2f", x );
		else
			return sprintf( dest, "%.0f", x );
	}
	return 0;
}

// ********************************************************
// Compare 2 date strings.
// ********************************************************
int	mydatecmp(const char* szDate1, const char* szDate2)
{
	int		i;
	int		iDate1[3], iDate2[3];

	
	if (1)	// Month/Day/Year !
	{
		if (sscanf(szDate1, "%d/%d/%d", &iDate1[1], &iDate1[0], &iDate1[2]) != 3)
			return 0;
		if (sscanf(szDate2, "%d/%d/%d", &iDate2[1], &iDate2[0], &iDate2[2]) != 3)
			return 0;
	}
	else	// Day/Month/Year
	{
		if (sscanf(szDate1, "%d/%d/%d", &iDate1[0], &iDate1[1], &iDate1[2]) != 3)
			return 0;
		if (sscanf(szDate2, "%d/%d/%d", &iDate2[0], &iDate2[1], &iDate2[2]) != 3)
			return 0;
	}

	for (i=2;i>0;--i)
	{
		if (iDate1[i] > iDate2[i])
			return 1;
		if (iDate1[i] < iDate2[i])
			return -1;
	}

	return 0;
}

/************************************************************************************/
void pstrcat( unsigned char * dst, unsigned char * src )	// concatenate src with dst p-string
{
	short	i;
	for (i = 0; i < src[0]; )  
		dst[ ++dst[0] ] = src[++i];
}


/************************************************************************************/
size_t mystrlen( const char *str )				// c-string length function
{
	size_t i = 0;

	if ( str ){
		while( *str++ )
			i++;
	}
	return( i );

}


/************************************************************************************/
long mystrcpy( char *dst, const char *src )	// c-string copy function
{
	long len=0;
	
	if ( src && dst )
		while(( *dst++ = *src++ ) != 0){ len++; };
	return len;
}

void strcpybreak( char *dst, char *src, char brk, int len)	// c-string copy function
{
	char c;
	short cnt=0;
	
	if ( dst && src ) {
		while((c=*src) && cnt++<len) {
			if ( c == brk ) break;
			*dst++ = c;
			src++;
		}
		*dst++ = 0;
	}
}


long mystrcpylower( char *dst, const char *src )	// c-string copy function
{
	long len=0;
	
	if( dst && src )
	{
		while( *dst = *src++ )
		{
			*dst=mytolower(*dst);
			++dst;
			++len;
		};
	}
	return len;
}


//case insensitive
char * strstrn(const char * str, const char * pat, int n)
{
	unsigned char * s1 = (unsigned char *) str;
	unsigned char * p1 = (unsigned char *) pat;
	unsigned char firstc, c1, c2;
	
	if (!(firstc = tolower(*p1)))		/* if pat is an empty string we return str */
		return((char *) str);

	p1++;
	
	while(c1 = tolower(*s1)) {
		s1++;
		if (c1 == firstc)
		{
			const unsigned char * s2 = s1;
			const unsigned char * p2 = p1;
			short cnt=0;

			while ((c1 = tolower(*s2)) == (c2 = tolower(*p2)) && c1 && cnt++<n) {
				s2++;
				p2++;
			}
			
			if (!c2 || cnt==n-1)
				return((char *) s1 - 1);
		}
	}
	return(NULL);
}


char *strstri(const char * str, const char * pat )
{
	unsigned char * s1 = (unsigned char *) str;
	unsigned char * p1 = (unsigned char *) pat;
	unsigned char firstc, c1, c2;
	
	if ( str && pat ){
		if ( !(firstc = tolower(*p1)) )		/* if pat is an empty string we return str */
			return((char *) str);

		p1++;

		while(c1 = tolower(*s1) ){
			s1++;
			if (c1 == firstc)
			{
				const unsigned char * s2 = s1;
				const unsigned char * p2 = p1;
				short cnt=0;

				while ( (c1 = tolower(*s2) ) == (c2 = tolower(*p2) ) && c1 ){
					s2++;
					p2++;
				}
				
				if (!c2)
					return((char *) s1 - 1);
			}
		}
	}	
	return(NULL);
}


char *mystrstr(const char * str, const char * pat )
{
	unsigned char * s1 = (unsigned char *) str;
	unsigned char * p1 = (unsigned char *) pat;
	unsigned char firstc, c1, c2;
	
	if ( str && pat ){
		if ( !(firstc = *p1) )		/* if pat is an empty string we return str */
			return((char *) str);

		p1++;

		while(c1 = *s1 ){
			s1++;
			if (c1 == firstc)
			{
				const unsigned char * s2 = s1;
				const unsigned char * p2 = p1;
				short cnt=0;

				while ( (c1 = *s2 ) == (c2 = *p2 ) && c1 ){
					s2++;
					p2++;
				}
				
				if (!c2)
					return((char *) s1 - 1);
			}
		}
	}	
	return(NULL);
}


long mystrncpyNull( char *dst, const char *src, int max )	// c-string copy function
{
	long numCopied=mystrncpy( dst, src, max );

	dst[numCopied]=0;

	return numCopied;
}


long mystrncpy( char *dst, const char *src, int max )	// c-string copy function
{
	long numCopied=0;

	QS_PRECONDITION(dst);   
	QS_PRECONDITION(src);
	QS_PRECONDITION(max>=0);	// crappy signed integers

	if( dst && src )			// damn, still have to perform this check for old code compatability in release mode
	{
		while( numCopied<max && (*dst++=*src++) )
		{
			numCopied++;	
		}
	}

	return numCopied;
}


// copy string of N limit and turn to lower case.
long mystrncpylower( char *dst, const char *src, int max )
{
	long numCopied=0;

	QS_PRECONDITION(dst);   
	QS_PRECONDITION(src);
	QS_PRECONDITION(max>=0);	// crappy signed integers

	if( dst && src )			// damn, still have to perform this check for old code compatability in release mode
	{
		while( numCopied<max && (*dst++=tolower(*src) ) )
		{
			src++;
			numCopied++;	
		}
	}
	*dst = 0;

	return numCopied;
}



/************************************************************************************/
void mystrcat( char *dst, const char *src )	// concatenate src with dst c-string
{
	if ( src && dst ){
		while( *dst )
			dst++;
		while(( *dst++ = *src++ ) != 0){};
	}
}

long mystrncatNull( char *dst, const char *src, int dstSize )	// concatenate src with dst c-string, but knowing dst size
{
	char *p = dst;
	long len = 0;
	if ( src && dst )
	{
		while( *dst )
		{
			len++;
			dst++;
		}
		while( *src != 0 )
		{
			len++;
			if ( len > dstSize )
			{
				dst[dstSize-1] = 0;
#ifdef QS_DEBUG
				ErrorMsg( "Buffer too small" );
#endif
				return dstSize-1;
			}
			*dst++ = *src++;
		}
		*dst = 0;
	}
	return len;
}


/* mystrcmpi - compare two strings, ignoring case */
int mystrcmpi(const char *s1, const char *s2)
{
	QS_PRECONDITION(s1);	// don't remove!
	QS_PRECONDITION(s2);	// ditto

	if( s1 && s2 )	// for compatability with older code that assumes nulls can be passed
	{
		while( *s1 && *s2 && toupper(*s1)==toupper(*s2) )
		{
			s1++;
			s2++;
		}

		return toupper(*s1)-toupper(*s2);
	}

	return 0;
}

	
int	mystrncmpi(const char* s1, const char* s2, int iLen)
{
	char	c1, c2;

	QS_PRECONDITION(s1);	// don't remove!
	QS_PRECONDITION(s2);	// ditto


	for (;;)
	{
		if (iLen-- == 0)
			return 0;
		c1 = mytoupper(*s1);
		c2 = mytoupper(*s2);
		if (c1 < c2)
			return -1;
		if (c1 > c2)
			return 1;
		if (c1 == '\0')	/* implied [&& (c2 == '\0')] */
			return 0;
		++s1;
		++s2;
	}
}


int strcmpd(const char * str1, const char * str2)
{
	const	unsigned char * p1 = (unsigned char *) str1;
	const	unsigned char * p2 = (unsigned char *) str2;
	unsigned char		c1, c2;
	
	if ( p2 && p1 ){
		c1 = (*p1++);
		c2 = (*p2++);
		c1 = mytoupper(c1);
		c2 = mytoupper(c2);
		
		while ((c1) == (c2)){
			c1 = (*p1++);
			c2 = (*p2++);
			c1 = mytoupper(c1);
			c2 = mytoupper(c2);
			if (!c1)
				return(0);
		}
		return(c1 - c2);
	} else
		return 1;
}


// return the string after the nth char 'c' from the right
char *mystrrnchr(const char *s, char c, int n)
{
	char *sPtr, *retPtr=0;
	int cnt=0;
	
	if ( s ) {
		sPtr = (char*)s;
		sPtr += mystrlen(sPtr);

		while( sPtr >= s ){
			if ( *sPtr == c )
				cnt++;
			if ( cnt == n )
				retPtr = sPtr+1;
			sPtr--;
		}
	}
	return retPtr;
}


char *mystrchr(const char *s, char c)
{
	if ( s ) {
		do {
			if (*s == c)
				return (char *)s;
		} while (*s++);
	}
	return NULL;
}


char *mystrrchr(const char *s, int c)
{
	const char *res = NULL;

	if ( s ) {
		do {
			if (*s == c)
				res = s;
		} while (*s++);
		return (char *)res;
	} else return NULL;
}


// Finds the last character in the src string which is part of the srchChars string and
// copies the string up to this last character found into the dest string.
char *mystrrchrs( char *dest, const char *src, char *srchChars )
{
	const char *res = NULL;
	int lastCharPos = -1;
	char *cmpChar;
	int pos = 0;
	char *workingDest = dest;
	const char *origSrc = src;

	if ( src && dest )
	{
		do 
		{
			cmpChar = srchChars;
			while( *cmpChar )
			{
				if (*src == *cmpChar)
					lastCharPos = pos;
				cmpChar++;
			}
			*workingDest = *src;
			pos++;
			workingDest++;
		} while (*src++);
		//mystrncpy( dest, src, lastCharPos );
		//dest[lastCharPos] = 0;
		if ( lastCharPos != -1 )
			dest[lastCharPos] = 0;
		mystrcpy( dest, &origSrc[lastCharPos+1] ); 
		return dest;
	}
	else
		return "";
}


const char* revstrrchr( const char *s, const char *e, int c )
{
	if ( s && e ) {
		do {
			if (*e == c)
				return e;
			e--;
		} while ( e>=s );
	}
	return NULL;
}


char *mystrr2chr(const char *s, int c, int c2 )
{
	const char *res = NULL, *res2=NULL;

	if ( s ) {
		register char i;
		while( i=*s++ ){
			if ( i==c )
				res = s;
			if ( i==c2 )
				res2 = s;
		}
		if ( !res ) res=res2;
		return (char *)res;
	} else return NULL;
}


/* strcmpiPart - compare two strings, ignoring case, and stopping at the end of str1 */
short strcmpiPart(char *str1, char *str2)
{
	char c1,c2;
	
	if ( str1 && str2 ){
		for (;; ++str1, ++str2) {
			c1 = mytoupper(*str1);
			c2 = mytoupper(*str2);
			if (!c1) break;
			if (c1 < c2) return(-1);
			if (c1 > c2) return(1);
		}
	}
	return(0);
}


long strcmpMany( char *string, char **comps )
{
	char *compare;

	while( (compare=*comps) ){
		if ( strcmpiPart( compare, string ) == 0 )
			return 0;
		comps++;
	}
	return 1;
}



// copy from string s2 the X words (until Y char) into string s1
// ie, copy X amount of words based on Y delimeter
// return words done.
size_t strcpyx( char *s1, char *s2, char brkch, size_t words )
{
	char c;
	size_t cnt=0;
	
	if ( s1 && s2 ){
		while( (c=*s2) && words>0 ){
			if ( c == brkch ) { words--; cnt++;}
			*s1++ = c;
			s2++;
		}
		*s1++ = 0;
	}
	return cnt;
}


// size limited version
// copy from string s2 the X words (until Y char) into string s1
// ie, copy X amount of words based on Y delimeter
// return words done.
size_t strncpyx( char *s1, char *s2, 
			  char brkch,		// break character
			  size_t words,		// amount of breaks to allow
			  size_t maxbuf		// s1 max buffer size.
			  )
{
	char	c;
	size_t		cnt=0, nchars=0;
	
	if ( s1 && s2 )
	{
		while( (c=*s2) && words>0 && nchars<maxbuf)
		{
			if ( c == brkch ) { words--; cnt++;}
			*s1++ = c;
			s2++;
			nchars++;
		}
		*s1++ = 0;
	}
	return cnt;
}


// break a string after 'words' steps using 'brkch' as a seperator
int breakatx( char *s2, char brkch, int words )
{
	char c;
	int cnt=0;
	
	if ( s2 ){
		while( (c=*s2) && words>0 ){
			if ( c == brkch ) { words--; cnt++;}
			if ( !words ) *s2 = 0;
			s2++;
		}
		*s2++ = 0;
	}
	return cnt;
}


// add Browser version info to s1 from s2
int strcpybrk( char *s1, const char *s2, char brkch, int words )
{
	char c;
	int cnt=0;
	
	if ( s1 && s2 ){
		while( (c=*s2) && words>0 ){
			if ( c == brkch ) { words--; cnt++;}
			if ( words>0 ){
				*s1++ = c;
				s2++;
			}
		}
		*s1++ = 0;
		if ( cnt == 1 ) cnt = 0;
	}
	return cnt;
}


// 990415:	new function added, this will copy the domain part of a URL to the dest , stoping at : or / at the end and
//			skipping the http crap and www3 stuff
//			copy http://www.domain.com:80/ so we get domain.com
int strcpydomain( char *d, char *src )
{
	char	c, *s = src;
	int		cnt=0, done=0, checkCount=0,l=0;
	char	check[]= { "http://\0" };
	
	if ( s && d ){
		char	waitfor=0;
		while( (c=*s) && done==0){
			if ( waitfor == 0 ){
				if ( c == check[cnt] ){ 
					checkCount++;
					cnt++;
				} else waitfor = 1;
			}
			if ( waitfor == 1 ){
				if ( s[0] == 'w' && s[1] == 'w' && s[2] == 'w' ){
					waitfor = 2; //skip until .
				} else
					waitfor = 3;
			}
			if ( waitfor == 2 ){
				if ( c == '.' )
					waitfor = 3;
			} else 
			if ( waitfor != 0 ){
				if ( c==':' || c=='/' || c=='\\')
					done = 1;
				else {
					*d++ = c;
					l++;
				}
			}
			s++;
		}
		*d++ = 0;
	}
	return l;
}



int getVersion( char *s1, char *s2, int max)
{
	if (!s1 || !s2)
		return 0;

	for (;*s2; ++s2)
	{
		if (isdigit(*s2))
		{
			int	iPos = 0;
			int iTmp;
			if (sscanf(s2, "%*d.%n%1d", &iPos, &iTmp) == 1 && iPos>0)
			{
				++iPos;	// for the digit.
				iPos = (max<iPos) ? max:iPos;	// Truncate the version number if needed.
				strncpy(s1, s2, iPos);
				*(s1+iPos) = 0;
				return 1;
			}
		}
	}

	return 0;
}


// copy from string s2 the first word (until space or null is reached) into string s1
int strcpywn( char *s1, char *s2, int words )
{
	char c;
	int cnt=0;
	
	if ( s1 && s2 ) {
		while( (c=*s2) && words>0) {
			if ( c == ' ' ) words--;
			*s1++ = c;
			s2++;
			cnt++;
		}
		*s1++ = 0;
	}
	return cnt;
}


//get first word separated by /, space or null
int strcpybr( char *s1, char *s2 )
{
	char c;
	int cnt=0;
	short word = 1;
	
	if ( s1 && s2 ) {
		while( (c=*s2) && (word>0) && cnt<512 ) {
			if ( c == ' ' || c== '/') {
				c= ' ';
				word--;
			}
			*s1++ = c;
			s2++;
			cnt++;
		}
		*s1++ = 0;
	}
	return cnt;
}


/* Checks output file paths to truncate any directory names longer than 31 characters	*/
/* Adds a tilde (~) in the 31st position if name is longer								*/
/* Hard-coded for colon (:) delimiter at present, otherwise just add a parameter		*/
void TrimPathForClassicMacOS (char *pathString)
{
#if QS_MAC
	char		*inputStr = pathString;
	char		retStr[512] = "";
	char		tempStr[64] = "";
	char		tokStr[1];
	short		length = 0;
	
	sprintf (tokStr, ":");

	mystrcpy (tempStr, strtok(inputStr, tokStr));

	while (tempStr[0] != 0)
	{
		length = mystrlen (tempStr);
		if (length > 31)
		{
			tempStr[30] = '~';
			tempStr[31] = '\0';
		}

		mystrcat (retStr, tempStr);
		mystrcat (retStr, ":");
		tempStr[0] = '\0';
		mystrcpy (tempStr, strtok(NULL, tokStr));
	}

	length = mystrlen (retStr);
	retStr[length - 1] = '\0';				// kill last ':' at end of output string
	
	mystrcpy (pathString, retStr);
#endif
}


// copy from string s2 the first word (until space or null is reached) into string s1
int strcpyw( char *s1, char *s2 )
{
	return strcpywn( s1,s2,1 );
}



//get first word separated by /, space or null
int strcpyuntil( char *s1, char *s2, char until )
{
	char c;
	int cnt=0;
	short word = 1;
	
	if ( s1 && s2 ) {
		while( (c=*s2) && (word>0) && cnt<512 ) {
			if ( c == until ) {
				c=0;
				word--;
			}
			*s1++ = c;
			s2++;
			cnt++;
		}
		*s1++ = 0;
	}
	return cnt;
}

// As above but with bounds-checking.
size_t
strncpyuntil(char* sz, const char* csz, size_t n, char c)
{
	const char* s = sz;
	while (--n>0 && *csz && *csz!=c)
	{
		*sz++ = *csz++;
	}
	*sz = 0;
	return sz-s;
}



char *CheckWWWprefix( char *host )
{
#if _SUNOS
	if ( !mystrcmpi( "www.", host ) )
		return host+4;
#else
	char x1[]= {"www."}, x2[]= {"WWW."};			// This stuff just crashes 64bit CPUs
	long c2;
	
	c2 = *(long*)host;
	if( *(long*)x1==c2 ) return host+4;
	if( *(long*)x2==c2 ) return host+4;
#endif	
	return 0;
}


/* made destination optional (RS,30MAR98) */
int PathFromFullPath( char *fullpath, char *path )
{
	char	*sp, *destpath;

	if ( path ) {
		destpath = path;
		mystrcpy( destpath, fullpath );
	} else
		destpath = fullpath;

	sp = mystrrchr( destpath, PATHSEP );
	if (!sp)
		sp = mystrrchr( destpath, OTHER_OS1_PATHSEP );
	if (!sp)
		sp = mystrrchr( destpath, OTHER_OS2_PATHSEP );
	if ( sp ) {
		sp++;
		*sp = 0;
		return ( sp - destpath );
	} else {
		*destpath = 0;
		return 0;
	}
}


int PathFromURL( char *url, char *path )
{
	char *sp, *destpath;

	if ( path ) {
		destpath = path;
		mystrcpy( destpath, url );
	} else
		destpath = url;

	sp = mystrrchr( destpath, '/' );
	if (!sp)
		sp = mystrrchr( destpath, OTHER_OS1_PATHSEP );
	if (!sp)
		sp = mystrrchr( destpath, OTHER_OS2_PATHSEP );
	if ( sp ) {
		//  "/dir/dir2/file.gif"  chop the file off
		if ( mystrchr( sp, '.' ) ){
			sp++;
			*sp = 0;
			return ( sp - destpath );
		} else
		if ( *(sp+1) ) {			// "dir/dirfolder"  attach a / at the end
			long len=strlen( destpath );
			destpath[len] = '/';
			destpath[len+1] = 0;
			return len+1;
		} else
			return strlen( destpath );
	} else {
		*destpath = 0;
		return 0;
	}
}



char HexToChar( char *txt )
{
	char c1,c2;

	c1 = *txt;
	txt++;
	c2 = *txt;

	if ( c1<= '9' ) c1 -= '0'; else c1 -= ('A'-10);
	if ( c2<= '9' ) c2 -= '0'; else c2 -= ('A'-10);

	return c1<<4 | c2;
}


void DecodeURLString( char *url, char *dest )
{
	char c;

	while( c=*url ){
		if ( c == '%' )	{
			c = HexToChar(url+1);
			url+=2;
		}
		*dest++ = c;
		url++;
	}
	*dest++ = 0;
}


char *Path2FilePath( char *path)
{
	register char *ptr=path, c;
	
	if ( path ) {
		while (c=*path) {
			if (c==':' || c=='\\')
				*path='/';
			path++;
		}
	}
	return ptr;
}


void makepath( char *path, char *oldpath)
{
	char *ptr=path;
	
	if ( path && oldpath) {
		if (*oldpath!=':' && *oldpath!='\\' && *oldpath!='/') {
			*path='/';
			path++;
		}
		while (*oldpath) {
			if (*oldpath==':' || *oldpath=='\\')
				*path='/';
			else
				*path=*oldpath;
			oldpath++;
			path++;
		}
		if (*(oldpath-1)!=':' && *(oldpath-1)!='\\' && *(oldpath-1)!='/') {
			*path='/';
			path++;
		}
		*path=0;
	}
}


void CopyFilenameUsingPath( char *destPath, char *pathfile, char *filename )
{
	char	thepath[512];
	
	PathFromFullPath( pathfile, thepath );
	mystrcpy( destPath, thepath );
	mystrcat( destPath, filename );
}


char *FileFromPath(const char *fullpath, char *outfile )
{
	char *sp;

	sp = mystrrchr( fullpath, PATHSEP );
	if (!sp)
		sp = mystrrchr( fullpath, OTHER_OS1_PATHSEP );
	if (!sp)
		sp = mystrrchr( fullpath, OTHER_OS2_PATHSEP );
	if ( sp ) {
		if ( outfile )
			mystrcpy( outfile, sp+1 );

		return sp+1;
	} else {
		if ( outfile )
			mystrcpy( outfile, (char*)fullpath );

		return (char*)fullpath;
	}

}


// trims off trailing returns and newlines from a string
void trimLine( char* line )
{
	QS_PRECONDITION(line);

	// scope to allow me to declare a variable here in C
	{
		size_t len=strlen(line);

		if( len )
		{
			char* end=line+len-1;

			while( end>=line && (*end=='\r' || *end=='\n') )
			{
				*(end--)='\0';
			}
		}
	}
}


// trims off trailing returns, newlines, tabs and spaces from a string
void trimLineWhite( char* line )
{
	QS_PRECONDITION(line);

	// scope to allow me to declare a variable here in C
	{
		size_t len=strlen(line);

		if( len )
		{
			char* end=line+len-1;		// will be -1 for empty string but this is ok

			while( end>=line && (*end==' ' || *end=='\r' || *end=='\n') )
			{
				*(end--)='\0';
			}
		}
	}
}


void ColonToSlash(char *instring)
{
	char c;
	while (c=*instring) {
		if (c==':' || c=='\\') {
			*instring='/';
		}
		instring++;
	}
}


void EqualToTab(char *instring)
{
	char c;
	while (c=*instring) {
		if (c=='=') {
			*instring='\t';
		}
		instring++;
	}
}


void TabToEqual(char *instring)
{
	char c;
	while (c=*instring) {
		if (c=='\t') {
			*instring='=';
		}
		instring++;
	}
}


void trimString(char *instring, int size)
{
	short theLen;
	char	*p;
	
	theLen=mystrlen(instring);
	if (theLen>size) {
		p = instring + size;
		if( *p ) *p++ = '.';
		if( *p ) *p++ = '.';
		if( *p ) *p++ = '.';
		if( *p ) *p++ = 0;
	} else
		return;
}


void stripxchar(char *instring, char x)
{
	short theLen;

	theLen=mystrlen(instring);
	if (instring[theLen-1]==x) {
		instring[theLen-1]=0;
	}
	return;
}


void stripURL(char *instring)
{
	short theLen;
	
	theLen=mystrlen(instring);
	if (instring[theLen-1]=='/' && instring[theLen-2]!='/') {
		instring[theLen-1]=0;
	}
	return;
}


//case insensensitive
void stripReferURL(char *instring, char *outstring)
{
	char	*pos;
	
	if (instring && outstring) {
		if (pos=strstr(instring,"//"))
		{
			pos+=2;
			mystrcpylower(outstring,pos);			// this works for "http://www.site.com"
		}
		else
			mystrcpylower (outstring, instring);	// and this works for "www.site.com"
	}
}


void createURL(char *dest, char *src)
{
	if ( src && dest ) {
		while(*src) {
			switch (*src) {
				case ':':
					*dest++='/';
					break;
				case ' ':
					*dest++='%';
					*dest++='2';
					*dest++='0';
					break;				
				case '/':
					*dest++='%';
					*dest++='3';
					*dest++='A';
					break;
				default:
					*dest++ = *src;
					break;
			}
			src++;
		}
		*dest=0;   //terminate string
	}
}


int stripType(char *dest, char *src)
{
	char c;
	
	if ( src && dest ) {
		while(c=*src) {
			if (!isalnum(c)) {
				*dest++=0;
				return 1;
			}
			*dest++ = c;
			src++;
		}
	}
	return 0;
}


void SetFilePrefix(char *srcFile, char *prefix)
{
	char	filename[512];
	char	newfilename[512];

	FileFromPath( srcFile, filename);
	StrAdd( newfilename, prefix, filename );
	CopyFilenameUsingPath( filename, srcFile, newfilename );
	mystrcpy( srcFile, filename );

}


void SetFileExtension(char *srcFile, const char *extn)
{
	char	*p;

	p=strrchr(srcFile,'.');
	if (p) {
		*p=0;
		strcat(srcFile,extn);
	}
}


void *mymemset(void *_s, int c, int n)
{
	char *s = (char *)_s;

	if ( s ) {
		while (n--)
			*s++ = c;
		return _s;
	}
	return NULL;
}


/*----------------------------------------------------------------------------
	CrackNum 
	
	Crack a decimal number from a string after skipping white space.
	
	Entry:	*x = pointer to string.
			
	Exit:	function result = cracked number.
			*x = pointer to first character following number in string.
----------------------------------------------------------------------------*/
long myCrackNum (char **x)
{	
	long result = 0;
	char *y;
	
	if ( x ){
		y = *x;
		while (myisspace(*y)) y++;
		while (myisdigit(*y)) {
			result = 10*result + (*y - '0');
			y++;
		}
		*x = y;
	}
	return result;
}


// convert the Integer into a 3digit String
int Int2Str( long num, char *str)
{
	unsigned char digit[3],pos;
	
	// 0-1-2
	digit[2] = (unsigned char)(num%10);
	digit[1] = (unsigned char)((num/10)%10);
	digit[0] = (unsigned char)((num/100)%10);

	pos = 0;
	
	if ( digit[0] )
		str[pos++] = digit[0]+0x30;

	if ( digit[1] || digit[0] )
		str[pos++] = digit[1]+0x30;

	str[pos++] = digit[2]+0x30;

	return pos;
}



// add to strings to 'd' string...
char  *StrAdd( char *d, char *s1, char *s2 )
{
	if ( d ){
		if ( s1 ){
			while( *d = *s1++ ) d++;
			//d += mystrcpy( d, s1 );
			//d += mystrlen( s1 );
		}
		if ( s2 ){
			while( *d = *s2++ ) d++;
			//d += mystrcpy( d, s2 );
			//d += mystrlen( s2 );
		}
	}
	return d;
}



void Ptr2HexStr( void *ptr, char *str )
{
	long	num, addr, digit;
	char	hex[]="0123456789ABCDEF";

	if ( ptr && str ){
		addr = (long)ptr;

		for ( num=7; num >=0; num--)
		{
			digit = addr & 0xf;
			str[num] = hex[digit];
			addr >>= 4;
		}
		str[8] = 0;
	}
}



char *Long2HexStr( long val )
{
	Ptr2HexStr( (void*)val, tempReturnStr );
	return tempReturnStr;
}


long HexStr2int( const char *str )
{
	long	num, addr=0;
	short	digit;
	char	hex[]="0123456789ABCDEF";
	
	if ( str ){
		for ( num=0; num < 8; num++){
			digit = toupper(str[num]);
			if ( digit>='A' && digit<='F')
				digit = digit - ('A' - 0xA);
			else
			if ( digit>='0' && digit<='9')
				digit = digit - '0';
			else
				return addr;
			addr <<= 4;
			addr |= digit;
		}
	}
	return addr;
}


void *HexStr2Ptr( char *str )
{
	return (void*)HexStr2int( str );
}


unsigned long myatoint( const char *text, long *num )
{
	unsigned long	retvalue=0, digits=0;
	register char	cv;
	
	if ( text ) {
		digits=0;
		while( *text ){
			cv = *text++;
			if ( cv == ' ' ) continue;
			if ( cv<'0' || cv>'9' ) break;
			retvalue = (retvalue*10) + (cv-'0');
			digits++;
		}
		if ( num )
			*num = digits;
	}
	return retvalue;
}


//given an IP address and Port numer this routine strips 
//out the Port information and returns the new strings length
short stripPort(char * str, short len, char chr)
{
	char *p = &str[len];
	short cnt=1;
	
	while (*p && cnt<7) {
		if (*p==chr) {
			*p = 0;
			return len-cnt;
		}
		cnt++; *p--;
	}
	return len;
}


unsigned long myatoi( const char *text )
{
	unsigned long x=0;
	
	if ( text ){
		long r;
		x = myatoint( text, &r );
	}
	
	return x;
}



long IsGzFP( FILE *fp )
{
	short	c;
	short	word;

	if ( fp ){
		c = (short)fgetc( fp );	word = (c);
		word <<= 8;
		c = (short)fgetc( fp );	word |= (c);
	}
	if ( word == 0x1f8b )
		return 1;
	else
		return 0;

}


long IsGzFile( char *fileName )
{
	FILE 	*fp;
	long	ret;
	
	if ( (fp = fopen( fileName, "rb" )) ) {
		ret = IsGzFP( fp );
		fclose( fp );
	}
	else
		return 0;
	return ret;
}


long IsBzFP( FILE *fp )
{
	short	c;
	short	word;

	if ( fp ){
		c = (short)fgetc( fp );	word = (c);
		word <<= 8;
		c = (short)fgetc( fp );	word |= (c);
	}
	if ( word == 0x425a )
		return 1;
	else
		return 0;

}


long IsBzFile( char *fileName )
{
	FILE 	*fp;
	long	ret;
	
	if ( (fp = fopen( fileName, "rb" )) ) {
		ret = IsBzFP( fp );
		fclose( fp );
	}
	else
		return 0;
	return ret;
}


long GetFPLength( FILE *fp )
{
	long len=0;

	if ( fp ) {
		fseek( fp, 0, SEEK_END );
		len = ftell( fp );
		fseek( fp, 0, SEEK_SET );
	}
	return len;
}


long GetGzFPLength( FILE *fp )
{
	long	len =0;
	long	c;
	long	gzlen = 0;
	
	if ( fp ) {
		fseek( fp, -4, SEEK_END );
		c = (long)fgetc( fp );	gzlen += (c);
		c = (long)fgetc( fp );	gzlen += (c<<8);
		c = (long)fgetc( fp );	gzlen += (c<<16);
		c = (long)fgetc( fp );	gzlen += (c<<24);

		// if the length is bogus make it big
		if ( gzlen <0 ) gzlen = 0x7fffffff;
	}
	return gzlen;
}


long GetGzFileLength( char *fileName )
{
	FILE 	*fp;
	long	len =0;
	long	gzlen = 0;

	if ( (fp = fopen( fileName, "rb" )) ) {
		gzlen = GetGzFPLength( fp );
		fclose( fp );
	}
	return gzlen;
}


long GetBzFPLength( FILE *fp )
{
	long	len =0;
	long	c;
	long	orglen = 0;
	
	if ( fp ) {
		//return GetFPLength(fp);
		fseek( fp, -8, SEEK_END );
		fread( (void*)&c, 4, 1, fp );
		if ( c == 0 ){
			fread( (void*)&orglen, 4, 1, fp );
		} else
			orglen = GetFPLength(fp);
	}
	return orglen;
}


long GetBzFileLength( char *fileName )
{
	FILE 	*fp;
	long	len =0;
	long	gzlen = 0;

	if ( (fp = fopen( fileName, "rb" )) ) {
		gzlen = GetBzFPLength( fp );
		fclose( fp );
	}
	return gzlen;
}


#ifdef QS_WINDOWS
__int64 WinGetFileLength( char *filename )
{
	OFSTRUCT ReOpenBuff;
	HFILE hFile;

	hFile = OpenFile( filename, &ReOpenBuff, OF_READ );
	if ( hFile ){
		unsigned long	upper,lower;
		__int64	fs;

		lower = GetFileSize( (HANDLE)hFile, &upper );

		fs = (__int64)upper<<32 | lower;

		_lclose( hFile );
		//OutDebugs( "size=%08x, %08x, i64=%lld", fs.LowPart, fs.HighPart, fs.QuadPart );
		return fs;
	}
	return 0;
}
#endif


//00000000  52 61 72 21 1A 07 00 3B D0 73 08 00 0D 00 00 00    Rar!...;Ðs......
__int64  GetRealFileLength( char *fileName )
{
	FILE 	*fp = 0;
	__int64 	len=0;
	
	if ( (fp = fopen( fileName, "rb" )) ) 
	{
#ifdef QS_WINDOWS
		len = WinGetFileLength( fileName );
#elif QS_UNIX
		fseek( fp, 0, SEEK_END );
		len = ftello( fp );
#else
		fseek( fp, 0, SEEK_END );
		len = ftell( fp );
#endif
		fclose( fp );
	}
	return len;
}



__int64  GetFileLength( char *fileName )
{
	FILE 	*fp = 0;
	__int64 	len=0;

	long	c1,c2;
	if ( IsGzFile( fileName )  )
		len = GetGzFileLength( fileName );
	else
	if ( IsBzFile( fileName ) )
		len = GetBzFileLength( fileName );
	else
	if ( (fp = fopen( fileName, "rb" )) ) {
		c1 = fgetc( fp );
		c2 = fgetc( fp );
		if ( (c1 == 0x1f) && (c2 == 0x8b) ){
			len = GetGzFileLength( fileName );
		} else
		{
#ifdef QS_WINDOWS
			len = WinGetFileLength( fileName );
#elif QS_UNIX
			fseek( fp, 0, SEEK_END );
			len = ftello( fp );
#else
			fseek( fp, 0, SEEK_END );
			len = ftell( fp );
#endif
		}
		fclose( fp );
	}
	return len;
}


long IsFilePKZIP( const char *fileName )
{
	FILE 	*fp;
	long	len=0;
	long	c1,c2;
	
	if ( (fp = fopen( fileName, "rb" )) ) {
		c1 = fgetc( fp );
		c2 = fgetc( fp );

		if ( (c1 == 'P') && (c2 == 'K') ){
			len = 1;
		} else {
			len = 0;
		}
		fclose( fp );
	}
	return len;
}


long IsFileStuffit( const char *fileName )
{
	FILE 	*fp;
	
	if ( (fp = fopen( fileName, "rb" )) ) {
		char txt[256];

		fread( txt, 1, 80, fp);
		fclose( fp );

		if ( strstr( txt, "Stuffit" ) )
			return 1;
	}
	return 0;
}


long IsFileInvalid( const char *fileName, char *format )
{
	FILE 	*fp;
	
	if ( (fp = fopen( fileName, "rb" )) ) {
		unsigned char txt[256];

		fread( txt, 1, 80, fp );
		fclose( fp );

		if ( strstr( (const char *)txt, "StuffIt (c)" ) ){
			sprintf( format, "Stuffit Archive" );
			return 1;
		}
		else
		if ( strstr( (const char *)txt, "converted with BinHex" ) ){
			sprintf( format, "Mac BINHEX" );
			return 1;
		}
		else
		if ( txt[0] == 0x4D && txt[1] == 0x5A ){
			sprintf( format, "Windows Executable" );
			return 1;
		}
		else
		if ( txt[0] == 0xFF && txt[1] == 0xFB ){
			sprintf( format, "MP3" );
			return 1;
		}
		else
		if ( txt[0] == 0xED && txt[1] == 0xAB && txt[2] == 0xEE && txt[3] == 0xDB ){
			sprintf( format, "Linux RPM" );
			return 1;
		}
		else
		if ( txt[0] == 0x52 && txt[1] == 0x61 && txt[2] == 0x72 && txt[3] == 0x21 ){
			sprintf( format, "RAR Archive" );
			return 1;
		}
	}
	return 0;
}


long FileToMem( const char *filename, void *data, long limitLen )
{
	FILE *fp;
	long	len=0;

	if ( fp = fopen( filename, "rb" ) ) {
		len = fread( data, 1, limitLen, fp );
		fclose( fp );
	}

	return len;
}


long MemToFile( char *filename, void *data, long limitLen )
{
	FILE *fp;
	long	len=0;
	if ( fp = Fopen(filename, "wb" )) {
		len = fwrite( data, 1, limitLen, fp );
		fclose( fp );
	}
	return len;
}


void ClearRam( long *p, long len )
{
	register long	lp=len>>4, l2=len&0xf, Z=0;
	while(lp) {
		*p++ = Z;	*p++ = Z;	*p++ = Z;	*p++ = Z;	lp--;
	}
	{ register char *p2=(char*)p;
		while(l2) { *p2++=0; l2--; }
	}
}


void ClearLongs( long *p, long len )
{
	register long	Z=0;
	while(len) {
		*p++ = Z;
		len--;
	}
}


typedef struct TSortInfo {
	long size;
	SortCmpFunction cmp;
} TSortInfo;


void MemSwap( register char *sc, register char *dc, register long len )
{
	register char tmp;
	register long	*s, *d, lp=len>>3, l2=len&0x7, t;
	
	if (sc==dc) return;
	s = (long *)sc;
	d = (long *)dc;
	while (lp--) {
		t = *s;  *s++ = *d;  *d++ = t;
		t = *s; * s++ = *d;  *d++ = t;
	}
	sc = (char *)s;
	dc = (char *)d;
	while (l2--) {
		tmp = *sc;  *sc++ = *dc;  *dc++ = tmp;
	}
}


static long ChooseMedian (void *pDest, void *pA, void *pB, void *pC, TSortInfo *sortInfo)
{
	register void *pMedian;
	register SortCmpFunction cmp;
	long result;
	long err;
	
	cmp = sortInfo->cmp;
	err = (*cmp)(pA, pB, &result);
	if (err != 0) return err;
	if (result > 0) {						/* pA > pB, what is pC?			*/
		err = (*cmp)(pA, pC, &result);
		if (err != 0) return err;
		if (result > 0) {					/* pA > pB, pA > pC				*/
			err = (*cmp)(pB, pC, &result);
			if (err != 0) return err;
			if (result > 0)
				pMedian = pB;				/* pA > pB > pC					*/
			else
				pMedian = pC;				/* pA > pC > pB					*/
		} else {
			pMedian = pA;					/* pC > pA > pB					*/
		}
	} else {								/* pB > pA, what is pC?			*/
		err = (*cmp)(pA, pC, &result);
		if (err != 0) return err;
		if (result > 0) {
			pMedian = pA;					/* pB > pA > pC					*/
		} else {							/* pB > pA, pC > pA				*/
			err = (*cmp)(pB, pC, &result);
			if (err != 0) return err;
			if (result > 0)
				pMedian = pC;				/* pB > pC > pA					*/
			else
				pMedian = pB;				/* pC > pB > pA					*/
		}
	}
	if (pDest != pMedian) MemSwap((char *)pDest, (char *)pMedian, sortInfo->size);
	return 0;
}


static long DoSegFastQSort (register char *first, char *last, TSortInfo *sortInfo )
{
	register SortCmpFunction cmp;
	register char *i, *j;
	register long n, size;
	long err;
	long result;
	 	
	size = sortInfo->size;
	cmp = sortInfo->cmp;
	while ((n = (last - first) / size) > 1) {
		if (n < 25) {
			MemSwap(first + (rand() % n) * size, first, size);
		} else {
			err = ChooseMedian(first, first, first + (n / 2) * size, 
				first + (rand() % n) * size, sortInfo);
			if (err != 0) return err;
		}
		i = first;
		j = last;
		while (1) {
			i += size;
			while (i < last) {
				err = (*cmp)(i, first, &result);
				if (err != 0) return err;
				if (result >= 0) break;
				i += size;
			}
			j -= size;
			while (j > first) {
				err = (*cmp)(j, first, &result);
				if (err != 0) return err;
				if (result <= 0) break;
				j -= size;
			}
			if (i >= j) break;
			MemSwap(i, j, size);
		}
		if (j == first) {
			first += size;
			continue;
		}
		MemSwap(first, j, size);
		if (j - first < last - (j + size)) {
			err = DoSegFastQSort(first, j, sortInfo);
			if (err != 0) return err;
			first = j + size;
		} else {
			err = DoSegFastQSort(j + size, last, sortInfo);
			if (err != 0) return err;
			last = j;
		}
	}
	return 0;
}


static long DoFastQSort (register char *first, char *last, TSortInfo *sortInfo, short sortOrder )
{
	register SortCmpFunction cmp;
	register char *i, *j;
	register long n, size;
	long err;
	long result;

	size = sortInfo->size;
	cmp = sortInfo->cmp;
	while ((n = (last - first) / size) > 1) {
		if (n < 25) {
			MemSwap(first + (rand() % n) * size, first, size);
		} else {
			err = ChooseMedian(first, first, first + (n / 2) * size, 
				first + (rand() % n) * size, sortInfo);
			if (err != 0)
				return err;
		}
		i = first;
		j = last;
		while (1) {
			i += size;
			while (i < last) {
				err = (*cmp)(i, first, &result);
				if (err != 0)
					return err;
				if ( sortOrder ) {  // TOP_SORT
					if (result >= 0)
						break;
					i += size;
				} else { // BOTTOM_SORT
					if (result < 0)
						break;
					i += size;
				}
			}
			j -= size;
			while (j > first) {
				err = (*cmp)(j, first, &result);
				if (err != 0)
					return err;
				if ( sortOrder ) {  // TOP_SORT
					if (result <= 0)
						break;
					j -= size;
				} else { // BOTTOM_SORT
					if (result > 0)
						break;
					j -= size;
				}
			}
			if ( sortOrder ) {  // TOP_SORT
				if (i >= j)
					break;
				MemSwap(i, j, size);
			} else { // BOTTOM_SORT
				if (i >= j)
					break;
				MemSwap(j, i, size);
			}
		}
		if (j == first) {
			first += size;
			continue;
		}
		if ( sortOrder ) {  // TOP_SORT
			MemSwap(first, j, size);
		} else { // BOTTOM_SORT
			MemSwap(j, first, size);
		}
		if (j - first < last - (j + size)) {
			err = DoFastQSort(first, j, sortInfo, sortOrder );
			if (err != 0)
				return err;
			first = j + size;
		} else {
			err = DoFastQSort(j + size, last, sortInfo, sortOrder );
			if (err != 0)
				return err;
			last = j;
		}
	}
	return 0;
}


long FastQSort (void *base, long n, long size, SortCmpFunction cmp, short sortOrder )
{
	TSortInfo sortInfo;
	
	srand(2002);
	
	sortInfo.cmp = cmp;
	sortInfo.size = size;
	
	return DoFastQSort( (char *)base, (char*)base + n * size, &sortInfo, sortOrder );
}


short IsCountryUSA( void )
{
#if	QS_WINDOWS
	{
static		char txt[129]="\0";
		
		if ( txt[0] == 0 )
			GetLocaleInfo( LOCALE_USER_DEFAULT , LOCALE_SENGCOUNTRY , txt, 128 );
		if ( txt[0] == 'U' && txt[0] == 'S' )
			return 1;
		else
			return 0;
	}
#endif

#if QS_UNIX
	return 1;
#endif
	return 1;
}



/* returns the IP as a string so 0x01020304 = 1.2.3.4 */
unsigned long ipstringtolong( const char *ipstr )
{
	long			pos=0,ret;
	unsigned long	ip1,ip2,ip3,ip4;
	unsigned long	ipnum;
	
	if( ipstr && isdigit(*ipstr) ){
		ip1 = myatoint( ipstr+pos, &ret); pos+= ret; if ( *(ipstr+pos) != '.' ) return 0;
		ip2 = myatoint( ipstr+pos, &ret); pos+= ret; if ( *(ipstr+pos) != '.' ) return 0;
		ip3 = myatoint( ipstr+pos, &ret); pos+= ret; if ( *(ipstr+pos) != '.' ) return 0;
		ip4 = myatoint( ipstr+pos, &ret);
		
		ipnum = ((ip1&0xff)<<24) | ((ip2&0xff)<<16) | ((ip3&0xff)<<8) | (ip4&0xff);
	} else
		ipnum = 0;
	return ipnum;
}


unsigned long IPStringToLong( const char *ipstr )
{
	long			pos=0,ret;
	unsigned long	ip1,ip2,ip3,ip4;
	unsigned long	ipnum, ip=0, ips[4]={0,0,0,0}, i = 0;
	

	ip1 = myatoint( ipstr+pos, &ret); pos+= ret; if ( *(ipstr+pos) != '.' ) return 0;
	ip2 = myatoint( ipstr+pos, &ret); pos+= ret; if ( *(ipstr+pos) != '.' ) return 0;
	ip3 = myatoint( ipstr+pos, &ret); pos+= ret; if ( *(ipstr+pos) != '.' ) return 0;
	ip4 = myatoint( ipstr+pos, &ret);
	
	ipnum = ((ip1&0xff)<<24) | ((ip2&0xff)<<16) | ((ip3&0xff)<<8) | (ip4&0xff);

	return ipnum;
}


/* returns the IP as a string so 0x01020304 = 1.2.3.4 */
unsigned long IPStrToIPnum( const char *ipstr )
{
	unsigned long	ipnum = 0, i = 0;
	
#if QS_MAC
	OSStatus	status = noErr;
	
	status = OTInetStringToHost ((char *)ipstr, &ipnum);
	//if (status != noErr)
	//	OutDebugs ("OTInetStringToHost error: %d, ipstr: %s, ipnum: %d", status, ipstr, ipnum);
	
#else

	if ( isdigit(*ipstr) )
	{
		unsigned char *p = (unsigned char *)ipstr, c, ip = 0;
		while ( *p )
		{
			if ( *p == '.' )
			{
				i++;
				ipnum |= ip;
				ipnum = ipnum << 8;
				ip = 0;
				p++;

			}
			else if( myisdigit(*p) )
			{
				c = *p - '0';
				ip = ip*10 + c;
				p++;
			}
			else
			{
				ipnum=0;
				break;
			}
		}
		if ( i == 3 )
		{
			ipnum |= ip;
		} else	// hey, this aint a real IP, or a bad one, return NULL
			ipnum = 0;
	}
#endif
	return ipnum;
}


long iplongtoipstring( long ip, char *str )
{
	if ( str ) 
		sprintf( str, "%u.%u.%u.%u", (ip)&0xff, (ip>>8)&0xff, (ip>>16)&0xff, (ip>>24)&0xff );
	return ip;
}


/*  extra usefull functions
// returns the IP as a string so 0x01020304 = 1.2.3.4
*/
char *IPnumToIPStr( long ip, char *ipstr )
{
	long num1,num2,num3,num4,pos=0;
	
	num1 = (ip&0xff000000) >> 24;
	num2 = (ip&0xff0000) >> 16;
	num3 = (ip&0xff00) >> 8;
	num4 = (ip&0xff);
	
	if ( ipstr ){
		pos += Int2Str( num1, ipstr+pos) ;  ipstr[pos++] = '.';
		pos += Int2Str( num2, ipstr+pos) ;  ipstr[pos++] = '.';
		pos += Int2Str( num3, ipstr+pos) ;  ipstr[pos++] = '.';
		pos += Int2Str( num4, ipstr+pos) ;  ipstr[pos++] = 0;

		return ipstr;
	} else return NULL;
}


double timems( void )
{
	long	time1[2];
	double  usec1;
	
#if	QS_WINDOWS

	time1[0] = GetTickCount()/1000;	time1[1] = (GetTickCount()%1000) * 1000;
#elif	QS_UNIX
	gettimeofday( (struct timeval *)&time1, NULL );
#else
	time1[0] = time(0); time1[1] = 0;
#endif
 	usec1 = ((double)time1[0]) + ((double)time1[1]/1000000.0);
	return usec1;
}


#if	QS_MAC
extern void NotifyUser(const char *message, short messageCode);
#endif

void UserMsg( const char *txt )
{
#if	QS_MAC
	NotifyUser (txt, 0);
#else	
	ErrorMsg( txt );
#endif
}


void ErrorCode( const char *text, int err )
{
	char	txt[256];
	
	sprintf( txt, "%s\nErr = %d (%04x)\n", text, err, err);
	UserMsg( txt );
}


long MakeDirPath( const char *dir )
{
	long	err = 0;
	char	dirName[512];

	mystrncpy (dirName, dir, 512);
	
#if QS_WINDOWS
	{
		struct	stat	pathStat; long staterr;
		staterr = stat( dirName, &pathStat );
		if ( staterr )
			err = CreateDirectory( dirName, NULL );
		else
			return 0;
	}
	err = !err;
#endif

#if __MACOSX__
	{
		long len = mystrlen( dirName );
		if( dirName[len - 1] == '/' )
			dirName[len - 1] = 0;
	}
#endif

#if QS_UNIX		
	err = mkdir( dirName , 0x3c0 );
#endif

#if	QS_MAC
	FSSpec 	mySpec;
	OSErr	anErr = noErr;
	short	num;
	long	dirID;
	
	TrimPathForClassicMacOS (dirName);					// truncate path names BEFORE we create the FSSpec
														// QCM #49484
	if (*dirName)
	{
		num = mystrlen (dirName);
		if (dirName[num - 1] == ':')
		{
			dirName[num - 1] = 0;
			num--;
		}
		anErr = FSpLocationFromFullPath (num, dirName, &mySpec);

/*		if (mySpec.name[0] > 31)						// this block replaced by above call
		{	// trim if too long							// QCM #49484
			mySpec.name[31] = '~';
			mySpec.name[0] = 31;
		}
*/
		anErr = FSpDirCreate (&mySpec, smSystemScript, &dirID);
	}
	err = anErr;
#endif		

	return err;
}



//
// This is a multi level make dir that will make a dir of "blah/blah/blah/lbha"
// for all levels, not just one
//
int MakeDir( const char *dir )
{
	long err, count=0, len, lp;
	char dirName[512], c;

	len = mystrlen( dir );
	for( lp=0;lp<=len;lp++){
		c = dir[lp];
		dirName[lp] = dir[lp];
		dirName[lp+1] = 0;
#if	QS_MAC
		if ( c==':' || lp==len ) {
#else
		if ( c!=0 && (c=='\\' || c=='/' || (lp+1)==len) ) {
#endif
			count++;
			//if NOT EXISTS
			//UserMsg( dirName );
			err = MakeDirPath( dirName );
		}
	}
	return err;
}


