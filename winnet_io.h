#ifndef _WNETIO_H
#define _WNETIO_H

#ifdef __cplusplus
extern "C" {
#endif


extern int				gFtpPassiveFlag;

enum NETTYPE { NETIS_NONE, NETIS_GPRS, NETIS_WIFI, NETIS_LAN=4 };

int FindNetworkAdaptors( CString findthis, CString *outip, CString *gateway, CString *outNetType, CString *outMAC );
int CheckNetAdaptors( CString *outip, CString *outNetType );

__int64  GetFileLength( char *fileName );

void *InitInternet( long );
void *INetOpen( char *url, __int64 *len );
void *INetClose( void *h );

const char *NetworkErr( long *lpErrorCode );
void NetClose( void *h );
long NetWrite( void *fs, char *buffer, long len );
long NetRead( void *ih, char *buffer, long len );


long FtpFileGetSize( char *ftpsite );
void *FtpOpen( void *fs, char *file, char type );

long FtpServerOpenError( const char *server );
void *FtpServerOpen( char *server, char *username, char *password );

void FtpServerClose( void *ih );
int FtpMakeDir( void *server, char *path );
int FtpDelFile( void *server, char *file );
long FtpFileGetSize( char *ftpsite );
long FtpFileUpload( void *fs, char *localfilename, char *remotepath, char *file );


#ifdef __cplusplus
}
#endif

#endif
