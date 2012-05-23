#include "stdafx.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "utils.h"
#include "winnet_io.h"

#include <sys/stat.h>
#include <windows.h>
#include <wininet.h>

HINTERNET		hNetSession = NULL;
HINTERNET		hFTPSession;
HINTERNET		hFileConnection;           // handle to file enumeration

int				gFtpPassiveFlag = 0;

extern long		OutDebugs( const char *txt, ... );
extern long		OutDebug( const char *txt );


#define	MAXFTPNAMESIZE		(256*2)
#define	MAXDOMAINNAMESIZE	128
#define	MAXERRORMSG			3000
#define	MAX_INETERRSIZE		3000


// -----------------------------------------------------------------------------------------------------
/*

  All the ftp/inet stuff is in

  http://msdn.microsoft.com/workshop/networking/wininet/overview/overview.asp

*/

static int AreWeOnline( void )
{
	if ( InternetAttemptConnect( 0 ) == ERROR_SUCCESS )
		return 1;
	else
		return 0;

}

#include "netadapter.h"


int FindNetworkAdaptors( CString findthis, CString *outip, CString *gateway, CString *outNetType, CString *outMAC )
{
	CNetworkAdapter *m_pAdapters = NULL;
	int m_ACount=0;
	DWORD dwErr;
	ULONG	ulNeeded	= 0;
	int retstatus = 0;

	dwErr = EnumNetworkAdapters( m_pAdapters, 0, &ulNeeded );

	if( dwErr == ERROR_INSUFFICIENT_BUFFER ) 
	{		
		m_ACount	= ulNeeded / sizeof( CNetworkAdapter );
		m_pAdapters = new CNetworkAdapter[ m_ACount ];
		if( m_pAdapters )
			dwErr		= EnumNetworkAdapters( m_pAdapters, ulNeeded, &ulNeeded );
	}

	if( m_pAdapters ) 
	{
//		OutDebugs( "%d ADaptors found", m_ACount );

		for( int lp=0; lp<m_ACount; lp++ )
		{
			CNetworkAdapter *pAdapt = &m_pAdapters[lp];
			CString sIP = pAdapt->GetIpAddr().c_str();
			CString sDesc = pAdapt->GetAdapterDescription().c_str();
			CString sSub = pAdapt->GetSubnetForIpAddr().c_str();
			CString sGate = pAdapt->GetGatewayAddr().c_str();
			CString sDns1 = pAdapt->GetDnsAddr( 0 ).c_str();
			CString sMac = pAdapt->GetAdapterMacAddress().c_str();
			int iType = pAdapt->GetAdapterType();
			std::string sType = pAdapt->GetAdapterTypeString(iType);

			if( iType == MIB_IF_TYPE_ETHERNET && sMac != "000000000000" )
			{
				if( (findthis.Find('*')>=0 ) ||
					(sDesc.Find( findthis ) >= 0)
					)
				{
					if( outip )	*outip = sIP;
					if( outNetType ) *outNetType = sDesc;
					if( outMAC ) *outMAC = sMac;
					if( gateway ) *gateway = sGate;
					retstatus = 1;
					break;
				}
			}
		}
	}

	delete [] m_pAdapters;
	m_pAdapters = NULL;
	return retstatus;
}




int CheckNetAdaptors( CString *outip, CString *outNetType )
{
static CNetworkAdapter *m_pAdapters = NULL;
static int m_ACount=0;
static BOOL busy = FALSE;
	// make sure we arent running twice here because of threads....
	while( busy )
		Sleep(1);

	busy = TRUE;

	DWORD dwErr;
	ULONG	ulNeeded	= 0;
	int retstatus = NETIS_NONE;

	dwErr = EnumNetworkAdapters( m_pAdapters, 0, &ulNeeded );

	if( dwErr == ERROR_INSUFFICIENT_BUFFER ) 
	{		
		m_ACount	= ulNeeded / sizeof( CNetworkAdapter );
		m_pAdapters = new CNetworkAdapter[ m_ACount ];
		if( m_pAdapters )
			dwErr		= EnumNetworkAdapters( m_pAdapters, ulNeeded, &ulNeeded );

		if( m_pAdapters == 0 )
			dwErr		= EnumNetworkAdapters( m_pAdapters, ulNeeded, &ulNeeded );
	}

	if( m_pAdapters ) 
	{
		//OutDebugs( "%d Adaptors found", m_ACount );

		for( int lp=0; lp<m_ACount; lp++ )
		{
			CNetworkAdapter *pAdapt = &m_pAdapters[lp];
			CString sIP = pAdapt->GetIpAddr().c_str();
			CString sDesc = pAdapt->GetAdapterDescription().c_str();
			CString sSub = pAdapt->GetSubnetForIpAddr().c_str();
			CString sGate = pAdapt->GetGatewayAddr().c_str();
			CString sDns1 = pAdapt->GetDnsAddr( 0 ).c_str();
			CString sMac = pAdapt->GetAdapterMacAddress().c_str();

			int iType = pAdapt->GetAdapterType();
			std::string sType = pAdapt->GetAdapterTypeString(iType);

			//OutDebugs( "Checking adaptor %d - type=%d , %s, ip=%s", lp, iType, sDesc.GetBuffer(0), sIP.GetBuffer(0) );

			if( sIP.Find( "0.0.0.0" ) < 0 && sMac != "000000000000" )
			{
				if( sDesc.Find( "Wireless" ) >= 0 )
				{
					if( outip )	*outip = sIP;
					if( outNetType ) *outNetType = "Wifi 11.g";
					retstatus = NETIS_WIFI;
					break;
				} else
				if( iType == MIB_IF_TYPE_PPP && retstatus == NETIS_NONE )
				{
					if( outip )	*outip = sIP;
					if( outNetType ) *outNetType = "GPRS PPP";
					retstatus = NETIS_GPRS;
				} else
				if( iType == MIB_IF_TYPE_ETHERNET )
				{
					if( outip )	*outip = sIP;
					if( outNetType ) *outNetType = "Ethernet LAN";
					retstatus = NETIS_LAN;
					break;
				}
			}
		}
	} else OutDebugs( "m_pAdapters == NULL" );

	if( retstatus == NETIS_NONE && outNetType )
		*outNetType = "No Network";

	if( m_pAdapters )
	{
		delete [] m_pAdapters;
		m_pAdapters = NULL;
	}

	busy = FALSE;
	return retstatus;
}




void *InitInternet( long mode )
{
	if ( AreWeOnline() ){
		if ( hNetSession == 0 ){

			if ( mode )
				mode = INTERNET_OPEN_TYPE_DIRECT;
			else
				mode = INTERNET_OPEN_TYPE_PRECONFIG;

			// Open Internet session.
			hNetSession = InternetOpen("FunnelWeb",
							mode,			//INTERNET_OPEN_TYPE_PRECONFIG,INTERNET_OPEN_TYPE_DIRECT,
							NULL, 
							NULL,
							INTERNET_FLAG_RELOAD ) ;

			OutDebug( "Init Internet" );
		}
	} else
		hNetSession = 0;
	return hNetSession;
}

const char *NetworkErr( long *lpErrorCode )
{
	unsigned long	err=-1,l=MAX_INETERRSIZE;
	static	char szError[MAX_INETERRSIZE];

	if ( !InternetGetLastResponseInfo( &err, szError, &l ) )
		err = 0;

	if ( lpErrorCode )
		*lpErrorCode = err;

	return szError;
}


long NetworkErrMsg( void )
{
	long errval;
	const char *msg;

	msg = NetworkErr( &errval );

	if( errval )
		OutDebugs( msg );

	return errval;
}

HINTERNET hFtpConnect = NULL;


void *INetOpenHTTP( char *url, __int64 *len )
{
	if ( url )
	{
		unsigned long length = 0;

		HINTERNET hNetFile = 0;

		if ( hNetSession == 0 )
		{
			InitInternet(0);
		}

		if ( hNetSession ) 
		{
			OutDebugs( "Opening URL: %s", url );

			if ( strstr( url, "http" ))
			{
				hNetFile = InternetOpenUrl( hNetSession, url, 0, 0, INTERNET_FLAG_HYPERLINK|INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_EXISTING_CONNECT /*flags*/ , 0 );
				OutDebugs( "InternetOpenUrl returned %08x", hNetFile );
				if ( hNetFile ){
					char szText[256];
					int ok;

					length = 32;
					OutDebugs( "Calling HttpQueryInfo()" );
					ok = HttpQueryInfo( hNetFile, HTTP_QUERY_CONTENT_LENGTH, szText, &length, NULL );
					OutDebugs( "HttpQueryInfo() returned %d", ok );
					if ( ok )
						length = atoi(szText) ;
					else
						length = 0;
				}
				if ( !hNetFile )
					NetworkErrMsg();
			}//else its not FTP and not HTTP

			if ( len )
				*len = (__int64)length;

		}
		return (void*)hNetFile;
	}
	return 0;
}



void *INetOpen( char *url, __int64 *len )
{
	if ( url )
	{
		unsigned long length = 0;

		HINTERNET hNetFile = 0;

		if ( hNetSession == 0 )
		{
			InitInternet(0);
		}


		if ( hNetSession ) 
		{
			OutDebugs( "Opening URL..." );

			if ( strstr( url, "http" ))
			{
				hNetFile = InternetOpenUrl( hNetSession, url, 0, 0, INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RESYNCHRONIZE , 0 );		//
				if ( hNetFile ){
					char szText[256];

					length = 32;
					HttpQueryInfo( hNetFile, HTTP_QUERY_CONTENT_LENGTH, szText, &length, NULL );
					length = atoi(szText) ;
				}
			} else
			if ( strstr( url, "ftp" ) )
			{
				char	server[128],
						name[128],
						passwd[128],
						path[MAXFTPNAMESIZE];

				ExtractUserFromURL( url, server, name, passwd, path );
				hFtpConnect = (void*)FtpServerOpen( server, name, passwd );
				if ( !hFtpConnect )
				{
					FtpServerOpenError( server );
					OutDebugs( "FTP Server %s@%s is unreachable", name, server );
				}
				else
				{
					WIN32_FIND_DATA  lpFindFileData;
					HINTERNET hFind;
					char newpath[MAXFTPNAMESIZE];
					long flags = INTERNET_FLAG_NO_CACHE_WRITE;

					strcpy( newpath, path+1 );

					// First check if file exists and get its size.
					hFind = FtpFindFirstFile( hFtpConnect, newpath, &lpFindFileData, flags , 0 );
					if ( !hFind )
					{
						unsigned long size;
						FtpServerClose( hFtpConnect );

						OutDebugs( "No File....Trying root level path instead..." );
						hFtpConnect = (void*)FtpServerOpen( server, name, passwd );
						if ( hFtpConnect )
						{
							FtpGetCurrentDirectory( hFtpConnect, newpath, &size );
							strcat( newpath, path );
							hFind = FtpFindFirstFile( hFtpConnect, newpath, &lpFindFileData, flags , 0 );
						}
					}

					if ( hFind )
					{
						length = lpFindFileData.nFileSizeLow;

						// Now open the FILE after getting its size.
						hNetFile = FtpOpen( hFtpConnect, newpath, 'r' );

						if ( hNetFile )
							OutDebugs( "Ftp File Open %s size = %d", path, length );
						else
							NetworkErrMsg();
					} else
						OutDebugs( "No File found." );

				} // server cannot be opened.
			}//else its not FTP and not HTTP

			if ( len )
				*len = (__int64)length;

		}
		return (void*)hNetFile;
	}
	return 0;
}

void *INetClose( void *ih )
{
	if ( ih ){
		InternetCloseHandle( (HINTERNET)ih );
	}
	return NULL;			
}

void NetClose( void *ih )
{
	INetClose( ih );
	if ( hFtpConnect )
		hFtpConnect = INetClose( hFtpConnect );
	hNetSession = INetClose( hNetSession );
}


long NetWrite( void *fs, char *buffer, long len )
{
	unsigned long	lendone=0;

	if ( !InternetWriteFile( fs, buffer, len, &lendone ) )
		OutDebugs( "netwrite failed" );
	return lendone;
}

long NetRead( void *ih, char *buffer, long len )
{
	unsigned long	lenread=0;

	if ( ih ){
		if ( InternetReadFile( ih, buffer, len, &lenread ) ){
			//lenread = len;
			OutDebugs( "read %d bytes", len );
		} else
			OutDebugs( "NetRead failed" );
	}
	return lenread;
}




long FtpServerOpenError( const char *server )
{
	const char *msg;
	long code;

	msg = NetworkErr( &code );

	if ( !code )
		OutDebugs( "Server %s is unreachable\n\nMake sure you have spelled the domain name correctly.", server );
	else
		OutDebugs( "Server %s is unreachable because ...\n%s", server, msg );
	//MsgBox_Error( IDS_ERR_CONNECTING, server, username, msg );		//Error connecting to ftp server (%s) as user (%s) because ..\n%s.

	OutDebugs( "FtpServerOpen failed return code = %d", code );
	return code;
}

void *FtpServerOpen( char *server, char *username, char *password )
{
	HINTERNET hServer=0;
	long flags;
	short port = INTERNET_DEFAULT_FTP_PORT;
	char *portPtr;

	portPtr = strchr( server, ':' );
	if ( portPtr ){
		port = (short)atoi( portPtr+1 );
		*portPtr = 0;
	}

	OutDebugs( "Doing FtpServerOpen..." );
	if ( hNetSession == 0 ){
		InitInternet(0);
	}

	flags = INTERNET_SERVICE_FTP | INTERNET_FLAG_RESYNCHRONIZE;
	if ( gFtpPassiveFlag )
		flags |= INTERNET_FLAG_PASSIVE;

	hServer = InternetConnect(  hNetSession, server, port , username, password	, INTERNET_SERVICE_FTP , flags , 1 );

	if ( hServer ){
		OutDebugs( "InternetConnect succeeded" );
	} else
		OutDebugs( "DEBUG: InternetConnect failed to %s", server );

	return hServer;
}

void FtpServerClose( void *ih )
{
	OutDebug( "Closing ftp connection." );
	INetClose( ih );
}


long FtpOpenWriteError( char *file )
{
	long err;
	const char *msg;

	msg = NetworkErr( &err );

	if ( err )
	{
		//Check if we really need to report on this error
		// if the last error is <300 then we dont need to really.
		//
		OutDebugs( "Cannot open file %s because ...\n%s", file, msg );
	}
	return err;
}

void *FtpOpenWrite( void *hConnect, char *file, int passive )
{
	HINTERNET hNetFile = 0;

	if ( hNetSession && hConnect ) {
		char cwd[MAXFTPNAMESIZE];
		unsigned long len;
		long flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_RELOAD;
		long access;
		access = GENERIC_WRITE;

		if ( gFtpPassiveFlag )
			flags |= INTERNET_FLAG_PASSIVE;

		len = MAXFTPNAMESIZE-1;

		FtpGetCurrentDirectory( hConnect, cwd, &len );
		strcat( cwd, file );
		hNetFile = FtpOpenFile( hConnect, cwd, access, FTP_TRANSFER_TYPE_BINARY ,0 );

		if( !hNetFile )
		{
			FtpOpenWriteError( file );
		}
	}
	return (void*)hNetFile;
}

void *FtpOpenRead( void *hConnect, char *file, int passive )
{
	HINTERNET hNetFile = 0;

	if ( hNetSession && hConnect ) {
		char cwd[MAXFTPNAMESIZE];
		unsigned long len;
		long flags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_RELOAD;
		long access;

		access = GENERIC_READ;

		if ( passive )
			flags |= INTERNET_FLAG_PASSIVE;

		len = MAXFTPNAMESIZE-1;

		hNetFile = FtpOpenFile( hConnect, cwd, access, FTP_TRANSFER_TYPE_BINARY ,0 );

		if ( !hNetFile  ){
			OutDebugs( "No File....Trying root level path instead..." );
			FtpGetCurrentDirectory( hConnect, cwd, &len );
			strcat( cwd, "/" );
			strcat( cwd, file );
			hNetFile = FtpOpenFile( hConnect, cwd, access, FTP_TRANSFER_TYPE_BINARY ,0 );
		}

		if( !hNetFile )
			NetworkErrMsg();
	}
	return (void*)hNetFile;
}


void *FtpOpen( void *hConnect, char *file, char type )
{
	void *hNetFile;

	type = tolower( type );
	switch( type )
	{
		case 'r' : hNetFile = FtpOpenRead( hConnect, file, gFtpPassiveFlag ); break;
		case 'w' : hNetFile = FtpOpenWrite( hConnect, file, gFtpPassiveFlag ); break;
	}
	return hNetFile;
}

int FtpMakeDir( void *server, char *path )
{
	char cwd[MAXFTPNAMESIZE]; 
	unsigned long len;
	long ret;

	len = MAXFTPNAMESIZE-1;
	FtpGetCurrentDirectory( server, cwd, &len );
	strcat( cwd, path );

	ret = FtpCreateDirectory( server, cwd );

	return ret;
}

int FtpDelFile( void *server, char *file )
{
	long ret;
	ret = FtpDeleteFile( server, file );
	// Try Root Level
	if ( !ret ){
		char cwd[MAXFTPNAMESIZE];
		unsigned long len;
		len = MAXFTPNAMESIZE-1;
		FtpGetCurrentDirectory( server, cwd, &len );
		strcat( cwd, file );
		ret = FtpDeleteFile( server, cwd );

		// last resort, try it with out /
		if ( !ret && file[0] == '/' )
			ret = FtpDeleteFile( server, file+1 );
	}

	if( !ret )
		NetworkErrMsg();
	return ret;
}

long FtpFileGetSize( char *ftpsite )
{
	char	url[MAXFTPNAMESIZE],
			server[128],
			name[128],
			passwd[128],
			path[MAXFTPNAMESIZE];

	WIN32_FIND_DATA  lpFindFileData;
	HINTERNET hFind, hServer;

	if ( ftpsite )
	{
		OutDebug( "FtpFileGetSize..." );
//		if ( IsURLShortCut( ftpsite ) )
//		{
//			GetURLShortCut( ftpsite, url );
//		} else
			strcpy( url, ftpsite );

		ExtractUserFromURL( url, server, name, passwd, path );

		hServer = FtpServerOpen( server, name, passwd );

		if ( hServer )
		{
			long flags = INTERNET_FLAG_NO_CACHE_WRITE;

			hFind = FtpFindFirstFile( hServer, path+1, &lpFindFileData, flags , 0 );
			if ( !hFind )
			{	// failed, so try ROOT path
				char cwd[MAXFTPNAMESIZE]; 
				unsigned long len = MAXFTPNAMESIZE-1;

				FtpServerClose( hServer );

				OutDebugs( "No File....Trying root level path instead..." );
				hServer = FtpServerOpen( server, name, passwd );
				if ( hServer )
				{
					FtpGetCurrentDirectory( hServer, cwd, &len );
					strcat( cwd, path );
					hFind = FtpFindFirstFile( hServer, cwd, &lpFindFileData, flags , 0 );
				}
			}

			FtpServerClose( hServer );
			if ( hFind ) 
				return lpFindFileData.nFileSizeLow;
		}
	}
	return 0;
}


/*

  BOOL InternetGetLastResponseInfo(
    OUT LPDWORD lpdwError,
    OUT LPTSTR lpszBuffer,
    IN OUT LPDWORD lpdwBufferLength
);


*/


void FtpClose( void *fs )
{
	INetClose( fs );
}




__int64  GetFileLength( char *fileName )
{
	FILE 	*fp = 0;
	__int64 	len=0;

	if( fileName && *fileName )
	{
		if ( (fp = fopen( fileName, "rb" )) ) 
		{
			fseek( fp, 0, SEEK_END );
			len = ftell( fp );
			fclose( fp );
		}
	}
	return len;
}



	
// returns BYTES transfered or erroers in negative format
// 0   stopped
#define	FTPERR_COMPLETE		1
#define	FTPERR_STOPPED		0
#define	FTPERR_NOFILE		-1
#define	FTPERR_CANTMAKEDIR	-2
#define	FTPERR_COMPLETEFAIL	-3

// -2  cant make dir
long FtpFileUpload( void *fs, char *localfilename, char *remotepath, char *file )
{
	long length=0, failedmkdir=FALSE, datadone;

	if ( fs )
	{
		void	*hFtpFile;
		FILE	*ff;
		long	dataleft, dataread, perc;
		char	*buffer=NULL;
		char	msg[500];

		if ( !(ff = fopen( localfilename, "rb" )) )
		{
			OutDebugs( "ERROR: FtpFileUpload()/Cannot open file %s", localfilename );
			return FTPERR_NOFILE;
		} else
		{
			char dir[256], *p;
			static char lastdir[256];
			int tries = 0;

			// Convert all Windows slashes to Unix ones.
			p = remotepath;
			while( p && *p )
			{
				if ( *p == '\\' ) *p = '/';
				p++;
			}

			// Try to create the remote directories first.
			if ( p && remotepath )
			{
				char remotefilename[256];
				sprintf( remotefilename, "%s%s", remotepath, file );
				// Try to open remote file, if we cant make the remote dir.
				while( (hFtpFile = (void*)FtpOpen( fs, remotefilename, 'W' )) == NULL && tries<5 )
				{
					//PathFromFullPath( remotepath, dir );
					strcpy( dir, remotepath );
					char *slashPtr = strchr( dir, '/' );
					if( slashPtr ) *slashPtr = 0;

					if ( dir && *dir && strcmp( dir, lastdir ) ){
						//long l=mystrcpy( lastdir, dir );
						//if ( dir[l-1] == '/' ) dir[l-1]=0;
						sprintf( msg, "Making dir %s...", dir );
						if ( !FtpMakeDir( fs, dir ) )
						{
							failedmkdir = TRUE;
							OutDebugs( "DEBUG: Cannot create remote ftp directory %s", dir );
						} else {
							failedmkdir = FALSE;
						}
					}
					tries++;
					OutDebugs( "Trying to FtpOpen again..." );
				}
			}


			// If remote file is opened.
			if ( hFtpFile ) 
			{
				const long read_size = (4*1024);

				datadone = 0;
				length = dataleft = (long)GetFileLength( localfilename );
				if ( length>0 )
					buffer = (char*)malloc( read_size+32 );

				if ( buffer )
				{
					while( dataleft>0 )
					{
						dataread = fread( buffer, 1, read_size, ff );
						if ( dataread )
							NetWrite( hFtpFile, buffer, dataread );

						dataleft -= dataread;
						datadone += dataread;

						// ------------------------------------------------------
						perc = (long)(100*((length-dataleft)/(float)length));
						OutDebugs( "Uploading %s  (%d bytes %d%%)", localfilename, datadone, perc );
						// ------------------------------------------------------
					}
					if ( datadone == length )
						OutDebugs( "DEBUG: Uploaded complete." );

					free( buffer );
				}
				FtpClose( hFtpFile );
				OutDebugs( "Ftp File Closed" );
			} else
				OutDebugs( "DEBUG: Ftp File could not be written to server" );

			fclose( ff );
		}
	}

	if ( failedmkdir )
		return FTPERR_CANTMAKEDIR;

	if ( datadone == length )
		return FTPERR_COMPLETE;

	return FTPERR_COMPLETEFAIL;
}














