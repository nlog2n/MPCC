# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=fig - Win32 Release
!MESSAGE No configuration specified.  Defaulting to fig - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "fig - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "FIG.MAK" CFG="fig - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fig - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "fig - Win32 Release"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe
# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\FIG.exe"

CLEAN : 
	-@erase "$(INTDIR)\fig.obj"
	-@erase "$(INTDIR)\FIG.pch"
	-@erase "$(INTDIR)\fig.res"
	-@erase "$(INTDIR)\figDlg.obj"
	-@erase "$(INTDIR)\FigDlg2.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(OUTDIR)\FIG.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/FIG.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/FIG.pdb" /machine:I386 /out:"$(OUTDIR)/FIG.exe" 
LINK32_OBJS= \
	"$(INTDIR)\fig.obj" \
	"$(INTDIR)\fig.res" \
	"$(INTDIR)\figDlg.obj" \
	"$(INTDIR)\FigDlg2.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\FIG.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)/FIG.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

RSC_PROJ=/l 0x409 /fo"$(INTDIR)/fig.res" /d "NDEBUG" 
MTL_PROJ=/nologo /D "NDEBUG" /win32 
################################################################################
# Begin Target

# Name "fig - Win32 Release"
################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
################################################################################
# Begin Source File

SOURCE=.\fig.cpp
DEP_CPP_FIG_C=\
	".\fig.h"\
	".\figDlg.h"\
	".\paradef.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\fig.obj" : $(SOURCE) $(DEP_CPP_FIG_C) "$(INTDIR)"\
 "$(INTDIR)\FIG.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\figDlg.cpp
DEP_CPP_FIGDL=\
	".\fig.h"\
	".\figDlg.h"\
	".\FigDlg2.h"\
	".\paradef.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\figDlg.obj" : $(SOURCE) $(DEP_CPP_FIGDL) "$(INTDIR)"\
 "$(INTDIR)\FIG.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	
# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp"$(INTDIR)/FIG.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\FIG.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\fig.rc
DEP_RSC_FIG_R=\
	".\res\fig.rc2"\
	

"$(INTDIR)\fig.res" : $(SOURCE) $(DEP_RSC_FIG_R) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\FigDlg2.cpp
DEP_CPP_FIGDLG=\
	".\fig.h"\
	".\FigDlg2.h"\
	".\paradef.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\FigDlg2.obj" : $(SOURCE) $(DEP_CPP_FIGDLG) "$(INTDIR)"\
 "$(INTDIR)\FIG.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\paradef.h
# End Source File
# End Target
# End Project
################################################################################
