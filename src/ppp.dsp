# Microsoft Developer Studio Project File - Name="ppp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ppp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ppp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ppp.mak" CFG="ppp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ppp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ppp - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ppp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "ppp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ppp - Win32 Release"
# Name "ppp - Win32 Debug"
# Begin Group "c source files"

# PROP Default_Filter "c"
# Begin Source File

SOURCE=.\ahdlc.c
# End Source File
# Begin Source File

SOURCE=.\autobaud.c
# End Source File
# Begin Source File

SOURCE=.\conio.c
# End Source File
# Begin Source File

SOURCE=.\console.c
# End Source File
# Begin Source File

SOURCE=.\globals.c
# End Source File
# Begin Source File

SOURCE=.\imm.c
# End Source File
# Begin Source File

SOURCE=.\ipcp.c
# End Source File
# Begin Source File

SOURCE=.\ircmd.c
# End Source File
# Begin Source File

SOURCE=.\lcp.c
# End Source File
# Begin Source File

SOURCE=.\memory.c
# End Source File
# Begin Source File

SOURCE=.\mip.c
# End Source File
# Begin Source File

SOURCE=.\modem.c
# End Source File
# Begin Source File

SOURCE=.\msg.c
# End Source File
# Begin Source File

SOURCE=.\mtcp.c
# End Source File
# Begin Source File

SOURCE=.\mudp.c
# End Source File
# Begin Source File

SOURCE=.\pap.c
# End Source File
# Begin Source File

SOURCE=.\ppp.c
# End Source File
# Begin Source File

SOURCE=.\resolve.c
# End Source File
# Begin Source File

SOURCE=.\sreg.c
# End Source File
# Begin Source File

SOURCE=.\time.c
# End Source File
# Begin Source File

SOURCE=.\udpcmd.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# Begin Source File

SOURCE=.\wserial.c
# End Source File
# End Group
# Begin Group "include files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\ahdlc.h
# End Source File
# Begin Source File

SOURCE=.\autobaud.h
# End Source File
# Begin Source File

SOURCE=.\conio.h
# End Source File
# Begin Source File

SOURCE=.\console.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\imm.h
# End Source File
# Begin Source File

SOURCE=.\ipcp.h
# End Source File
# Begin Source File

SOURCE=.\ircmd.h
# End Source File
# Begin Source File

SOURCE=.\lcp.h
# End Source File
# Begin Source File

SOURCE=.\memory.h
# End Source File
# Begin Source File

SOURCE=.\micmp.h
# End Source File
# Begin Source File

SOURCE=.\mip.h
# End Source File
# Begin Source File

SOURCE=.\modem.h
# End Source File
# Begin Source File

SOURCE=.\msg.h
# End Source File
# Begin Source File

SOURCE=.\mtcp.h
# End Source File
# Begin Source File

SOURCE=.\mTypes.h
# End Source File
# Begin Source File

SOURCE=.\mudp.h
# End Source File
# Begin Source File

SOURCE=.\pap.h
# End Source File
# Begin Source File

SOURCE=.\ports.h
# End Source File
# Begin Source File

SOURCE=.\ppp.h
# End Source File
# Begin Source File

SOURCE=.\pppconfig.h
# End Source File
# Begin Source File

SOURCE=.\resolve.h
# End Source File
# Begin Source File

SOURCE=.\sreg.h
# End Source File
# Begin Source File

SOURCE=.\time.h
# End Source File
# Begin Source File

SOURCE=.\udpcmd.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\wserial.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\dev_history.txt
# End Source File
# Begin Source File

SOURCE=.\documentation.txt
# End Source File
# Begin Source File

SOURCE=.\makefile
# End Source File
# End Target
# End Project
