// #
// # ACTek comms code for uploading data to the ftp server....
// #
// #
// #
// #
// #
// #
// #
// #
// #
// #
// #
// #
// #
// #
// #
// #################################

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>


void FtpClose( void *fs );

	
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

	if ( fs && !IsStopped() )
	{
		void	*hFtpFile;
		FILE	*ff;
		long	dataleft, dataread, perc;
		char	*buffer=NULL;
		char	msg[500];

		if ( !(ff = fopen( localfilename, "rb" )) )
		{
			StatusSetID( IDS_ERR_FILEFAILED, localfilename );
			return FTPERR_NOFILE;
		}

		if ( fs && ff )
		{
			char dir[256], *p;
			static char lastdir[256];
			int tries = 0;

			// Convert all Windows slashes to Unix ones.
			p = remotepath;
			while( *p )
			{
				if ( *p == '\\' ) *p = '/';
				p++;
			}

			// Try to create the remote directories first.
			if ( p )
			{
				// Try to open remote file, if we cant make the remote dir.
				while( (hFtpFile = (void*)FtpOpen( fs, remotepath, 'W' )) == NULL && tries<5 )
				{
					PathFromFullPath( remotepath, dir );
					if ( strcmp( dir, lastdir ) ){
						//long l=mystrcpy( lastdir, dir );
						//if ( dir[l-1] == '/' ) dir[l-1]=0;
						sprintf( msg, "Making dir %s...", dir );
						StatusSet( msg );

						if ( !FtpMakeDir( fs, dir ) )
						{
							failedmkdir = TRUE;
							OutDebugs( "Cannot create remote ftp directory %s", dir );
						} else {
							failedmkdir = FALSE;
						}
					}
					tries++;
					OutDebug( "Trying again to write to server..." );
				}
			}


			// If remote file is opened.
			if ( hFtpFile ) 
			{
				StatusSetID( IDS_UPLOADINGTO, remotepath );
				const long read_size = (4*1024);

				datadone = 0;
				length = dataleft = (long)GetFileLength( localfilename );
				if ( length )
					buffer = (char*)malloc( read_size );

				if ( buffer )
				{
					while( dataleft>0 && buffer && !IsStopped() )
					{
						dataread = fread( buffer, 1, read_size, ff );
						if ( dataread )
							NetWrite( hFtpFile, buffer, dataread );

						dataleft -= dataread;
						datadone += dataread;

						// ------------------------------------------------------
						perc = (long)(100*((length-dataleft)/(float)length));
						sprintf( msg, "Uploading %s  (%d bytes %d%%)", remotepath, datadone, perc );
						StatusSet( msg );
						// ------------------------------------------------------
					}
					if ( datadone == length )
						OutDebug( "Uploaded complete." );

					free( buffer );
				}
				FtpClose( hFtpFile );
				OutDebug( "Ftp File Closed" );
			}
		}
		if ( ff )
			fclose( ff );
	}

	if ( failedmkdir )
		return FTPERR_CANTMAKEDIR;

	if ( IsStopped() )
		return FTPERR_STOPPED;

	if ( datadone == length )
		return FTPERR_COMPLETE;

	return FTPERR_COMPLETEFAIL;
}









