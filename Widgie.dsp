# Microsoft Developer Studio Project File - Name="Widgie" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Widgie - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Widgie.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Widgie.mak" CFG="Widgie - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Widgie - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Widgie - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Widgie - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "Widgie - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "C:\obj\Debug"
# PROP Intermediate_Dir "C:\obj\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\include\\" /I ".\include\cdx\\" /I "C:\Widgie_source" /D "CDX" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D _WIN32_WINNT=0x0400 /D "_AFXDLL" /FAcs /Fr /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vfw32.lib ddraw.lib dxguid.lib libs\xerces-c_2D.lib shlwapi.lib winmm.lib libs\cdx.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /out:".\RuntimeFolder\WidgieD.exe" /pdbtype:sept /static
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Widgie - Win32 Release"
# Name "Widgie - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=".\AVI PlayerDlg.cpp"
# End Source File
# Begin Source File

SOURCE=".\AVI PlayerDlg.h"
# End Source File
# Begin Source File

SOURCE=.\Widgie.cpp
# End Source File
# Begin Source File

SOURCE=.\Widgie.rc
# End Source File
# Begin Source File

SOURCE=.\WidgieDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\WidgieXML.cpp
# End Source File
# Begin Source File

SOURCE=.\flagDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\history.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageThread.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageThread.h
# End Source File
# Begin Source File

SOURCE=.\iTVPrinter.cpp
# End Source File
# Begin Source File

SOURCE=.\iTVPrinter.h
# End Source File
# Begin Source File

SOURCE=.\logoDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\LoopSeqInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\LoopSeqXML_Handler.cpp
# End Source File
# Begin Source File

SOURCE=.\LPTComm.cpp
# End Source File
# Begin Source File

SOURCE=.\ManifestInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\ManifestXML_Handler.cpp
# End Source File
# Begin Source File

SOURCE=.\mfcconsole.cpp
# End Source File
# Begin Source File

SOURCE=.\ModemControl.cpp
# End Source File
# Begin Source File

SOURCE=.\ModemControl.h
# End Source File
# Begin Source File

SOURCE=.\NewsBarThread.cpp
# End Source File
# Begin Source File

SOURCE=.\NewsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Newsflash.cpp
# End Source File
# Begin Source File

SOURCE=.\NewsflashInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\NewsXML_Handler.cpp
# End Source File
# Begin Source File

SOURCE=.\Picture.cpp
# End Source File
# Begin Source File

SOURCE=.\Serial.cpp
# End Source File
# Begin Source File

SOURCE=.\Serial.h
# End Source File
# Begin Source File

SOURCE=.\shockwaveflash.cpp
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\swFlashDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TaxiDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TaxiMeterThread.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Widgie.h
# End Source File
# Begin Source File

SOURCE=.\WidgieDlg.h
# End Source File
# Begin Source File

SOURCE=.\WidgieXML.h
# End Source File
# Begin Source File

SOURCE=.\flagDialog.h
# End Source File
# Begin Source File

SOURCE=.\history.h
# End Source File
# Begin Source File

SOURCE=.\LoopSeqInfo.h
# End Source File
# Begin Source File

SOURCE=.\LoopSeqXML_Handler.h
# End Source File
# Begin Source File

SOURCE=.\LPTComm.h
# End Source File
# Begin Source File

SOURCE=.\ManifestInfo.h
# End Source File
# Begin Source File

SOURCE=.\ManifestXML_Handler.h
# End Source File
# Begin Source File

SOURCE=.\NewsBarThread.h
# End Source File
# Begin Source File

SOURCE=.\NewsDialog.h
# End Source File
# Begin Source File

SOURCE=.\Newsflash.h
# End Source File
# Begin Source File

SOURCE=.\NewsflashInfo.h
# End Source File
# Begin Source File

SOURCE=.\NewsXML_Handler.h
# End Source File
# Begin Source File

SOURCE=.\Picture.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\shockwaveflash.h
# End Source File
# Begin Source File

SOURCE=.\Splash.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\swFlashDialog.h
# End Source File
# Begin Source File

SOURCE=.\TaxiDialog.h
# End Source File
# Begin Source File

SOURCE=.\TaxiMeterThread.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\VicLPT.h
# End Source File
# Begin Source File

SOURCE=.\viclptirq.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Widgie.ico
# End Source File
# Begin Source File

SOURCE=.\res\Widgie.rc2
# End Source File
# Begin Source File

SOURCE=.\res\taxi_meter.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\VicLPT.obj
# End Source File
# End Target
# End Project
# Section Widgie : {D27CDB6C-AE6D-11CF-96B8-444553540000}
# 	2:5:Class:CShockwaveFlash
# 	2:10:HeaderFile:shockwaveflash.h
# 	2:8:ImplFile:shockwaveflash.cpp
# End Section
# Section Widgie : {D27CDB6E-AE6D-11CF-96B8-444553540000}
# 	2:21:DefaultSinkHeaderFile:shockwaveflash.h
# 	2:16:DefaultSinkClass:CShockwaveFlash
# End Section
