# Microsoft Developer Studio Project File - Name="my_c" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=my_c - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "my_c.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "my_c.mak" CFG="my_c - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "my_c - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "my_c - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "my_c - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "..\custom_scripting_language" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "my_c - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /ZI /Od /I "..\custom_scripting_language" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /debug /machine:I386

!ENDIF 

# Begin Target

# Name "my_c - Win32 Release"
# Name "my_c - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\intcode.cpp
# End Source File
# Begin Source File

SOURCE=.\lex.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\parse.cpp
# End Source File
# Begin Source File

SOURCE=.\symbolTable.cpp
# End Source File
# Begin Source File

SOURCE=.\syntaxTree.cpp
# End Source File
# Begin Source File

SOURCE=.\vm.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\intcode.h
# End Source File
# Begin Source File

SOURCE=.\lex.h
# End Source File
# Begin Source File

SOURCE=.\lexSymbol.h
# End Source File
# Begin Source File

SOURCE=.\parse.h
# End Source File
# Begin Source File

SOURCE=.\symbolTable.h
# End Source File
# Begin Source File

SOURCE=.\syntaxTree.h
# End Source File
# Begin Source File

SOURCE=.\vm.h
# End Source File
# End Group
# Begin Group "Lex / Yacc Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\my_c.l

!IF  "$(CFG)" == "my_c - Win32 Release"

# Begin Custom Build - Generating lexer... lex.cpp
InputPath=.\my_c.l

"lex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex.bat

# End Custom Build

!ELSEIF  "$(CFG)" == "my_c - Win32 Debug"

# Begin Custom Build - Generating lexer... lex.cpp
InputPath=.\my_c.l

"lex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex.bat

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\my_c.y

!IF  "$(CFG)" == "my_c - Win32 Release"

# Begin Custom Build - Generating parser... parse.cpp, lexSymbol.h
InputPath=.\my_c.y

"parse.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	bison.bat

# End Custom Build

!ELSEIF  "$(CFG)" == "my_c - Win32 Debug"

# Begin Custom Build - Generating parser... parse.cpp, lexSymbol.h
InputPath=.\my_c.y

"parse.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	bison.bat

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Batch Files"

# PROP Default_Filter ".bat"
# Begin Source File

SOURCE=.\bison.bat
# End Source File
# Begin Source File

SOURCE=.\flex.bat
# End Source File
# End Group
# Begin Group "Script Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Scripts\addition_operator.myc
# End Source File
# Begin Source File

SOURCE=.\Scripts\comments.myc
# End Source File
# Begin Source File

SOURCE=.\Scripts\hello_world.myc
# End Source File
# Begin Source File

SOURCE=.\Scripts\if_else.myc
# End Source File
# Begin Source File

SOURCE=.\Scripts\input.myc
# End Source File
# Begin Source File

SOURCE=.\Scripts\numeric_equality.myc
# End Source File
# Begin Source File

SOURCE=.\Scripts\string_equality.myc
# End Source File
# End Group
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Target
# End Project
