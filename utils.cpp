#include "stdafx.h"
#include "utils.h"
#include "history.h"







// return the domain and port details seperately from HOST:PORT
CString URLGetPort( CString domain, long *port )
{
	int pos;
	if( (pos=domain.Find( ":" )) >0 )
	{
		if( port )
		{
			*port = atoi( domain.GetBuffer()+pos+1 );
			if( *port == 0 )
				*port = 80;
		}
		domain.Truncate( pos );
	} else
	{
		if( port )
			*port = 80;
	}

	return domain;
}




unsigned long crctab[256] = {
  0x0,
  0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
  0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6,
  0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
  0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
  0x5BD4B01B, 0x569796C2, 0x52568B75, 0x6A1936C8, 0x6ED82B7F,
  0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A,
  0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
  0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58,
  0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033,
  0xA4AD16EA, 0xA06C0B5D, 0xD4326D90, 0xD0F37027, 0xDDB056FE,
  0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4,
  0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
  0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5,
  0x2AC12072, 0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
  0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 0x7897AB07,
  0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C,
  0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1,
  0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
  0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B,
  0xBB60ADFC, 0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698,
  0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D,
  0x94EA7B2A, 0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
  0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 0xC6BCF05F,
  0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
  0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80,
  0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
  0x58C1663D, 0x558240E4, 0x51435D53, 0x251D3B9E, 0x21DC2629,
  0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C,
  0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
  0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E,
  0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65,
  0xEBA91BBC, 0xEF68060B, 0xD727BBB6, 0xD3E6A601, 0xDEA580D8,
  0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2,
  0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
  0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74,
  0x857130C3, 0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
  0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 0x7B827D21,
  0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A,
  0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E, 0x18197087,
  0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
  0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D,
  0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE,
  0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB,
  0xDBEE767C, 0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
  0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 0x89B8FD09,
  0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
  0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF,
  0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

/* convert a string to a magic number */
unsigned long HashIt(char *string, unsigned short len, unsigned long startingHash )
{          
	register unsigned long answer=startingHash,count;
	register unsigned char b0,c0;
	register char *p;

	p=string;
	for (count=0;count<len;count++) 
	{
		c0 = *p++;
		b0 = (unsigned char)(answer >> 24);
		answer = answer << 8;
		answer |= b0;
		answer ^= (crctab[(b0^c0)&0xff]);
	}

	return(answer);
}



unsigned long CHashIt( CString string )
{
	return HashIt( string.GetBuffer(0), string.GetLength(), 0 );

}

// return crc32 of a file.
unsigned long CHashFile( CString filename )
{
	CStdioFile localFile;

	CString line;
	unsigned long hash = 0;

	if( localFile.Open(filename, CFile::modeRead) )
	{
		while( localFile.ReadString(line)==1 )
		{
			hash = HashIt( line.GetBuffer(0), line.GetLength(), hash );
		}
		localFile.Close();
	}
	return hash;
}





// load a text file into a CString
CString LoadFileintoCString( CString filename )
{
	CStdioFile localFile;
	CString line, fileData;


	if( localFile.Open(filename, CFile::modeRead) )
	{
		while( localFile.ReadString(line)==1 )
		{
			fileData.Append( line );
		}
		localFile.Close();
	}
	return fileData;
}






// ###################################  UPLOAD ERRORS LOG, ROTATE LOGS  #####################################
#include "zlib.h"
// This now supports gzip or bzip2 to compress with.
long GZipCompressFiles( char *sourcefile  )
{
	long	dataread = 0, dataout;
	long    datatotal= 0;
	char	newlogname[512];
	void *outfp;
	char *ram;
	long blocksize = 1024*32;

	if ( ram = (char*)malloc( blocksize ) )
	{
		CFile input;

		if( input.Open( sourcefile, CFile::modeRead ) )
		{
			sprintf( newlogname, "%s.gz", sourcefile );
			outfp = gzopen( newlogname, "wb6" );

			dataout = 0;
			{
				dataread = 1;
				while( dataread>0 )
				{
					dataread = input.Read( ram, blocksize );
					if ( dataread>0 )
					{
						datatotal += dataread;
						gzwrite( outfp, ram , dataread );
					}
				}
				OutDebugs( "gzclose" );
				gzclose( outfp );
				OutDebugs( "gzclose done" );
			}
			input.Close();
		}
		free( ram );
	}
	return datatotal;
}




#include "zipmake.h"
#include ".\widgiexml.h"
// Compress a file on disc into ZIP format.
int zipUpFileX( CString sourcefile, CString zfilename, CString filename )
{
	CFile input;
	int totalread = 0;

	if( input.Open( sourcefile.GetBuffer(0), CFile::modeRead ) )
	{
		zipFile zipData;

		//OutDebugs( "zip open" );
		zipData = zipOpen( (const char *)zfilename.GetBuffer(0), 0 );
		if( zipData )
		{
			static char data[1032];
			static zip_fileinfo zinfo;
			int readsize = 0;

			//OutDebugs( "memset" );
			memset( &zinfo, 0, sizeof(zip_fileinfo) );

			//OutDebugs( "zipOpenNewFileInZip" );
			zipOpenNewFileInZip( zipData, filename.GetBuffer(0), &zinfo, 0, 0, 0, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION );

			//OutDebugs( "input.Read" );
			while( readsize = input.Read( data, 1000 ) )
			{
				totalread += readsize;
				//OutDebugs( "zipWriteInFileInZip %d bytes", readsize );
				zipWriteInFileInZip( zipData, data , readsize );
			}

			//OutDebugs( "zipCloseFileInZip %d bytes", totalread );
			zipCloseFileInZip( zipData );

			//OutDebugs( "zipClose" );
			zipClose( zipData, NULL );
		}
	}
	return totalread;
}





#include <afxtempl.h>

int GetDirectoryList( char *szTempFile, CList <CString,CString> &fileList )
{
	WIN32_FIND_DATA fd; 
	HANDLE hFind; 
	int nNext = 0;

	if ( szTempFile )
	{
		int 	fFound=FALSE;

		hFind = FindFirstFile(szTempFile,&fd); 
		if ( hFind != INVALID_HANDLE_VALUE ) fFound = TRUE;

		while ( fFound )  
		{
			if ( !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			{
				fileList.AddTail( fd.cFileName );
			}
			fFound = FindNextFile( (HANDLE)hFind, &fd );
		}
		FindClose(hFind);
		return nNext;
	}
	return nNext;  
} //




/**
 *  
 *  
 *
 *  
 */

#include <sys/types.h>
#include <sys/stat.h>

BOOL fileExists( char *localFilename )
{
	struct stat sbuf;
	return !stat( localFilename, &sbuf );
}


BOOL FileExists( CString localFilename )
{
	return fileExists( localFilename.GetBuffer(0) );
}


long fileSize( char *localFilename )
{
	struct stat sbuf;

	if( stat( localFilename, &sbuf ) == 0 )
	{
		return sbuf.st_size;
	}

	return 0;
}


void TouchFile( CString filename )
{
	if( FileExists( filename ) == FALSE )
	{
		CFile tmpfile;
		if( tmpfile.Open( filename, CFile::modeCreate | CFile::modeWrite ) )
			tmpfile.Close();
	}
}


// 
char *GetLastErrorString( void )
{
	static char lpMsgBuf[256];

	FormatMessage( 
		//FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		256,
		NULL 
	);
	return lpMsgBuf;
}





CString CryptCString( CString txt, int magickey )
{
	if( magickey )
	{
		CString etxt = txt;
		long e;
		srand( magickey );
		for(int i=0; i< etxt.GetLength(); i++ )
		{
			char c = etxt.GetAt(i);
			e = rand() & 0x7f;
			if( e < 0x21 ) e = 0x21;
			if( e > 125 ) e = 125;
			c = c ^ e;
			etxt.SetAt( i, c );
		}
		return etxt;
	} else
		return txt;
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


int strcmpd(const char * str1, const char * str2)
{
	const	unsigned char * p1 = (unsigned char *) str1;
	const	unsigned char * p2 = (unsigned char *) str2;
	unsigned char		c1, c2;
	
	if ( p2 && p1 ){
		c1 = (*p1++);
		c2 = (*p2++);
		c1 = toupper(c1);
		c2 = toupper(c2);
		
		while ((c1) == (c2)){
			c1 = (*p1++);
			c2 = (*p2++);
			c1 = toupper(c1);
			c2 = toupper(c2);
			if (!c1)
				return(0);
		}
		return(c1 - c2);
	} else
		return 1;
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
	long	oldlen, newlen, len, ret=0, rep=0;

	oldlen = strlen( key );
	newlen = strlen( newkey );

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
			strncpy( p, string, (short)len );
			p += len;
			strncpy( p, newkey, newlen );
			p += newlen;
			string = oldkey + oldlen;
			ret++; rep++;
		} else {
			strncpy( p, string, (short)len );
			p += len;
			strncpy( p, oldkey, (short)oldlen );
			p += oldlen;
			string = oldkey + oldlen;
			rep++;
		}
	}
	if ( ret )
	{
		p += sprintf( p, string );
	}
	*p = 0;

	if ( !newstring )
	{
		if ( ret )
			strcpy( srcstring, ram );
		free( ram );
	}
	return ret;
}




// ftp://user:pass@site.com/path/
void ExtractUserFromURL( char *url, char *server, char *username, char *passwd, char *file )
{
	char	*p, *d, *adr, *host;
	long	l;

	if ( url )
	{
		if ( p = strstr( url, "ftp://" ) )
			adr = url+6;
		else
		if ( p = strstr( url, "http://" ) )
			adr = url+7;
		else
			adr = url;

		if( server )
			server[0] = 0;

		if ( host = mystrrchr( p, '@' ) )
		{
			if( server ){
				strcpyx( server, host+1, '/', 1 );
				l = strlen( server );
			}
			//server[ l-1 ] = 0;
			if ( p = strchr( adr, ':' ) )
			{
				p++;
				if( passwd ){
					d = passwd;
					while( p<host )
						*d++ = *p++;
					*d = 0;
				}
				//strcpyx( passwd, p+1, '@',1 );
				//l = strlen( passwd );
				//passwd[ l-1 ] = 0;
				if( username ){
					strcpyx( username, adr, ':',1 );
					l = strlen( username );
					username[ l-1 ] = 0;
				}
			}
			else
			{
				if( username ){
					strcpyx( username, adr, '@',1 );
					l = strlen( server );
					username[ l-1 ] = 0;
				}
			}
		}
		else
		{
			if( username ){
				username[0] = 0;
				passwd[0] = 0;
				strcpyx( server, adr, '/',1 );
			}
		}

		if( server ){
			l = strlen( server );
			server[ l-1 ] = 0;
			file[0] = 0;
			if ( (p = strchr( adr, '/' )) && file )
				strcpy( file, p );
		}
	}
}


long HexStr2UBYTE( const char *str )
{
	long	num, addr=0;
	short	digit;
	char	hex[]="0123456789ABCDEF";
	
	if ( str ){
		for ( num=0; num < 2; num++){
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




long OutDebug( const char *txt )
{
	if ( txt )
	{
		OutputDebugString( txt );
	}
	return 1;
}


extern void PrintDebugText( char *txt, int color=-1 );
extern BOOL g_logdebug;

static char lineout[5000];
static CString debugMsg;

CString GetLastDebugMsg( void )
{
	return debugMsg;
}

long OutDebugs( const char *txt, ... )
{
	if ( txt )
	{

		if ( txt )
		{
			va_list		args;
			va_start( args, txt);
			vsprintf( lineout, txt, args );
			va_end( args );

			debugMsg = lineout;

			if( g_logdebug )
			{
				Log_App_Debug( debugMsg );
			}
			else
			if( strstr( lineout, "DEBUG:" ) )
			{
				Log_App_Debug( debugMsg );
				PrintDebugText( lineout, 0x00FFFF );
			}

			if( strstr( lineout, "ERROR:" ) )
			{
				Log_App_Error( lineout );
				PrintDebugText( lineout, 0x0000FF );			//BGR
			}
			else
			if( strstr( lineout, "STATUS:" ) )
			{
				Log_App_Event( 0, debugMsg );
				PrintDebugText( lineout );
			}

			strcat( lineout, "\n" );
			OutDebug( lineout );
		}
	}
	return 1;
}





//
// convert a filename such as "125_print_English_003.tif" to "125_print_English_002.tif"
//
CString DecreaseFilenameVersion( CString filename )
{
	CString newfilename = filename;

	CString resToken;
	int curPos=0, segs = 0;

	resToken= newfilename.Tokenize("_",curPos);
	while (resToken != "")
	{
		segs++;
		resToken= newfilename.Tokenize("_",curPos);

		if( segs == 4 )
		{
			int num = atoi( resToken.GetBuffer(0) );
			CString newver, oldver;
			oldver.Format( "%03d", num );
			num--;
			newver.Format( "%03d", num );
			newfilename.Replace( oldver, newver );
		}
	}
	return newfilename;
}




// ######################### TIME FUNCTIONS

CTime TodaysNewTime( char *todaysTimeStr )
{
	CString todaysTime = todaysTimeStr;
	CString todaysDate;
	CTime	today = CTime::GetCurrentTime();

	todaysDate = today.Format( "%Y-%m-%d" );
	todaysTime += ":00";

	today = TimeDateToCTime( todaysTime, todaysDate );
	
	return today;
}


CTime TimeDateToCTime(CString timeString, CString dateString)
{

	int firstSlashPos = dateString.Find(DATE_SEPARATOR, 0);
	int secondSlashPos = dateString.Find(DATE_SEPARATOR, firstSlashPos + 1);

	if( firstSlashPos < 0 )
		firstSlashPos = dateString.Find( '/', 0);

	if( secondSlashPos < 0 )
		secondSlashPos = dateString.Find( '/', firstSlashPos + 1);


	dateString.TrimLeft();
	dateString.TrimRight();


	CString strYear = dateString.Left(firstSlashPos);


	CString strMonth = dateString.Mid(firstSlashPos + 1, secondSlashPos - (firstSlashPos + 1));


	CString strDay = dateString.Right(dateString.GetLength() - secondSlashPos - 1);


    	
	int year = atoi(strYear.GetBuffer(strYear.GetLength()));
	strYear.ReleaseBuffer();

	int month = atoi(strMonth.GetBuffer(strMonth.GetLength()));
	strMonth.ReleaseBuffer();
	
	int day = atoi(strDay.GetBuffer(strDay.GetLength()));
	strDay.ReleaseBuffer();



    int firstColonPos = timeString.Find(TIME_SEPARATOR, 0);
	int secondColonPos = timeString.Find(TIME_SEPARATOR, firstColonPos + 1);

	timeString.TrimLeft();
	timeString.TrimRight();


	CString strHour = timeString.Left(firstColonPos);


	CString strMinute = timeString.Mid(firstColonPos + 1, secondColonPos - (firstColonPos + 1));


	CString strSecond = timeString.Right(timeString.GetLength() - secondColonPos - 1);


	
	int hour = atoi(strHour.GetBuffer(strHour.GetLength()));
	strHour.ReleaseBuffer();

	int minute = atoi(strMinute.GetBuffer(strMinute.GetLength()));
	strMinute.ReleaseBuffer();
	
	int second = atoi(strSecond.GetBuffer(strSecond.GetLength()));
	strSecond.ReleaseBuffer();


    CTime* pCTime = new CTime(year, month, day, 
                              hour, minute, second);

    return(*pCTime);
}




/**
 *  
 *  
 *
 *  
 */


CTimeSpan TimeDateToCTimeSpan(CString timeString, int days)
{
    int firstColonPos = timeString.Find(TIME_SEPARATOR, 0);
	int secondColonPos = timeString.Find(TIME_SEPARATOR, firstColonPos + 1);

	timeString.TrimLeft();
	timeString.TrimRight();


	CString strHour = timeString.Left(firstColonPos);
	CString strMinute,strSecond;

	if( secondColonPos >0 ){
		strMinute = timeString.Mid(firstColonPos + 1, secondColonPos - (firstColonPos + 1));
		strSecond = timeString.Right(timeString.GetLength() - secondColonPos - 1);
	} else {
		strMinute = timeString.Mid(firstColonPos + 1, firstColonPos + 3 );
		strSecond = "00";
	}


	
	int hour = atoi(strHour.GetBuffer(strHour.GetLength()));
	strHour.ReleaseBuffer();

	int minute = atoi(strMinute.GetBuffer(strMinute.GetLength()));
	strMinute.ReleaseBuffer();
	
	int second = atoi(strSecond.GetBuffer(strSecond.GetLength()));
	strSecond.ReleaseBuffer();

 
    CTimeSpan* pCTimeSpan = new CTimeSpan(days, hour, minute, second);

    return(*pCTimeSpan);
}


/**
 *  Retrives individual date elemnets from a Date String
 *  retrived from a <archive_date> tag in the newsflash.xlm file.
 *
 *  The elements it retrives are: 	Year, Month and Day
 *
 *  @param dateString   the string contained in an <archive_date>
 *                      tag
 *
 */


void ExtractDate(CString* dateString, int* pYear, int* pMonth, int* pDay)
{
	int firstSlashPos = dateString->Find(DATE_SEPARATOR, 0);
	int secondSlashPos = dateString->Find(DATE_SEPARATOR, firstSlashPos + 1);

	if( firstSlashPos < 0 )
		firstSlashPos = dateString->Find( '/', 0);

	if( secondSlashPos < 0 )
		secondSlashPos = dateString->Find( '/', firstSlashPos + 1);

	dateString->TrimLeft();
	dateString->TrimRight();


	CString strYear = dateString->Left(firstSlashPos);

	CString strMonth = dateString->Mid(firstSlashPos + 1, secondSlashPos - (firstSlashPos + 1));

	CString strDay = dateString->Right(dateString->GetLength() - secondSlashPos - 1);

	
	*pYear = atoi(strYear.GetBuffer(strYear.GetLength()));
	strYear.ReleaseBuffer();

	*pMonth = atoi(strMonth.GetBuffer(strMonth.GetLength()));
	strMonth.ReleaseBuffer();
	
	*pDay = atoi(strDay.GetBuffer(strDay.GetLength()));
	strDay.ReleaseBuffer();

}

/**
 *  Retrives individual time elemnets from a Time String
 *  retrived from a <archive_time> tag in the newsflash.xlm file.
 *
 *  The elements it retrives are: 	Hour, Min and Sec
 *
 *  @param timeString   the string contained in an <archive_time>
 *                      tag
 *
 */

void ExtractTime(CString* timeString, int* pHour, int* pMinute, int* pSecond)
{
	int firstColonPos = timeString->Find(TIME_SEPARATOR, 0);
	int secondColonPos = timeString->Find(TIME_SEPARATOR, firstColonPos + 1);

	timeString->TrimLeft();
	timeString->TrimRight();


	CString strHour = timeString->Left(firstColonPos);

	CString strMinute = timeString->Mid(firstColonPos + 1, secondColonPos - (firstColonPos + 1));

	CString strSecond = timeString->Right(timeString->GetLength() - secondColonPos - 1);

    
	*pHour = atoi(strHour.GetBuffer(strHour.GetLength()));
	strHour.ReleaseBuffer();

	*pMinute = atoi(strMinute.GetBuffer(strMinute.GetLength()));
	strMinute.ReleaseBuffer();
	
	*pSecond = atoi(strSecond.GetBuffer(strSecond.GetLength()));
	strSecond.ReleaseBuffer();
}
