// tstDlg.cpp : implementation file
//

#include "stdafx.h"
#include "tst.h"
#include "tstDlg.h"

#include "ras.h"
#include "raserror.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTstDlg dialog

CTstDlg::CTstDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTstDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTstDlg)
	m_strPassword = _T("");
	m_strPhoneNumber = _T("");
	m_strUserName = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTstDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTstDlg)
	DDX_Text(pDX, IDC_EDIT_PASSWD, m_strPassword);
	DDX_Text(pDX, IDC_EDIT_PHONE, m_strPhoneNumber);
	DDX_Text(pDX, IDC_EDIT_USER, m_strUserName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTstDlg, CDialog)
	//{{AFX_MSG_MAP(CTstDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_HANG_UP, OnButtonHangUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTstDlg message handlers

BOOL CTstDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTstDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTstDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



 


bool  CTstDlg::DialUp() 
{
    RASDIALPARAMS rdParams;
    rdParams.dwSize = sizeof(RASDIALPARAMS);
	rdParams.szEntryName[0] = '\0';
	lstrcpy( rdParams.szPhoneNumber, m_strPhoneNumber );
	rdParams.szCallbackNumber[0] = '\0';
	lstrcpy( rdParams.szUserName, m_strUserName );
	lstrcpy( rdParams.szPassword, m_strPassword );
	rdParams.szDomain[0] = '\0';

	HRASCONN hRasConn = NULL;
    DWORD dwRet = RasDial( NULL, NULL, &rdParams, 0L, NULL, &hRasConn );
    if ( dwRet == 0 )  return true;
    char  szBuf[256];
	if ( RasGetErrorString( (UINT)dwRet, (LPSTR)szBuf, 256 ) != 0 )
		wsprintf( (LPSTR)szBuf, "Undefined RAS Dial Error (%ld).", dwRet );
	RasHangUp( hRasConn );
	MessageBox( (LPSTR)szBuf, "Error", MB_OK | MB_ICONSTOP );
	return false;
}



bool  CTstDlg::HangUp() 
{
	RASCONN ras[20];
	DWORD  dSize, dNumber;
	char  szBuf[256];
	
	ras[0].dwSize = sizeof( RASCONN );
	dSize = sizeof( ras );   // Get active RAS - Connection
	DWORD  dwRet = RasEnumConnections( ras, &dSize, &dNumber );
	if ( dwRet != 0 )
	{
		if ( RasGetErrorString( (UINT)dwRet, (LPSTR)szBuf, 256 ) != 0 )
			wsprintf( (LPSTR)szBuf, "Undefined RAS Enum Connections error (%ld).", dwRet );
		MessageBox( (LPSTR)szBuf, "RasHangUp", MB_OK | MB_ICONSTOP );
		return false;
	}
	bool bOK = true;
	for( DWORD dCount = 0;  dCount < dNumber;  dCount++ )
	{    // Hang up that connection
		HRASCONN hRasConn = ras[dCount].hrasconn;
		DWORD dwRet = RasHangUp( hRasConn );
		if ( dwRet != 0 )
		{
			char  szBuf[256];
			if ( RasGetErrorString( (UINT)dwRet, (LPSTR)szBuf, 256 ) != 0 )
				wsprintf( (LPSTR)szBuf, "Undefined RAS HangUp Error (%ld).", dwRet );
			MessageBox( (LPSTR)szBuf, "RasHangUp", MB_OK | MB_ICONSTOP );
			bOK = false;
		}
	}
	return bOK;
}





void CTstDlg::OnButtonConnect() 
{
	UpdateData( TRUE );
	DialUp();	
	
	CDialog::OnCancel();
}



void CTstDlg::OnButtonHangUp() 
{
	HangUp();	
}
