
///////////////////////////////////////////////////////////
//
//   httpinterface.c
//
//   - Implements a tiny web server to handle such things
//     as integration with Funnel Web Profiler and 
//     Benchmark Factory
//
//   Oct 2001 - implemented OpenTransport server for MacOS
//
//
///////////////////////////////////////////////////////////
#include "stdafx.h"

#include "afxwin.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <winsock.h>

#include "utils.h"
#include "winnet_io.h"
#include "version.h"
#include "Widgie.h"
#include "widgieDlg.h"
#include "history.h"
#include "httpinterface.h"

#define QS_WINDOWS

long SpawnHTTPD( void );



static SOCKET http_socket = 0, peersock = 0;
static struct sockaddr_in A, Peer;
static long	msgcount=0;
static CWidgieApp *s_cfg = NULL;

static char *HTMLheader = "<HTML>\
<HEAD>\
   <TITLE>Http Interface</TITLE>\
   <META HTTP-EQUIV=\"refresh\" CONTENT=\"%d\">\
</HEAD>\
<BODY BGCOLOR=#666669 TEXT=WHITE LINK=#2222FF VLINK=#cccc33>\
<P><BR>\r\n";







// added in rev 8
char *CopyLine( char *d, char *s )
{
	char* s_orig=s;

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

long mystrcpy( char *dst, const char *src )	// c-string copy function
{
	long len=0;
	
	if( src && dst )
		while(( *dst++ = *src++ ) != 0){ len++; };
	return len;
}







#ifdef _WINE
#define WSA_MAJOR_VERSION 1
#define WSA_MINOR_VERSION 0
#else
#define WSA_MAJOR_VERSION 1
#define WSA_MINOR_VERSION 1
#endif
#define WSA_VERSION MAKEWORD(WSA_MAJOR_VERSION, WSA_MINOR_VERSION)


static BOOL winsockInit=FALSE;
static WSADATA wsaData;

long WSInit( void )
{
	int err;
	
	if( !winsockInit ){
		err = WSAStartup ( WSA_VERSION, &wsaData  ); 
		if( err ) {
			/* Tell the user that we couldn't find a usable */
			/* WinSock DLL.*/
			OutDebugs( "Couldn't open up Winsock" );
			winsockInit = FALSE;
		} else
			winsockInit = TRUE;
	} else
		winsockInit = TRUE;
	return winsockInit;
}

void WSClose( void )
{
	if( winsockInit )
		WSACleanup( );
	winsockInit = FALSE;
}



void CloseHttpPeer( void ){
	if( peersock ){
		shutdown(peersock,2);
		closesocket(peersock);
		peersock = 0;
	}
}



static int	threadcount = 1;
static BOOL	s_shuttingDown = 0;

typedef int socklen_t;

DWORD WINAPI HTTPDThread( PVOID lpParam )
{
	long ret, count=0;

	if( lpParam ){
		SOCKET client_socket;
		struct sockaddr_in client;
		socklen_t client_len;
		long	client_ip;
		char	ipStr[32];
		char	*p;

		threadcount++;

		client_len = sizeof(client);
		client_socket = accept(http_socket,(struct sockaddr *) &client,&client_len);
		SpawnHTTPD();

		if( client_socket != INVALID_SOCKET )
		{
			long keepalive = 1;
			char line[10240];
			char recbuf[20000];

	        struct linger lin;
            int buffsize = 64000 ;
	        int	opt      = 1;
	        int	optlen   = sizeof(opt);

	        lin.l_onoff =  1 ; lin.l_linger = 60;
	        setsockopt(client_socket, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(lin));
	        setsockopt(client_socket, SOL_SOCKET, SO_RCVBUF, (char *)&buffsize, sizeof(buffsize));
	        setsockopt(client_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt, optlen);

			client_ip = (long)client.sin_addr.s_addr;

            Sleep(100) ;
			ret = recv( client_socket, recbuf, 20000, 0 );
			// while recv() is open... keep geting data...
			while ( ret>0 || ret==keepalive )
			{

				// check to see if we are quiting.
				if( s_shuttingDown == FALSE && 
					AFXDIALOG && 
					AFXDIALOG->shuttingDown )
				{
					s_shuttingDown = TRUE;
				}

				if( ret>0 )
				{
					char httpurl[10240];

					recbuf[ret] = 0;
					if( recbuf[ret-1] == '\n' ) 
					{
						p = recbuf;
						while( p )
						{
							p = CopyLine( line, p );
							if( strstr( line, "GET " ) || strstr( line, "POST " ) )
								CopyLine( httpurl, line );
							//if( strstr( line, "Keep-Alive" ) )
							//	keepalive = -1;
							sprintf( ipStr, "HTTPD%d", lpParam );
                            line[256] = '\0' ; // truncate because ShowReceivedLine can't cope with big URLS and line is finished with now
							//ShowRecievedLine( ipStr, line );
						}
						if( !s_shuttingDown && HandleHttpURL( client_socket, httpurl ) )
							count=0;
						else
							count++;
					}
				}
				if( ret == keepalive ) 
				{
					ret = recv( client_socket, recbuf, 30000, 0 );
					//Sleep(1);
					//OutDebugs( "sleep" );
				} else ret = 0;
			}
			if( ret<0 ){
				sprintf( line, "ret=%d, errno=%d", ret, errno );
				//OutDebugs( line );
			}
			if( ret == SOCKET_ERROR ){
				sprintf( line, "HTTPD%d: socket recv error", lpParam );
				//OutDebugs( line );
			}

			shutdown(client_socket,2);
			closesocket(client_socket);

		} else 
		{
			OutDebugs( "Failed to listen" );
		}
	}
	threadcount--;

	ExitThread( count );
    return(TRUE) ;
}



long SpawnHTTPD( void )
{
	HANDLE 	ghProcessThread = NULL;
	unsigned long dwThreadId;

	CreateThread( 
		NULL,                        // no security attributes 
		0,                           // use default stack size  
		HTTPDThread,				 // thread function 
		(void*)threadcount,	     // argument to thread function 
		0,                           // use default creation flags 
		&dwThreadId);                // returns the thread identifier  
    
#ifdef QS_WINDOWS
	if (ghProcessThread) {
		SetThreadPriority( ghProcessThread, THREAD_PRIORITY_BELOW_NORMAL );
	}
#endif
	return(1);
}


void CloseHttpServer( void ){
	if( http_socket ){
		shutdown(http_socket,2);
		closesocket(http_socket);
		http_socket = 0;
	}
}





void InitHttpServer( short port )
{
	int success = 0;

	s_cfg = CFG;


	OutDebugs( "WSinit()" );

	success = WSInit();

	if( success ){
		long ret;
		http_socket = socket(AF_INET,SOCK_STREAM,0);
		A.sin_family=AF_INET;
		A.sin_port = htons( port );
		A.sin_addr.s_addr = INADDR_ANY;			//inet_addr("127.0.0.1");
		OutDebugs( "binding port..." );
		ret = bind(http_socket,(struct sockaddr *)&A,sizeof(A));
		if( ret != 0 )
		{
			A.sin_port = htons( port+1 );
			ret = bind(http_socket,(struct sockaddr *)&A,sizeof(A));
		}
		if( !ret ){
			ret=listen( http_socket,25 );
			SpawnHTTPD();
		}
	}
	OutDebugs( "Init Http complete." );
}

long SendData( long os, const char *data, long len )
{
	long ret=0;
	if( os )
	{
		ret = send( os, data, len, 0 );
	}

	return ret;
}

long SendStrHTTP( long os, char *txt )
{
	long ret;
	if( os )
		ret = SendData( os, txt,strlen(txt));

	return ret;
}







#define	BUFFSIZE	(1024*10)
void SendFile( long socket, char *filename )
{
	FILE *fp;

	if( fp = fopen( filename, "rb" ) ){
		char *ram = (char *)malloc( BUFFSIZE );
		long dataread;
		while( !feof( fp ) ){
			dataread = fread( ram, 1, BUFFSIZE, fp );
			if( dataread>0 )
				SendData( socket, ram, dataread );
		}
		fclose(fp);
		free( ram );
	}
}




long MakeHTTPHeader( char *d, char *file, long type )
{
	long filesize = GetFileLength( file );

	if( type >= 0 )
		strcpy( d, "HTTP/1.0 200 OK\r\n" );
	else
		strcpy( d, "HTTP/1.0 404 ERR\r\n" );
	strcat( d, "Server: Widgie (Windows)\r\n" );
	//strcat( d, "Last-Modified: Thu, 25 Mar 1999 06:14:37 GMT\r\n" );
	CString tmp;

	if( type <= 0 ){
		strcat( d, "Accept-Ranges: bytes\r\n" );
		strcat( d, "Connection: close\r\n" );
		strcat( d, "Content-Type: text/html\r\n\r\n\0" );
	} else {
		strcat( d, "Accept-Ranges: bytes\r\n" );
		if (type == 1)
		{
			strcat( d, "Connection: close\r\n" );
			strcat( d, "Content-Type: application/xml\r\n" );
		}	
		else 
		if (type == 2)
		{
			char *ext;
			ext = strrchr( file, '.' );
			strcat( d, "Connection: close\r\n" );
			strcat( d, "Content-Type: image/jpeg\r\n" );	//strcat( d, ext+1 );
		}	
		else 
		if (type == 3)
		{
			strcat( d, "Accept-Ranges: bytes\r\n" );
			strcat( d, "Connection: close\r\n" );
			strcat( d, "Content-Type: video/mpeg\r\n" );
		}
		else
		if (type == 4)
		{
			strcat( d, "Accept-Ranges: bytes\r\n" );
			strcat( d, "Connection: close\r\n" );
			strcat( d, "Content-Type: application/x-shockwave-flash\r\n" );
		}
		else
		{
			strcat( d, "Accept-Ranges: bytes\r\n" );
			strcat( d, "Connection: close\r\n" );
			strcat( d, "Content-Type: text/plain\r\n" );
		}
		tmp.Format( "Content-Length: %d", filesize );
		strcat( d, tmp.GetBuffer(0) );
		strcat( d, "\r\n\r\n\0" );
	}
	return strlen(d);
}



// convert any %20 keys to ' '
void ConvertURLtoPlain( char *string )
{
	register char c, c2, *p = string;
	while( (c=*string++) ){
		if( c == '%' ){
			c = *string++;
			c = toupper(c);
			if( c>= 'A' ) c -= 'A'-10;
			else c -= '0';

			c2 = *string++;
			c2 = toupper(c2);
			if( c2>= 'A' ) c2 -= 'A'-10;
			else c2 -= '0';
			c = c<<4 | c2;
		}
        else
        if( c == '+' ){
           c = ' ' ;
        }
		*p++ = c;
	}

    *p = 0;
}

// load a file from HD into ram and return pointer
char *LoadFile( char *filename, long length )
{
	FILE *fp;
	char *ram;
	long dataread;

	if( !length )
		length = (long)GetFileLength( filename );
	if( fp = fopen( filename, "rb" ) ){
		ram = (char *)malloc( length+16 );
		dataread = fread( ram, 1, length, fp );
		ram[dataread] = 0;
		fclose(fp);
	}
	return ram;
}


typedef struct {
	short type;		//1=string, 2=longnum,3=short,4=radio box, 5=8bit radio, 6=8bit checkbox
	char *name;
	void *data;
} DataMap, *DataMapPtr;

#define NO_TYPE			0
#define CHAR_SET_TYPE	1
#define LONG_SET_TYPE	2
#define SHORT_SET_TYPE	3
#define CHARVAL_SET_TYPE 4
#define SHORTVAL_SET_TYPE 5

DataMap settingsmap[] = {
	//  REPORT SETTINGS
/*	CHAR_SET_TYPE, "[OUT]", MyPrefStruct.outfile ,
	CHAR_SET_TYPE, "[SITEURL]", MyPrefStruct.siteurl,
	CHAR_SET_TYPE, "[DEFAULT]", MyPrefStruct.defaultindex,
	LONG_SET_TYPE, "[DNSNUM]", &MyPrefStruct.dnsAmount,
	SHORTVAL_SET_TYPE, "[reportformat]", &MyPrefStruct.report_format,
	SHORTVAL_SET_TYPE, "[DNR]", &MyPrefStruct.dnslookup,
	SHORTVAL_SET_TYPE, "[sort]", &MyPrefStruct.sortby,
	6, "[ignoreself]", &MyPrefStruct.ignore_selfreferral,
	6, "[ignorebookmark]", &MyPrefStruct.ignore_bookmarkreferral,
	6, "[filterzero]", &MyPrefStruct.filter_zerobyte,
	6, "[ignorecase]", &MyPrefStruct.ignorecase,
	6, "[usecgi]", &MyPrefStruct.useCGI,
*/
	NO_TYPE,"",0
};



// saving config into output format.
// for a given template file with [keywords] in form objects, replace them
// with actual values/numbers whatever to fill in the data.
long FillInVariables( char *htmltext, char *out )
{
	DataMapPtr p = settingsmap;
	long len = 0;
	char *txt, *vp;
	char numStr[32];
	long *longdata;
	short *shortdata, num;
	char	*chardata;

	while( p->type ){
		if( txt=strstr( htmltext,p->name ) ){
			switch( p->type ){
				case CHAR_SET_TYPE:
					len = ReplaceStr( htmltext, out, p->name, (char*) p->data, 0 );
					break;
				case LONG_SET_TYPE:
					longdata = (long *)p->data;
					sprintf( numStr, "%d", *longdata );
					len = ReplaceStr( htmltext, out, p->name, numStr, 0 );
					break;
				case 3 :
					shortdata = (short *)p->data;
					sprintf( numStr, "%d", *shortdata );
					len = ReplaceStr( htmltext, out, p->name, numStr, 0 );
					break;
				case 4 :
					shortdata = (short *) p->data;
					if( vp = strstr( txt, "value=" ) ){
						vp+=6;
						if( *vp == 34 ) vp++;
						num = (short)atoi( vp+6 );
						if( num == *shortdata )
							len = ReplaceStr( htmltext, out, "unchecked", "  checked", 0 );
					}
					break;
				case SHORTVAL_SET_TYPE:
					chardata = (char *)p->data;
					if( vp = strstr( txt, "value=" ) ){
						vp+=6;
						if( *vp == 34 ) vp++;
						num = (short)atoi( vp );
						if( num == *chardata )
							len = ReplaceStr( htmltext, out, "unchecked", "  checked", 0 );
					}
					break;
				case 6 :
					chardata = (char *)p->data;
					if( *chardata )
						len = ReplaceStr( htmltext, out, "unchecked", "  checked", 0 );
					break;

			}

		}
		p++;
	}
	return len;
}




// for a block of data/html page, replace tokens with variable data
// and copy into a new block of ram.
// Send out this data via socket tcp to client.
long SendRemappedHtml( long os, char *htmldata, long datalen )
{
	char *out, *ram, *data = htmldata;
	char *p,c, foundbracket = 0;
	long len = 0, done=0, outlen = 0;

	ram = (char *)malloc( datalen+12048 );
	p = data;
	out = ram;
	while( c=*data ){
		if( c == '[' )
			foundbracket++;

		if( (c == '\n') && foundbracket ){
			*data = 0;
			if( len=FillInVariables( p, out ) ){
				len = strlen(out);
				outlen += len;
				out += len;
				done++;
			} else {
				len = mystrcpy( out, p );
				outlen += len;
				out += len;
				done++;
			}
			foundbracket=0;
			*out++ = '\r';
			outlen++;
			p = data+1;
		}
		data++;
	}
	if( done ){
		len = mystrcpy( out, p );
		outlen += len;
		out += len;
		done++;
		SendData( os, ram, outlen );
	} else
		SendData( os, htmldata, datalen );

	free( ram );

	return done;
}

long ServeFile( long os, char *name, char *outline )
{
	long d = 0, type , len;

	if( len = (long)GetFileLength( name ) ){
		if( strstr( name, ".htm" ) )
			type = 0;
		else
		if( strstr( name, ".xml" ) )
			type = 1;
		else
		if( strstr( name, ".jpg" ) || strstr( name, ".gif" )  )
			type = 2;
		else
		if( strstr( name, ".mpg" ) || strstr( name, ".mpeg" )  )
			type = 3;
		else
		if( strstr( name, ".swf" ) )
			type = 4;
		else
			type = -1;

		d = MakeHTTPHeader( outline, name, type );

		if( strstr( name, ".log" ) )
			strcat( outline, "<PRE>\n" );

		SendStrHTTP( os, outline );
		//SendData( os, outline, d );

		if( type == 0 ){
			char *ram;
			if( ram = LoadFile( name, len ) ){
				if( SendRemappedHtml( os, ram, len ) == 0 ){
					;//SendData( os, ram, len );
				}
				free( ram );
			}
		} else {
			SendFile( os, name );
		}
	}
	return d;
}

void GetURLPath( char *url, char *destination )
{
	if( *url == '/' )
	{
		strcpy( destination, url );
	} else {
		sprintf( destination, "wwwserver\\%s", url );
	}
}



// ------------------------------------- INTERFACE LEVEL CODE HANDLING -----------------------------

#define	SENDHTML(x)		MakeHTTPHeader( outline, NULL, 0 ); strcat( outline, x ); d = SendStrHTTP( os, outline )


#define	HEADLINE(text)	"<TABLE width=540><TR><TD bgcolor=#aaaaaa>" text "</TD></TR></TABLE>"

static char HTMLindex[] =
{
	"<BR><BR><BR>"
	HEADLINE( "====== Status Options ======" )
	"<A HREF=/status>System Status Info</A><BR>"
	"<A HREF=/slideshowlist>Slideshow List</A><BR>"
	"<A HREF=/displaylist>All Slideshows List</A><BR>"
	"<A HREF=/eventlist>Event History List</A><BR>"
	"<A HREF=/debug>Debug Messages Monitor</A><BR>"
	"--- LOGS ---<br>"
	"<A HREF=/logs/debug.log>Debug Log File</A><BR>"
	"<A HREF=/logs/operations.log>Operations Log File</A><BR>"
	"<A HREF=/logs/errors.log>Error Log File</A><BR><br>"
	"<A HREF=/content/widgie.exe>Download Exe</A><BR>"
	"--- XML FILES ---<br>"
	"<A HREF=/content/config/newsflash.xml>View newsflash.xml</A><BR>"
	"<A HREF=/content/config/slideshow.xml>View slideshow.xml</A><BR>"
	"<A HREF=/content/config/config.xml>View config.xml</A><BR>"
	"<BR><BR>"
	HEADLINE( "====== Admin Menu ======" )
	"<A HREF='/action.cgi?command=1'>1.  Clear Image Cache</A><BR>"
	"<A HREF='/action.cgi?command=2'>2.  Reload Play List from disk</A><BR>"
	"<A HREF='/action.cgi?command=3'>3.  Update All Content now</A><BR>"
	"<A HREF='/action.cgi?command=4'>4.  Download XML files</A><BR>"
	"<A HREF='/action.cgi?command=5'>5.  Download News XML content now</A><BR>"
	"<A HREF='/action.cgi?command=6'>6.  Download Image/Media Content now</A><BR>"
	"<A HREF='/action.cgi?command=7'>7.  Delete unused/old Media Content</A><BR>"
	"<A HREF='/action.cgi?command=8'>8.  Debug Status Messages On/Off</A><BR>"
	"<A HREF='/action.cgi?command=9'>9.  Download new Widgie.exe</A><BR>"
	"<A HREF='/action.cgi?command=10'>10. Scroll Speed Change</A><BR>"
	"<A HREF='/action.cgi?command=11'>11. Skip to next video</A><BR>"
	"<A HREF='/action.cgi?command=12'>12. Pause/UnPause</A><BR>"

	"<A HREF='/action.cgi?command=19'>19. Launch VNC Server</A><BR>"
	"<A HREF='/action.cgi?command=20'>20. Quit to windows and stop</A><BR>"
	"<A HREF='/action.cgi?command=21'>21. Quit and restart App</A><BR>"
	"<A HREF='/action.cgi?command=22'>22. Quit and Reboot</A><BR>"
	"<A HREF='/action.cgi?command=23'>23. Quit and Shutdown</A><BR>"
};


// ------------------------------------- CGI/PATH LEVEL CODE HANDLING -----------------------------



// handle incoming url requests, decide weather to serve a file or do internal 
// html page generation

long HandleHttpURL( long os, char *line )
{
	long	out=0,d=0,len;
	char	*outline, tmp[512];
	char	*p;

	if( !strcmpd( "GET", line ) || !strcmpd( "POST", line ) )
	{
		char *paramUrl = line + 4;
		outline = (char *) malloc( 65365 );

		//OutDebugs( "HTTP SERV : %s", line );

		if( p = strchr( paramUrl, ' ' ) )
			*(p) = 0;

		if( !strcmpd( "/action.cgi", paramUrl )  )
		{
			p = strchr( paramUrl, '=' );
			if( p )
			{
				p++;
				AFXDIALOG->ChooseAdminMenu( atoi(p) );
			}
			d = 1;
			strcpy( paramUrl, "/index.html" );
		}

		if( !strcmpd( "/index.html", paramUrl ) || !strcmp( "/", paramUrl ) ){
			MakeHTTPHeader( tmp, NULL, 0 );
			CString page = tmp;
			page += HTMLheader;
			CString text = CString(CFG->GetExeName()) + " v" PRODUCT_VERSION_STR " System Control : CLIENT ID = " + s_cfg->cfgID;
			page += HEADLINE( +text+ );

			page += HTMLindex;			// the menu hardcoded, should really be dyngenerated using main menu static text.
			d = SendStrHTTP( os, page.GetBuffer(0) );
		} else
		if( !strcmpd( "/debug", paramUrl ) )
		{
			MakeHTTPHeader( tmp, NULL, 0 );
			CString page = tmp;
			page += HTMLheader;
			CString text = "Debug Status Monitor : IVA ID = " + s_cfg->cfgID;
			page += HEADLINE( +text+ );
			d = SendStrHTTP( os, page.GetBuffer(0) );

			CString prevMsg;
			while( os && d>0 )
			{
				// results to be printed to printer...
				CString msg = GetLastDebugMsg();
				if( msg.Compare( "Exit " ) == FALSE )
				{
					os = NULL;
				} else
				if( prevMsg.Compare(msg.GetBuffer(0)) )
				{
					prevMsg = msg;

					CTime nowTime = CTime::GetCurrentTime();
					CString now = nowTime.Format( "%Y-%m-%d %H:%M:%S" ) + " ";
					msg.Insert( 0, now );
					msg += "<br>\n";
					d = SendStrHTTP( os, msg.GetBuffer(0) );
				}
				Sleep(1);
			}
			d = 1;
		} else

		if( !strcmpd( "/status", paramUrl ) )
		{
			MakeHTTPHeader( tmp, NULL, 0 );
			CString page = tmp, header;
			header.Format( HTMLheader, 3 );
			page += header;
			CString text = "System Status Info : CLIENT ID = " + s_cfg->cfgID;
			page += HEADLINE( +text+ );
			page += "<PRE>";
			page += AFXDIALOG->GetStatusText();
			page += "</PRE><br><br>";
			page += "<A HREF='currentimage.jpg'><noimg src='currentimage.jpg' width='50%' border=0>View Impression Image</A><br>";
			page += "<A HREF='currentmedia'>View Impression Media Presentation</A><br>";
			page += "<A HREF='/index.html'>back</A><br>";
			d = SendStrHTTP( os, page.GetBuffer(0) );
		} else
		if( !strcmpd( "/ver", paramUrl ) || !strcmpd( "/ver.html", paramUrl ) )
		{
			MakeHTTPHeader( outline, NULL, 0 );
			strcat( outline, HTMLheader );
			strcat( outline, HEADLINE( "Widgie Version" ) );

			strcat( outline, "<H3>Version information</H3><HR><BR>" );
			sprintf( tmp, "%s (socket=%08lx<BR>",PRODUCT_VERSION_STR, os );
			strcat( outline, tmp );
			d = SendStrHTTP( os, outline );
		} else
		if( !strcmpd( "/slideshowlist", paramUrl ) )			// ascii slideshow list
		{
			MakeHTTPHeader( tmp, NULL, 0 );
			CString page = tmp, header;
			header.Format( HTMLheader, 5 );
			page += header;
			CString text = "System Slide Show List : CLIENT ID = " + s_cfg->cfgID;
			page += HEADLINE( +text+ );
			page += "<PRE>";
			page += AFXDIALOG->Status_GetSlideshow();
			page += "</PRE>";
			page += "<A HREF='/index.html'>back</A><br>";
			d = SendStrHTTP( os, page.GetBuffer(0) );
		} else
		if( !strcmpd( "/displaylist", paramUrl ) )			// ascii slideshow list
		{
			MakeHTTPHeader( tmp, NULL, 0 );
			CString page = tmp, header;
			header.Format( HTMLheader, 120 );
			page += header;
			CString text = "System Display all Slideshows List : CLIENT ID = " + s_cfg->cfgID;
			page += HEADLINE( +text+ );
			page += "<PRE>";
			page += AFXDIALOG->Status_GetSlideShowsList();
			page += "</PRE>";
			page += "<A HREF='/index.html'>back</A><br>";
			d = SendStrHTTP( os, page.GetBuffer(0) );
		} else

		if( !strcmpd( "/eventlist", paramUrl ) )			// ascii slideshow list
		{
			MakeHTTPHeader( tmp, NULL, 0 );
			CString page = tmp, header;
			header.Format( HTMLheader, 120 );
			page += header;
			CString text = "System Display Current Event List : CLIENT ID = " + s_cfg->cfgID;
			page += HEADLINE( +text+ );
			page += "<PRE>";
			page += AFXDIALOG->Status_GetEventList();
			page += "</PRE>";
			page += "<A HREF='/index.html'>back</A><br>";
			d = SendStrHTTP( os, page.GetBuffer(0) );
		} else
			
		if( !strcmpd( "/media/", paramUrl ) || 
			!strcmpd( s_cfg->cfgServerImagePath, paramUrl ) )
		{
			CString filename = paramUrl+13;
			filename.Insert( 0, DEFPATH_CONTENT );
			if( ServeFile( os, filename.GetBuffer(0), outline ) )
				d = 1;
		} else
		// return video file asked for using MDID standard paths  "/content/video/file.mpeg"
		if( !strcmpd( s_cfg->cfgServerVideoPath, paramUrl ) )
		{
			char *fileP = strrchr( paramUrl, '/' );
			if( fileP )
			{
				CString filename = fileP+1;
				filename.Insert( 0, DEFPATH_CONTENT );
				if( ServeFile( os, filename.GetBuffer(0), outline ) )
					d = 1;
			}
		} else
		
		// return current slide content
		if( !strcmpd( "/currentimage.jpg", paramUrl )  )
		{
			CString filename = DEFPATH_CONTENT + AFXDIALOG->currentImpression.m_image_file;
			if( ServeFile( os, filename.GetBuffer(0), outline ) )
				d = 1;
		} else
		if( !strcmpd( "/currentmedia", paramUrl )  )
		{
			CString filename = DEFPATH_CONTENT + AFXDIALOG->currentImpression.m_multimedia_file;
			if( ServeFile( os, filename.GetBuffer(0), outline ) )
				d = 1;
		} else
		// return logs/ contents
		if( !strcmpd( "/logs/debug.log", paramUrl )  )
		{
			CString filename = FN_DEBUG;
			if( ServeFile( os, filename.GetBuffer(0), outline ) )
				d = 1;
		} else
		if( !strcmpd( "/logs/operations.log", paramUrl )  )
		{
			CString filename = FN_OPERATIONS;
			if( ServeFile( os, filename.GetBuffer(0), outline ) )
				d = 1;
		} else
		if( !strcmpd( "/logs/errors.log", paramUrl )  )
		{
			CString filename = FN_ERRORS;
			if( ServeFile( os, filename.GetBuffer(0), outline ) )
				d = 1;
		} else
		if( !strcmpd( "/logs/", paramUrl )  )
		{
			GetURLPath( paramUrl+1, tmp );
			if( ServeFile( os, tmp, outline ) )
				d = 1;
		} else
		// return config details.
		if( !strcmpd( XMLFN_NEWS, paramUrl )  )
		{
			if( ServeFile( os, "Config/newsflash.xml", outline ) )
				d = 1;
		} else
		if( !strcmpd( XMLFN_MANIFEST, paramUrl )  )
		{
			if( ServeFile( os, "Config/manifest.xml", outline ) )
				d = 1;
		} else
		if( !strcmpd( XMLFN_CONFIG, paramUrl )  )
		{
			if( ServeFile( os, "Config/config.xml", outline ) )
				d = 1;
		} else
		if( !strcmpd( "/content/widgie.exe", paramUrl )  )
		{
			if( ServeFile( os, "widgie.exe", outline ) )
				d = 1;
		} else
		// MDID proxy return results, return slideshow
		if( !strcmpd( "/content/config/slideshow.xml", paramUrl ) ||
			!strcmpd( URL_MDID_GETSLIDESHOW, paramUrl ) )
		{
			CString filename;
			char *idP = strstr( paramUrl, "macid=" );
			if( idP )
			{
				idP = strchr( paramUrl, '=' );
				filename.Format( "Config/slideshow_%s.xml", idP+1 );
			} else
				filename.Format( "Config/slideshow.xml" );

			if( ServeFile( os, filename.GetBuffer(0), outline ) )
				d = 1;
		} else
		// MDID login
		if( !strcmpd( URL_MDID_LOGIN, paramUrl ) )			//URL_MDID_LOGIN 
		{
			if( ServeFile( os, "login.xml", outline ) )
				d = 1;
		} else
		// return image using MDID type url
		if( !strcmpd( XMLFN_GETIMAGE, paramUrl ) )
		{
			char *idP = strstr( paramUrl, "id=" );
			char *cidP = strstr( paramUrl, "cid=" );

			int id=0, cid=0;
			if( idP )
				id = atoi( idP+3 );
			if( cidP )
				cid = atoi( idP+4 );

			CString filename;
			filename.Format( "%s\\%d_%d.jpg", DEFPATH_CONTENT, cid,id );
			if( ServeFile( os, filename.GetBuffer(0), outline ) )
				d = 1;
		} else
		{
			GetURLPath( paramUrl+1, tmp );
			if( ServeFile( os, tmp, outline ) )
				d = 1;
		}

		if( !d )
		{
			len = MakeHTTPHeader( outline, NULL, -1 );
			sprintf( outline+len, "<H3>Error 404, file not found</H3><HR><BR>%s", paramUrl);
			d = SendStrHTTP( os, outline );
		}

		out = 1;
		free(outline);
	}
	return out;
}


