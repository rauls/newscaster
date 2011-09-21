; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CTaxiDialog
LastTemplate=CWinThread
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Dvorak.h"

ClassCount=9
Class1=CDvorakApp
Class2=CDvorakDlg

ResourceCount=15
Resource2=IDD_NEWSFLASH_DIALOG1 (English (Australia))
Resource3=IDD_TAXI_METER_DIALOG
Resource1=IDR_MAINFRAME
Class3=NewsDialog
Resource4=IDD_SHOCKWAVE_DIALOG
Class4=swFlashDialog
Class5=NewsBarThread
Resource5=IDD_FLAG_DIALOG
Class6=CSplash
Resource6=IDD_TAXI_METER_DIALOG (English (Australia))
Class7=flagDialog
Resource7=IDD_SPLASH
Class8=CTaxiDialog
Class9=TaxiMeterThread
Resource8=IDD_NEWSFLASH_DIALOG1
Resource9=IDD_DVORAK_DIALOG
Resource10=IDD_SHOCKWAVE_DIALOG (English (Australia))
Resource11=IDD_SPLASH (English (Australia))
Resource12=IDD_FLAG_DIALOG (English (Australia))
Resource13=IDD_LOGO_DIALOG
Resource14=IDD_DVORAK_DIALOG (English (U.S.))
Resource15=IDD_VIDEO_DIALOG

[CLS:CDvorakApp]
Type=0
HeaderFile=Dvorak.h
ImplementationFile=Dvorak.cpp
Filter=N

[CLS:CDvorakDlg]
Type=0
HeaderFile=DvorakDlg.h
ImplementationFile=DvorakDlg.cpp
Filter=D
LastObject=CDvorakDlg
BaseClass=CDialog
VirtualFilter=dWC



[DLG:IDD_DVORAK_DIALOG]
Type=1
Class=CDvorakDlg
ControlCount=0

[DLG:IDD_DVORAK_DIALOG (English (U.S.))]
Type=1
Class=CDvorakDlg
ControlCount=0

[DLG:IDD_NEWSFLASH_DIALOG1]
Type=1
Class=NewsDialog
ControlCount=0

[CLS:NewsDialog]
Type=0
HeaderFile=NewsDialog.h
ImplementationFile=NewsDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=NewsDialog
VirtualFilter=dWC

[DLG:IDD_SHOCKWAVE_DIALOG]
Type=1
Class=swFlashDialog
ControlCount=1
Control1=IDC_SHOCKWAVEFLASH1,{D27CDB6E-AE6D-11CF-96B8-444553540000},1342177280

[CLS:swFlashDialog]
Type=0
HeaderFile=swFlashDialog.h
ImplementationFile=swFlashDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=swFlashDialog
VirtualFilter=dWC

[CLS:NewsBarThread]
Type=0
HeaderFile=NewsBarThread.h
ImplementationFile=NewsBarThread.cpp
BaseClass=CWinThread
Filter=N
LastObject=NewsBarThread

[DLG:IDD_SPLASH]
Type=1
Class=CSplash
ControlCount=0

[CLS:CSplash]
Type=0
HeaderFile=Splash.h
ImplementationFile=Splash.cpp
BaseClass=CDialog
Filter=D
LastObject=CSplash

[DLG:IDD_FLAG_DIALOG]
Type=1
Class=flagDialog
ControlCount=0

[CLS:flagDialog]
Type=0
HeaderFile=flagDialog.h
ImplementationFile=flagDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=flagDialog
VirtualFilter=dWC

[DLG:IDD_TAXI_METER_DIALOG]
Type=1
Class=CTaxiDialog
ControlCount=0

[CLS:CTaxiDialog]
Type=0
HeaderFile=TaxiDialog.h
ImplementationFile=TaxiDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=CTaxiDialog
VirtualFilter=dWC

[CLS:TaxiMeterThread]
Type=0
HeaderFile=TaxiMeterThread.h
ImplementationFile=TaxiMeterThread.cpp
BaseClass=CWinThread
Filter=N
LastObject=TaxiMeterThread

[DLG:IDD_FLAG_DIALOG (English (Australia))]
Type=1
Class=flagDialog
ControlCount=0

[DLG:IDD_NEWSFLASH_DIALOG1 (English (Australia))]
Type=1
Class=NewsDialog
ControlCount=0

[DLG:IDD_SHOCKWAVE_DIALOG (English (Australia))]
Type=1
Class=swFlashDialog
ControlCount=1
Control1=IDC_SHOCKWAVEFLASH1,{D27CDB6E-AE6D-11CF-96B8-444553540000},1342177280

[DLG:IDD_SPLASH (English (Australia))]
Type=1
Class=CSplash
ControlCount=0

[DLG:IDD_TAXI_METER_DIALOG (English (Australia))]
Type=1
Class=CTaxiDialog
ControlCount=0

[DLG:IDD_LOGO_DIALOG]
Type=1
Class=?
ControlCount=0

[DLG:IDD_VIDEO_DIALOG]
Type=1
Class=?
ControlCount=0

