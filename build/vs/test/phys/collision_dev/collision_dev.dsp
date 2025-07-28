# Microsoft Developer Studio Project File - Name="collision_dev" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=collision_dev - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "collision_dev.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "collision_dev.mak" CFG="collision_dev - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "collision_dev - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "collision_dev - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "collision_dev - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../../../src" /I "../../../include" /I "../../../win32-support/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "collision_dev - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../../src" /I "../../../include" /I "../../../win32-support/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "collision_dev - Win32 Release"
# Name "collision_dev - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\bucket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\close_counter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\coll_hash.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\coll_obj.cpp

!IF  "$(CFG)" == "collision_dev - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "collision_dev - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\glDriver.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\glDriver2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\hash_test.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\pairs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\physEngine.cpp

!IF  "$(CFG)" == "collision_dev - Win32 Release"

!ELSEIF  "$(CFG)" == "collision_dev - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\physEngine2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\physObject.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\treap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\treap_test.cpp
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\bucket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\close_counter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\coll_hash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\coll_obj.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\collisionQueue.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\exceptions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\pairs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\physConstants.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\physEngine.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\physEngine2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\physObject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\src\phys\collision_dev\treap.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE="..\..\..\win32-support\lib\glut32.lib"
# End Source File
# End Target
# End Project
