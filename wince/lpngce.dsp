# Microsoft Developer Studio Project File - Name="lpngce" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (WCE x86em) Dynamic-Link Library" 0x7f02
# TARGTYPE "Win32 (WCE SH3) Dynamic-Link Library" 0x8102
# TARGTYPE "Win32 (WCE PPC) Dynamic-Link Library" 0x8402
# TARGTYPE "Win32 (WCE x86) Dynamic-Link Library" 0x8302
# TARGTYPE "Win32 (WCE MIPSFP) Dynamic-Link Library" 0x8702
# TARGTYPE "Win32 (WCE ARM) Dynamic-Link Library" 0x8502
# TARGTYPE "Win32 (WCE SH4) Dynamic-Link Library" 0x8602
# TARGTYPE "Win32 (WCE MIPS) Dynamic-Link Library" 0x8202

CFG=lpngce - Win32 (WCE MIPS) Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "lpngce.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lpngce.mak" CFG="lpngce - Win32 (WCE MIPS) Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lpngce - Win32 (WCE MIPS) Release" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE MIPS) Debug" (based on "Win32 (WCE MIPS) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE SH4) Release" (based on "Win32 (WCE SH4) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE SH4) Debug" (based on "Win32 (WCE SH4) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE SH3) Release" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE SH3) Debug" (based on "Win32 (WCE SH3) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE ARM) Release" (based on "Win32 (WCE ARM) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE ARM) Debug" (based on "Win32 (WCE ARM) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE MIPSFP) Release" (based on "Win32 (WCE MIPSFP) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE MIPSFP) Debug" (based on "Win32 (WCE MIPSFP) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE x86) Release" (based on "Win32 (WCE x86) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE x86) Debug" (based on "Win32 (WCE x86) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE x86em) Release" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE x86em) Debug" (based on "Win32 (WCE x86em) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE PPC) Release" (based on "Win32 (WCE PPC) Dynamic-Link Library")
!MESSAGE "lpngce - Win32 (WCE PPC) Debug" (based on "Win32 (WCE PPC) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "H/PC Ver. 2.00"
# PROP WCE_FormatVersion "6.0"

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WMIPSRel"
# PROP BASE Intermediate_Dir "WMIPSRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WMIPSRel"
# PROP Intermediate_Dir "WMIPSRel"
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /M$(CECrt) /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /QMRWCE /c
# ADD CPP /nologo /M$(CECrt) /W3 /O2 /I "..\..\zlib" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /QMRWCE /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WMIPSDbg"
# PROP BASE Intermediate_Dir "WMIPSDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WMIPSDbg"
# PROP Intermediate_Dir "WMIPSDbg"
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /QMRWCE /c
# ADD CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /I ".." /I "..\..\zlib" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /QMRWCE /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCESH4Rel"
# PROP BASE Intermediate_Dir "WCESH4Rel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCESH4Rel"
# PROP Intermediate_Dir "WCESH4Rel"
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /Qsh4 /MC /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /Qsh4 /MC /W3 /O2 /I ".." /I "..\..\zlib" /D "NDEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "SHx" /d "SH4" /d "_SH4_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "SHx" /d "SH4" /d "_SH4_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESH4Dbg"
# PROP BASE Intermediate_Dir "WCESH4Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESH4Dbg"
# PROP Intermediate_Dir "WCESH4Dbg"
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /Qsh4 /MC /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /Qsh4 /MC /W3 /Zi /Od /I ".." /I "..\..\zlib" /D "DEBUG" /D "SHx" /D "SH4" /D "_SH4_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "SHx" /d "SH4" /d "_SH4_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "SHx" /d "SH4" /d "_SH4_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:SH4 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCESH3Rel"
# PROP BASE Intermediate_Dir "WCESH3Rel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCESH3Rel"
# PROP Intermediate_Dir "WCESH3Rel"
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /M$(CECrt) /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /M$(CECrt) /W3 /O2 /I ".." /I "..\..\zlib" /D "NDEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCESH3Dbg"
# PROP BASE Intermediate_Dir "WCESH3Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCESH3Dbg"
# PROP Intermediate_Dir "WCESH3Dbg"
# PROP Target_Dir ""
CPP=shcl.exe
# ADD BASE CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /I ".." /I "..\..\zlib" /D "DEBUG" /D "SHx" /D "SH3" /D "_SH3_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "SHx" /d "SH3" /d "_SH3_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:SH3 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCEARMRel"
# PROP BASE Intermediate_Dir "WCEARMRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCEARMRel"
# PROP Intermediate_Dir "WCEARMRel"
# PROP Target_Dir ""
CPP=clarm.exe
# ADD BASE CPP /nologo /MC /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /MC /W3 /O2 /I ".." /I "..\..\zlib" /D "NDEBUG" /D "ARM" /D "_ARM_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "ARM" /d "_ARM_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "ARM" /d "_ARM_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 coredll.lib commctrl.lib /nologo /dll /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 coredll.lib commctrl.lib /nologo /dll /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCEARMDbg"
# PROP BASE Intermediate_Dir "WCEARMDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCEARMDbg"
# PROP Intermediate_Dir "WCEARMDbg"
# PROP Target_Dir ""
CPP=clarm.exe
# ADD BASE CPP /nologo /MC /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "ARM" /D "_ARM_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /MC /W3 /Zi /Od /I ".." /I "..\..\zlib" /D "DEBUG" /D "ARM" /D "_ARM_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "ARM" /d "_ARM_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "ARM" /d "_ARM_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 coredll.lib commctrl.lib /nologo /dll /debug /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 coredll.lib commctrl.lib /nologo /dll /debug /machine:ARM /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WMIPSFPRel"
# PROP BASE Intermediate_Dir "WMIPSFPRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WMIPSFPRel"
# PROP Intermediate_Dir "WMIPSFPRel"
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /QMFWCE /MC /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /QMFWCE /MC /W3 /O2 /I ".." /I "..\..\zlib" /D "NDEBUG" /D "MIPS" /D "_MIPS_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WMIPSFPDbg"
# PROP BASE Intermediate_Dir "WMIPSFPDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WMIPSFPDbg"
# PROP Intermediate_Dir "WMIPSFPDbg"
# PROP Target_Dir ""
CPP=clmips.exe
# ADD BASE CPP /nologo /QMFWCE /MC /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /QMFWCE /MC /W3 /Zi /Od /I ".." /I "..\..\zlib" /D "DEBUG" /D "MIPS" /D "_MIPS_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "MIPS" /d "_MIPS_" /d UNDER_CE=$(CEVersion) /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:MIPS /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCEX86Rel"
# PROP BASE Intermediate_Dir "WCEX86Rel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCEX86Rel"
# PROP Intermediate_Dir "WCEX86Rel"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D "x86" /D "_i386_" /D "_x86_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "NDEBUG" /D "i_386_" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /Gs8192 /GF /c
# ADD CPP /nologo /ML /W3 /O2 /I ".." /I "..\..\zlib" /D "x86" /D "_i386_" /D "_x86_" /D "NDEBUG" /D "i_386_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /Gs8192 /GF /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "x86" /d "_i386_" /d "_x86_" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "x86" /d "_i386_" /d "_x86_" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /base:"0x00010000" /dll /machine:IX86 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /base:"0x00010000" /dll /machine:IX86 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCEX86Dbg"
# PROP BASE Intermediate_Dir "WCEX86Dbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCEX86Dbg"
# PROP Intermediate_Dir "WCEX86Dbg"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MLd /W3 /Zi /Od /D "x86" /D "_i386_" /D "_x86_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "DEBUG" /D "i_386_" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /Gs8192 /GF /c
# ADD CPP /nologo /MLd /W3 /Zi /Od /I ".." /I "..\..\zlib" /D "x86" /D "_i386_" /D "_x86_" /D "DEBUG" /D "i_386_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /Gs8192 /GF /c
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "x86" /d "_i386_" /d "_x86_" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "x86" /d "_i386_" /d "_x86_" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /base:"0x00010000" /dll /debug /machine:IX86 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /base:"0x00010000" /dll /debug /machine:IX86 /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x86emRel"
# PROP BASE Intermediate_Dir "x86emRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x86emRel"
# PROP Intermediate_Dir "x86emRel"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /ML /W3 /O2 /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "NDEBUG" /D "x86" /D "i486" /D "_x86_" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /ML /W3 /O2 /I ".." /I "..\..\zlib" /D "_UNICODE" /D "WIN32" /D "STRICT" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "NDEBUG" /D "x86" /D "i486" /D "_x86_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "NDEBUG"
# ADD RSC /l 0x411 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /stack:0x10000,0x1000 /dll /machine:I386 /nodefaultlib:"$(CENoDefaultLib)" /windowsce:emulation
# ADD LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /stack:0x10000,0x1000 /dll /machine:I386 /nodefaultlib:"$(CENoDefaultLib)" /windowsce:emulation

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x86emDbg"
# PROP BASE Intermediate_Dir "x86emDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x86emDbg"
# PROP Intermediate_Dir "x86emDbg"
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MLd /W3 /Gm /Zi /Od /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "STRICT" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /MLd /W3 /Gm /Zi /Od /I ".." /I "..\..\zlib" /D "_UNICODE" /D "WIN32" /D "STRICT" /D "_WIN32_WCE_EMULATION" /D "INTERNATIONAL" /D "USA" /D "INTLMSG_CODEPAGE" /D "_DEBUG" /D "x86" /D "i486" /D "_x86_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
# ADD RSC /l 0x411 /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "_UNICODE" /d "WIN32" /d "STRICT" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d "_WIN32_WCE_EMULATION" /d "INTERNATIONAL" /d "USA" /d "INTLMSG_CODEPAGE" /d "_DEBUG" /d "x86" /d "i486" /d "_x86_"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /stack:0x10000,0x1000 /dll /debug /machine:I386 /nodefaultlib:"$(CENoDefaultLib)" /windowsce:emulation
# ADD LINK32 $(CEx86Corelibc) commctrl.lib coredll.lib /nologo /stack:0x10000,0x1000 /dll /debug /machine:I386 /nodefaultlib:"$(CENoDefaultLib)" /windowsce:emulation

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WCEPPCRel"
# PROP BASE Intermediate_Dir "WCEPPCRel"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WCEPPCRel"
# PROP Intermediate_Dir "WCEPPCRel"
# PROP Target_Dir ""
CPP=clppc.exe
# ADD BASE CPP /nologo /M$(CECrt) /W3 /O2 /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "NDEBUG" /D "PPC" /D "_PPC_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /M$(CECrt) /W3 /O2 /I ".." /I "..\..\zlib" /D "NDEBUG" /D "PPC" /D "_PPC_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "ppc" /d "_ppc_" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "NDEBUG"
# ADD RSC /l 0x411 /r /d "ppc" /d "_ppc_" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "NDEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /machine:PPC /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /machine:PPC /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WCEPPCDbg"
# PROP BASE Intermediate_Dir "WCEPPCDbg"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WCEPPCDbg"
# PROP Intermediate_Dir "WCEPPCDbg"
# PROP Target_Dir ""
CPP=clppc.exe
# ADD BASE CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D "DEBUG" /D "PPC" /D "_PPC_" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "_USRDLL" /D "LPNGCE_EXPORTS" /YX /c
# ADD CPP /nologo /M$(CECrtDebug) /W3 /Zi /Od /I ".." /I "..\..\zlib" /D "DEBUG" /D "PPC" /D "_PPC_" /D _WIN32_WCE=$(CEVersion) /D "$(CEConfigName)" /D UNDER_CE=$(CEVersion) /D "UNICODE" /D "_MBCS" /D "PNG_BUILD_DLL" /c
# SUBTRACT CPP /YX
RSC=rc.exe
# ADD BASE RSC /l 0x411 /r /d "ppc" /d "_ppc_" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "DEBUG"
# ADD RSC /l 0x411 /r /d "ppc" /d "_ppc_" /d _WIN32_WCE=$(CEVersion) /d "$(CEConfigName)" /d UNDER_CE=$(CEVersion) /d "UNICODE" /d "DEBUG"
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:PPC /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT BASE LINK32 /pdb:none /nodefaultlib
# ADD LINK32 commctrl.lib coredll.lib /nologo /dll /debug /machine:PPC /nodefaultlib:"$(CENoDefaultLib)" /subsystem:$(CESubsystem) /STACK:65536,4096
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "lpngce - Win32 (WCE MIPS) Release"
# Name "lpngce - Win32 (WCE MIPS) Debug"
# Name "lpngce - Win32 (WCE SH4) Release"
# Name "lpngce - Win32 (WCE SH4) Debug"
# Name "lpngce - Win32 (WCE SH3) Release"
# Name "lpngce - Win32 (WCE SH3) Debug"
# Name "lpngce - Win32 (WCE ARM) Release"
# Name "lpngce - Win32 (WCE ARM) Debug"
# Name "lpngce - Win32 (WCE MIPSFP) Release"
# Name "lpngce - Win32 (WCE MIPSFP) Debug"
# Name "lpngce - Win32 (WCE x86) Release"
# Name "lpngce - Win32 (WCE x86) Debug"
# Name "lpngce - Win32 (WCE x86em) Release"
# Name "lpngce - Win32 (WCE x86em) Debug"
# Name "lpngce - Win32 (WCE PPC) Release"
# Name "lpngce - Win32 (WCE PPC) Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\png.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNG_C=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNG_C=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNG_C=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNG_C=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\png32ce.def
# End Source File
# Begin Source File

SOURCE=..\pngerror.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGER=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGER=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGER=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGER=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pnggccrd.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGGC=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGGC=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGGC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngget.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGGE=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGGE=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGGE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGGE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngmem.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGME=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGME=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGME=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGME=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngpread.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGPR=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGPR=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGPR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGPR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngread.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGRE=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGRE=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGRE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngrio.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGRI=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGRI=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGRI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngrtran.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGRT=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGRT=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGRT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngrutil.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGRU=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGRU=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGRU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGRU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngset.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGSE=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGSE=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGSE=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGSE=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngtrans.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGTR=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGTR=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGTR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGTR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngvcrd.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGVC=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGVC=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGVC=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGVC=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngwio.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGWI=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGWI=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGWI=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWI=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngwrite.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGWR=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGWR=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGWR=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWR=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngwtran.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGWT=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGWT=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGWT=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWT=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\pngwutil.c

!IF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Release"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPS) Debug"

DEP_CPP_PNGWU=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Release"

DEP_CPP_PNGWU=\
	"..\..\zlib\zconf.h"\
	"..\..\zlib\zlib.h"\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH4) Debug"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Release"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE SH3) Debug"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Release"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE ARM) Debug"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Release"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE MIPSFP) Debug"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Release"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86) Debug"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Release"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE x86em) Debug"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Release"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ELSEIF  "$(CFG)" == "lpngce - Win32 (WCE PPC) Debug"

DEP_CPP_PNGWU=\
	"..\png.h"\
	"..\pngasmrd.h"\
	"..\pngconf.h"\
	
NODEP_CPP_PNGWU=\
	"..\alloc.h"\
	"..\..\zlib\zlib.h"\
	

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\png.h
# End Source File
# Begin Source File

SOURCE=..\pngasmrd.h
# End Source File
# Begin Source File

SOURCE=..\pngconf.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
