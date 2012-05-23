# Microsoft Developer Studio Generated NMAKE File, Based on Dvorak.dsp
!IF "$(CFG)" == ""
CFG=Dvorak - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Dvorak - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Dvorak - Win32 Release" && "$(CFG)" != "Dvorak - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Dvorak.mak" CFG="Dvorak - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Dvorak - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Dvorak - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Dvorak - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Dvorak.exe"


CLEAN :
	-@erase "$(INTDIR)\Dvorak.obj"
	-@erase "$(INTDIR)\Dvorak.pch"
	-@erase "$(INTDIR)\Dvorak.res"
	-@erase "$(INTDIR)\DvorakDlg.obj"
	-@erase "$(INTDIR)\DvorakXML.obj"
	-@erase "$(INTDIR)\LoopSeqInfo.obj"
	-@erase "$(INTDIR)\LoopSeqXML_Handler.obj"
	-@erase "$(INTDIR)\ManifestInfo.obj"
	-@erase "$(INTDIR)\ManifestXML_Handler.obj"
	-@erase "$(INTDIR)\mfcconsole.obj"
	-@erase "$(INTDIR)\NewsBarThread.obj"
	-@erase "$(INTDIR)\NewsDialog.obj"
	-@erase "$(INTDIR)\Newsflash.obj"
	-@erase "$(INTDIR)\NewsflashInfo.obj"
	-@erase "$(INTDIR)\NewsXML_Handler.obj"
	-@erase "$(INTDIR)\Picture.obj"
	-@erase "$(INTDIR)\shockwaveflash.obj"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\swFlashDialog.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Dvorak.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\Dvorak.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc09 /fo"$(INTDIR)\Dvorak.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Dvorak.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Dvorak.pdb" /machine:I386 /out:"$(OUTDIR)\Dvorak.exe" 
LINK32_OBJS= \
	"$(INTDIR)\Dvorak.obj" \
	"$(INTDIR)\DvorakDlg.obj" \
	"$(INTDIR)\DvorakXML.obj" \
	"$(INTDIR)\LoopSeqInfo.obj" \
	"$(INTDIR)\LoopSeqXML_Handler.obj" \
	"$(INTDIR)\ManifestInfo.obj" \
	"$(INTDIR)\ManifestXML_Handler.obj" \
	"$(INTDIR)\NewsBarThread.obj" \
	"$(INTDIR)\NewsDialog.obj" \
	"$(INTDIR)\Newsflash.obj" \
	"$(INTDIR)\NewsflashInfo.obj" \
	"$(INTDIR)\NewsXML_Handler.obj" \
	"$(INTDIR)\Picture.obj" \
	"$(INTDIR)\shockwaveflash.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\swFlashDialog.obj" \
	"$(INTDIR)\Dvorak.res" \
	"$(INTDIR)\mfcconsole.obj"

"$(OUTDIR)\Dvorak.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Dvorak.exe" "$(OUTDIR)\Dvorak.bsc"


CLEAN :
	-@erase "$(INTDIR)\Dvorak.obj"
	-@erase "$(INTDIR)\Dvorak.pch"
	-@erase "$(INTDIR)\Dvorak.res"
	-@erase "$(INTDIR)\Dvorak.sbr"
	-@erase "$(INTDIR)\DvorakDlg.obj"
	-@erase "$(INTDIR)\DvorakDlg.sbr"
	-@erase "$(INTDIR)\DvorakXML.obj"
	-@erase "$(INTDIR)\DvorakXML.sbr"
	-@erase "$(INTDIR)\LoopSeqInfo.obj"
	-@erase "$(INTDIR)\LoopSeqInfo.sbr"
	-@erase "$(INTDIR)\LoopSeqXML_Handler.obj"
	-@erase "$(INTDIR)\LoopSeqXML_Handler.sbr"
	-@erase "$(INTDIR)\ManifestInfo.obj"
	-@erase "$(INTDIR)\ManifestInfo.sbr"
	-@erase "$(INTDIR)\ManifestXML_Handler.obj"
	-@erase "$(INTDIR)\ManifestXML_Handler.sbr"
	-@erase "$(INTDIR)\mfcconsole.obj"
	-@erase "$(INTDIR)\mfcconsole.sbr"
	-@erase "$(INTDIR)\NewsBarThread.obj"
	-@erase "$(INTDIR)\NewsBarThread.sbr"
	-@erase "$(INTDIR)\NewsDialog.obj"
	-@erase "$(INTDIR)\NewsDialog.sbr"
	-@erase "$(INTDIR)\Newsflash.obj"
	-@erase "$(INTDIR)\Newsflash.sbr"
	-@erase "$(INTDIR)\NewsflashInfo.obj"
	-@erase "$(INTDIR)\NewsflashInfo.sbr"
	-@erase "$(INTDIR)\NewsXML_Handler.obj"
	-@erase "$(INTDIR)\NewsXML_Handler.sbr"
	-@erase "$(INTDIR)\Picture.obj"
	-@erase "$(INTDIR)\Picture.sbr"
	-@erase "$(INTDIR)\shockwaveflash.obj"
	-@erase "$(INTDIR)\shockwaveflash.sbr"
	-@erase "$(INTDIR)\Splash.obj"
	-@erase "$(INTDIR)\Splash.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\swFlashDialog.obj"
	-@erase "$(INTDIR)\swFlashDialog.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Dvorak.bsc"
	-@erase "$(OUTDIR)\Dvorak.exe"
	-@erase "$(OUTDIR)\Dvorak.ilk"
	-@erase "$(OUTDIR)\Dvorak.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Dvorak.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc09 /fo"$(INTDIR)\Dvorak.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Dvorak.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Dvorak.sbr" \
	"$(INTDIR)\DvorakDlg.sbr" \
	"$(INTDIR)\DvorakXML.sbr" \
	"$(INTDIR)\LoopSeqInfo.sbr" \
	"$(INTDIR)\LoopSeqXML_Handler.sbr" \
	"$(INTDIR)\ManifestInfo.sbr" \
	"$(INTDIR)\ManifestXML_Handler.sbr" \
	"$(INTDIR)\NewsBarThread.sbr" \
	"$(INTDIR)\NewsDialog.sbr" \
	"$(INTDIR)\Newsflash.sbr" \
	"$(INTDIR)\NewsflashInfo.sbr" \
	"$(INTDIR)\NewsXML_Handler.sbr" \
	"$(INTDIR)\Picture.sbr" \
	"$(INTDIR)\shockwaveflash.sbr" \
	"$(INTDIR)\Splash.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\swFlashDialog.sbr" \
	"$(INTDIR)\mfcconsole.sbr"

"$(OUTDIR)\Dvorak.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=xerces-c_2.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\Dvorak.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Dvorak.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Dvorak.obj" \
	"$(INTDIR)\DvorakDlg.obj" \
	"$(INTDIR)\DvorakXML.obj" \
	"$(INTDIR)\LoopSeqInfo.obj" \
	"$(INTDIR)\LoopSeqXML_Handler.obj" \
	"$(INTDIR)\ManifestInfo.obj" \
	"$(INTDIR)\ManifestXML_Handler.obj" \
	"$(INTDIR)\NewsBarThread.obj" \
	"$(INTDIR)\NewsDialog.obj" \
	"$(INTDIR)\Newsflash.obj" \
	"$(INTDIR)\NewsflashInfo.obj" \
	"$(INTDIR)\NewsXML_Handler.obj" \
	"$(INTDIR)\Picture.obj" \
	"$(INTDIR)\shockwaveflash.obj" \
	"$(INTDIR)\Splash.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\swFlashDialog.obj" \
	"$(INTDIR)\Dvorak.res" \
	"$(INTDIR)\mfcconsole.obj"

"$(OUTDIR)\Dvorak.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Dvorak.dep")
!INCLUDE "Dvorak.dep"
!ELSE 
!MESSAGE Warning: cannot find "Dvorak.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Dvorak - Win32 Release" || "$(CFG)" == "Dvorak - Win32 Debug"
SOURCE=.\Dvorak.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\Dvorak.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\Dvorak.obj"	"$(INTDIR)\Dvorak.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\Dvorak.rc

"$(INTDIR)\Dvorak.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\DvorakDlg.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\DvorakDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\DvorakDlg.obj"	"$(INTDIR)\DvorakDlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\DvorakXML.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\DvorakXML.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\DvorakXML.obj"	"$(INTDIR)\DvorakXML.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\LoopSeqInfo.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\LoopSeqInfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\LoopSeqInfo.obj"	"$(INTDIR)\LoopSeqInfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\LoopSeqXML_Handler.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\LoopSeqXML_Handler.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\LoopSeqXML_Handler.obj"	"$(INTDIR)\LoopSeqXML_Handler.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\ManifestInfo.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\ManifestInfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\ManifestInfo.obj"	"$(INTDIR)\ManifestInfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\ManifestXML_Handler.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\ManifestXML_Handler.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\ManifestXML_Handler.obj"	"$(INTDIR)\ManifestXML_Handler.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\mfcconsole.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\mfcconsole.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\mfcconsole.obj"	"$(INTDIR)\mfcconsole.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\NewsBarThread.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\NewsBarThread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\NewsBarThread.obj"	"$(INTDIR)\NewsBarThread.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\NewsDialog.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\NewsDialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\NewsDialog.obj"	"$(INTDIR)\NewsDialog.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\Newsflash.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\Newsflash.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\Newsflash.obj"	"$(INTDIR)\Newsflash.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\NewsflashInfo.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\NewsflashInfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\NewsflashInfo.obj"	"$(INTDIR)\NewsflashInfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\NewsXML_Handler.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\NewsXML_Handler.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\NewsXML_Handler.obj"	"$(INTDIR)\NewsXML_Handler.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\Picture.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\Picture.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\Picture.obj"	"$(INTDIR)\Picture.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\shockwaveflash.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\shockwaveflash.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\shockwaveflash.obj"	"$(INTDIR)\shockwaveflash.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\Splash.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\Splash.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\Splash.obj"	"$(INTDIR)\Splash.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\Dvorak.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\Dvorak.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Dvorak.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\Dvorak.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\swFlashDialog.cpp

!IF  "$(CFG)" == "Dvorak - Win32 Release"


"$(INTDIR)\swFlashDialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ELSEIF  "$(CFG)" == "Dvorak - Win32 Debug"


"$(INTDIR)\swFlashDialog.obj"	"$(INTDIR)\swFlashDialog.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Dvorak.pch"


!ENDIF 


!ENDIF 

