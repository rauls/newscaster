#include "stdafx.h"
#include "SmartInfo.h"
#include "winioctl.h"
#include "windows.h"
#include "SmartDef.h"

BYTE	AttrOutCmd[sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE - 1];
BYTE	ThreshOutCmd[sizeof(SENDCMDOUTPARAMS) + READ_THRESHOLD_BUFFER_SIZE - 1];
BYTE	IdOutCmd[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];

/*
	IDE Commands using S.M.A.R.T
*/
BOOL DoIDENTIFY(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, BYTE, PDWORD);
BOOL DoEnableSMART(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE, PDWORD);
BOOL DoReadAttributesCmd(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE );
BOOL DoReadThresholdsCmd(HANDLE, PSENDCMDINPARAMS, PSENDCMDOUTPARAMS, BYTE );
VOID ChangeByteOrder(PCHAR, USHORT);
HANDLE OpenSMART(VOID);
int DoGetAttrData(PCHAR pAttrBuffer, PCHAR pThrsBuffer, BYTE bIDNum);
CString DoPrintData(PCHAR, PCHAR, BYTE);
CString DisplayIdInfo(PIDSECTOR, PSENDCMDINPARAMS, BYTE, BYTE, BYTE);
CString PrintIDERegs(PSENDCMDINPARAMS);



CString GetCommandError()
{
	CString Result;

	DWORD dwError=GetLastError();
	LPVOID lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
		);
	
	Result.Format("Error: %s\r\n",(char*) lpMsgBuf);
	LocalFree( lpMsgBuf );

	return Result;
}



CString QueryHDDSmartCommand( int driveNumber, int smartCommand, long &resultData )
{
	HANDLE      		hSMARTIOCTL;
	DWORD       		cbBytesReturned;
	GETVERSIONOUTPARAMS VersionParams;
	SENDCMDINPARAMS 	scip;
	SENDCMDOUTPARAMS	OutCmd;
	BYTE				bDfpDriveMap;
	BYTE				bSuccess;
	BYTE				bIDCmd;		// IDE or ATAPI IDENTIFY cmd

	CString Result;

	char Temp[512]="";

//#ifdef WINDOWS9X

	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&os);
	if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		// Version Windows 95 OSR2, Windows 98
		if ((hSMARTIOCTL = CreateFile("\\\\.\\SMARTVSD", 0,0,0,
			CREATE_NEW, 0, 0)) == INVALID_HANDLE_VALUE)
		{
			sprintf_s(Temp,"Unable to open SMARTVSD, error code: 0x%lX\r\n", GetLastError());
		}
		else
		{
			sprintf_s(Temp,"SMARTVSD opened successfully\r\n");
		}
		
		Result+=Temp;
	}
	else
	{
		// Windows NT, Windows 2000
		if ((hSMARTIOCTL = CreateFile("\\\\.\\PhysicalDrive0",GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
			OPEN_EXISTING,0,NULL)) == INVALID_HANDLE_VALUE)
		{
			sprintf_s(Temp, "Unable to open physical drive, error code: 0x%lX\r\n", GetLastError());
		}
		/*
		else
				{
					sprintf_s(Temp, "Physical drive opened successfully\r\n");
				}
		*/
		
		Result+=Temp;
	}
	
//#else
//#endif

	if (hSMARTIOCTL != INVALID_HANDLE_VALUE)
    {
		//
		// Get the version, etc of SMART IOCTL
		//
		memset((void*)&VersionParams, 0, sizeof(VersionParams));

		if (DeviceIoControl(hSMARTIOCTL, DFP_GET_VERSION,
			NULL, 
			0,
			&VersionParams,
			sizeof(VersionParams),
			&cbBytesReturned, NULL) )
    {
		sprintf_s(Temp,"DFP_GET_VERSION returned:\r\n");
		Result+=Temp;
		sprintf_s(Temp,"\tbVersion        = %d\r\n", VersionParams.bVersion);
		Result+=Temp;
		sprintf_s(Temp,"\tbRevision       = %d\r\n", VersionParams.bRevision);
		Result+=Temp;
		sprintf_s(Temp,"\tfCapabilities   = 0x%lx\r\n", VersionParams.fCapabilities);
		Result+=Temp;
		sprintf_s(Temp,"\tbReserved       = 0x%x\r\n", VersionParams.bReserved);
		Result+=Temp;
		sprintf_s(Temp,"\tbIDEDeviceMap   = 0x%x\r\n", VersionParams.bIDEDeviceMap);
		Result+=Temp;
		sprintf_s(Temp,"\tcbBytesReturned = %d\r\n\r\n", cbBytesReturned);
		Result+=Temp;

		// force drive 0 to be looked at.
		VersionParams.bIDEDeviceMap = 1;

	}
    else
    {
        sprintf_s(Temp,"DFP_GET_VERSION failed.\r\n");
		Result+=Temp;
    }


	//for (i = 0; i < MAX_IDE_DRIVES; i++)
	{
		int i = driveNumber;
		//
		// If there is a IDE device at number "i" issue commands
		// to the device.
		//
		if (VersionParams.bIDEDeviceMap >> i & 1)
		{

			//
			// Try to enable SMART so we can tell if a drive supports it.
			// Ignore ATAPI devices.
			//

			if (!(VersionParams.bIDEDeviceMap >> i & 0x10))
			{

				memset(&scip, 0, sizeof(scip));
				memset(&OutCmd, 0, sizeof(OutCmd));

	    		if (DoEnableSMART(hSMARTIOCTL, 
					&scip, 
					&OutCmd, 
					i,
					&cbBytesReturned))
				{
					sprintf_s(Temp,"SMART Enabled on Drive: %d\r\n", i);
					Result+=Temp;
					//
					// Mark the drive as SMART enabled
					//
					bDfpDriveMap |= (1 << i);
				}
				else
	    		{
					sprintf_s(Temp,"SMART Enable Command Failed, Drive: %d.\r\n",i);
					Result+=Temp;
					sprintf_s(Temp," DriverStatus: bDriverError=0x%X, bIDEStatus=0x%X\r\n\r\n", 
	    					OutCmd.DriverStatus.bDriverError, 
	    					OutCmd.DriverStatus.bIDEError);
					Result+=Temp;
	    		}
				sprintf_s(Temp,"\tcbBytesReturned: %d\r\n\r\n", cbBytesReturned);
				Result+=Temp;
			}


			//
			// Now, get the ID sector for all IDE devices in the system.
			// If the device is ATAPI use the IDE_ATAPI_ID command,
			// otherwise use the IDE_ID_FUNCTION command.
			//
			bIDCmd = (VersionParams.bIDEDeviceMap >> i & 0x10) ? \
				IDE_ATAPI_ID : IDE_ID_FUNCTION;

			memset(&scip, 0, sizeof(scip));
			memset(IdOutCmd, 0, sizeof(IdOutCmd));

			if (DoIDENTIFY(hSMARTIOCTL, 
					&scip, 
					(PSENDCMDOUTPARAMS)&IdOutCmd, 
					bIDCmd,
					i,
					&cbBytesReturned))
	        {
				Result+=
					DisplayIdInfo((PIDSECTOR) ((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer,
						&scip, bIDCmd, bDfpDriveMap, i);
			}

	       	else
	       		{
	       			sprintf_s(Temp,"Identify Command Failed on Drive: %d\r\n", i);
					Result+=Temp;
					sprintf_s(Temp," DriverStatus: bDriverError=0x%X, bIDEStatus=0x%X\r\n\r\n", 
					((PSENDCMDOUTPARAMS)IdOutCmd)->DriverStatus.bDriverError, 
					((PSENDCMDOUTPARAMS)IdOutCmd)->DriverStatus.bIDEError);
					Result+=Temp;
	        	}
	    		sprintf_s(Temp,"\tcbBytesReturned: %d\r\n\r\n", cbBytesReturned);
				Result+=Temp;
	    }

	}

	//
	// Loop through all possible IDE drives and send commands to the ones that support SMART.
	//
	//for (i = 0; i < MAX_IDE_DRIVES; i++)
	{
		int i = driveNumber;

		if (bDfpDriveMap >> i & 1)
		{

			memset(AttrOutCmd, 0, sizeof(AttrOutCmd));
			memset(ThreshOutCmd, 0, sizeof(ThreshOutCmd));

			if ( !(bSuccess = DoReadAttributesCmd(hSMARTIOCTL, 
					&scip, 
					(PSENDCMDOUTPARAMS)&AttrOutCmd, 
					i)))
			{
				sprintf_s(Temp,"\r\nSMART Read Attr Command Failed on Drive: %d.\r\n", i);
				sprintf_s(Temp," DriverStatus: bDriverError=0x%X, bIDEStatus=0x%X\r\n\r\n", 
					((PSENDCMDOUTPARAMS)AttrOutCmd)->DriverStatus.bDriverError, 
					((PSENDCMDOUTPARAMS)AttrOutCmd)->DriverStatus.bIDEError);
			}	
	
			// ReadAttributes worked. Try ReadThresholds.
			else if (!(DoReadThresholdsCmd(hSMARTIOCTL, &scip, 
							(PSENDCMDOUTPARAMS)&ThreshOutCmd, i)))
			{
				sprintf_s(Temp,"\r\nSMART Read Thrsh Command Failed on Drive: %d.\r\n", i);
				Result+=Temp;
				sprintf_s(Temp," DriverStatus: bDriverError=0x%X, bIDEStatus=0x%X\r\n\r\n", 
					((PSENDCMDOUTPARAMS)ThreshOutCmd)->DriverStatus.bDriverError, 
					((PSENDCMDOUTPARAMS)ThreshOutCmd)->DriverStatus.bIDEError);
				Result+=Temp;
			}

			//
			// The following report will print if ReadAttributes works.
			// If ReadThresholds works, the report will also show values for
			// Threshold values.
			//
			if (bSuccess)
			{
				Result+=
					DoPrintData((char*) ((PSENDCMDOUTPARAMS) AttrOutCmd)->bBuffer, 
						(char*) ((PSENDCMDOUTPARAMS)ThreshOutCmd)->bBuffer, i);

				resultData = DoGetAttrData((char*) ((PSENDCMDOUTPARAMS) AttrOutCmd)->bBuffer,
						(char*) ((PSENDCMDOUTPARAMS)ThreshOutCmd)->bBuffer, smartCommand );
			}
	
		}
	}

	//
    // Close SMART.
	//
        CloseHandle(hSMARTIOCTL);
    }

	return Result;
}


int QueryHDDSmartTemp( int driveNumber )
{
	CString resultStr;
	long temp;
	
	resultStr = QueryHDDSmartCommand( 0, SMART_TEMP, temp );

	//if (temp>>16 == 0xC)			// its Celcius, else if its 0xF its Farenheight
	return (temp&0xff);
}



BOOL DoIDENTIFY(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP,
	PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum, PDWORD lpcbBytesReturned)
{
	//
	// Set up data structures for IDENTIFY command.
	//

	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

	pSCIP->irDriveRegs.bFeaturesReg = 0;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = 0;
	pSCIP->irDriveRegs.bCylHighReg = 0;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 

	//
	// The command can either be IDE identify or ATAPI identify.
	//
	pSCIP->irDriveRegs.bCommandReg = bIDCmd;
	pSCIP->bDriveNumber = bDriveNum;
	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

    return (DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
             	(LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
               	(LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
               	lpcbBytesReturned, NULL) );
}

/****************************************************************************
*
* DisplayIdInfo
*
* Display the contents of the ID buffer
*
****************************************************************************/
CString DisplayIdInfo(PIDSECTOR pids, PSENDCMDINPARAMS pSCIP, BYTE bIDCmd, BYTE bDfpDriveMap, BYTE bDriveNum)
{
	BYTE	szOutBuffer[41];
	CString Result;

	if (bIDCmd == IDE_ID_FUNCTION)
	{
		char Temp[512]="";
		sprintf_s(Temp,"Drive %d is an IDE Hard drive%s\r\n", bDriveNum, (
			bDfpDriveMap >> bDriveNum & 1) ? " that supports SMART" : "");

		Result+=Temp;
		sprintf_s(Temp,"\t#Cylinders: %d, #Heads: %d, #Sectors per Track: %d\r\n",
				pids->wNumCyls, 
				pids->wNumHeads, 
				pids->wSectorsPerTrack);
		Result+=Temp;

		PrintIDERegs(pSCIP);

	}
	else
	{
		char Temp[512]="";
		sprintf_s(Temp,"Drive %d is an ATAPI device.\r\n", bDriveNum); 
		Result+=Temp;
	}

	//
	// Change the WORD array to a BYTE
	// array
	//
	ChangeByteOrder(pids->sModelNumber, 
		sizeof pids->sModelNumber);

	memset(szOutBuffer,0, sizeof(szOutBuffer));
	strncpy_s((char*) szOutBuffer, sizeof(szOutBuffer), pids->sModelNumber, sizeof(pids->sModelNumber));

	char Temp[512]="";
	sprintf_s(Temp, "\tModel number: %s\r\n", szOutBuffer);
	Result+=Temp;


	//
	// Change the WORD array to a BYTE
	// array
	//
	ChangeByteOrder(pids->sFirmwareRev, 
		sizeof pids->sFirmwareRev);

	memset(szOutBuffer,0, sizeof(szOutBuffer));
	strncpy_s((char*) szOutBuffer, sizeof(szOutBuffer), pids->sFirmwareRev, sizeof(pids->sFirmwareRev));

	sprintf_s(Temp, "\tFirmware rev: %s\r\n", szOutBuffer);
	Result+=Temp;

	//
	// Change the WORD array to a BYTE
	// array
	//
	ChangeByteOrder(pids->sSerialNumber, 
		sizeof pids->sSerialNumber);

	memset(szOutBuffer,0, sizeof(szOutBuffer));
	strncpy_s((char*) szOutBuffer, sizeof(szOutBuffer), pids->sSerialNumber, sizeof(pids->sSerialNumber));

	sprintf_s(Temp, "\tSerial number: %s\r\n",szOutBuffer);
	Result+=Temp;

	return Result;
}

/****************************************************************************
*
* DoEnableSMART
*
* FUNCTION: Send a SMART_ENABLE_SMART_OPERATIONS command to the drive
* bDriveNum = 0-3
*
****************************************************************************/
BOOL DoEnableSMART(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum, PDWORD lpcbBytesReturned)
{
	//
	// Set up data structures for Enable SMART Command.
	//
	pSCIP->cBufferSize = 0;

	pSCIP->irDriveRegs.bFeaturesReg = SMART_ENABLE_SMART_OPERATIONS;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	pSCIP->irDriveRegs.bCylHighReg = SMART_CYL_HI;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	pSCIP->irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	pSCIP->bDriveNumber = bDriveNum;

        return ( DeviceIoControl(hSMARTIOCTL, DFP_SEND_DRIVE_COMMAND,
                (LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
                (LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) - 1,
                lpcbBytesReturned, NULL) );
}

/****************************************************************************
*
* DoReadAttributesCmd
*
* FUNCTION: Send a SMART_READ_ATTRIBUTE_VALUES command to the drive
* bDriveNum = 0-3
*
****************************************************************************/
BOOL DoReadAttributesCmd(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum)
{
DWORD	cbBytesReturned;

	//
	// Set up data structures for Read Attributes SMART Command.
	//

	pSCIP->cBufferSize = READ_ATTRIBUTE_BUFFER_SIZE;

	pSCIP->irDriveRegs.bFeaturesReg = SMART_READ_ATTRIBUTE_VALUES;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	pSCIP->irDriveRegs.bCylHighReg = SMART_CYL_HI;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	pSCIP->irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	pSCIP->bDriveNumber = bDriveNum;
        return ( DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
                (LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
                (LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE - 1,
                &cbBytesReturned, NULL) );
}

/****************************************************************************
*
* DoReadThresholdsCmd
*
* FUNCTION: Send a SMART_READ_ATTRIBUTE_THRESHOLDS command to the drive
* bDriveNum = 0-3
*
****************************************************************************/
BOOL DoReadThresholdsCmd(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum)
{
DWORD	cbBytesReturned;

	//
	// Set up data structures for Read Thresholds SMART Command.
	//

	pSCIP->cBufferSize = READ_THRESHOLD_BUFFER_SIZE;

	pSCIP->irDriveRegs.bFeaturesReg = SMART_READ_ATTRIBUTE_THRESHOLDS;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	pSCIP->irDriveRegs.bCylHighReg = SMART_CYL_HI;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	pSCIP->irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	pSCIP->bDriveNumber = bDriveNum;


        return ( DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
                (LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
                (LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + READ_THRESHOLD_BUFFER_SIZE - 1,
                &cbBytesReturned, NULL) );
}



/****************************************************************************
*
* DoGetAttrData
*
* FUNCTION: Get the SMART Attributes and Thresholds
*
****************************************************************************/
int DoGetAttrData(PCHAR pAttrBuffer, PCHAR pThrsBuffer, BYTE bIDNum)
{
	int	result=0;
	PDRIVEATTRIBUTE	pDA;
	PATTRTHRESHOLD	pAT;

	pDA = (PDRIVEATTRIBUTE)&pAttrBuffer[2];
	pAT = (PATTRTHRESHOLD)&pThrsBuffer[2];

	int c=0;
	while( pDA && (pDA->bAttrID!=bIDNum) && c<(512/7))
	{
		pDA++;
		c++;
	}

	if( pDA->bAttrID==bIDNum )
	{

		result = pDA->bRawValue[0];
		//pAT->bWarrantyThreshold );
	}
	return result;
}



/****************************************************************************
*
* DoPrintData
*
* FUNCTION: Display the SMART Attributes and Thresholds
*
****************************************************************************/
CString DoPrintData(PCHAR pAttrBuffer, PCHAR pThrsBuffer, BYTE bDriveNum)
{
	int	i;
	PDRIVEATTRIBUTE	pDA;
	PATTRTHRESHOLD	pAT;
	BYTE Attr;
	CString Result;
	
	//
	// Print the drive number
	//
	char Temp[512]="";
	sprintf_s(Temp,"\r\nData for Drive Number %d\n", bDriveNum);
	Result+=Temp;
	//
	// Print the revisions of the data structures
	//
	sprintf_s(Temp, "Attribute Structure Revision          Threshold Structure Revision\r\n");
	Result+=Temp;

	sprintf_s(Temp,"             %d                                      %d\r\n\r\n", 
				(WORD)pAttrBuffer[0], 
				(WORD)pThrsBuffer[0]);

	Result+=Temp;
	//
	// Print the header and loop through the structures, printing
	// the structures when the attribute ID is known.
	//
	sprintf_s(Temp, "   -Attribute Name-      -Attribute Value-     -Threshold Value-\r\n");
	Result+=Temp;

	pDA = (PDRIVEATTRIBUTE)&pAttrBuffer[2];
	pAT = (PATTRTHRESHOLD)&pThrsBuffer[2];

	for (i = 0; i < NUM_ATTRIBUTE_STRUCTS; i++)
	{
		Attr = pDA->bAttrID;
		if (Attr)
		{
			if (Attr > MAX_KNOWN_ATTRIBUTES)
				Attr = MAX_KNOWN_ATTRIBUTES+1;
			sprintf_s(Temp,"%2X %-29s%d%20c%d\r\n", 
					pDA->bAttrID,
					pAttrNames[Attr], 
					pDA->bAttrValue, 
					' ',
					pAT->bWarrantyThreshold );
			Result+=Temp;
		}
		pDA++;
		pAT++;
	}

	return Result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
VOID ChangeByteOrder(PCHAR szString, USHORT uscStrSize)
{
	USHORT	i;
	CHAR	temp;

	for (i = 0; i < uscStrSize; i+=2)
	{
		temp = szString[i];
		szString[i] = szString[i+1];
		szString[i+1] = temp;
	}
}

//---------------------------------------------------------------------
// Display contents of IDE hardware registers reported by SMART
//---------------------------------------------------------------------
CString PrintIDERegs(PSENDCMDINPARAMS pscip)
{
	char Temp[512]="";
	CString Result;

	sprintf_s(Temp,"\tIDE TASK FILE REGISTERS:\r\n");
	Result+=Temp;

	sprintf_s(Temp,"\t\tbFeaturesReg     = 0x%X\r\n", pscip->irDriveRegs.bFeaturesReg);
	Result+=Temp;
	sprintf_s(Temp,"\t\tbSectorCountReg  = 0x%X\r\n", pscip->irDriveRegs.bSectorCountReg);
	Result+=Temp;
	sprintf_s(Temp,"\t\tbSectorNumberReg = 0x%X\r\n", pscip->irDriveRegs.bSectorNumberReg);
	Result+=Temp;
	sprintf_s(Temp,"\t\tbCylLowReg       = 0x%X\r\n", pscip->irDriveRegs.bCylLowReg);
	Result+=Temp;
	sprintf_s(Temp,"\t\tbCylHighReg      = 0x%X\r\n", pscip->irDriveRegs.bCylHighReg);
	Result+=Temp;
	sprintf_s(Temp,"\t\tbDriveHeadReg    = 0x%X\r\n", pscip->irDriveRegs.bDriveHeadReg); 
	Result+=Temp;
	sprintf_s(Temp,"\t\tStatus           = 0x%X\r\n", pscip->irDriveRegs.bCommandReg);
	Result+=Temp;

	return Result;
}
