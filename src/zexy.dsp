# Microsoft Developer Studio Project File - Name="zexy" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ZEXY - WIN32 RELEASE
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "zexy.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "zexy.mak" CFG="ZEXY - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "zexy - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "obj\"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZEXY_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /I "..\..\src" /D "WIN32" /D "NT" /D "_WINDOWS" /D "ZEXY" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib wsock32.lib uuid.lib libc.lib oldnames.lib pd.lib /nologo /dll /machine:I386 /nodefaultlib /out:"..\zexy.dll" /libpath:"../../bin" /export:zexy_setup
# SUBTRACT LINK32 /pdb:none
# Begin Target

# Name "zexy - Win32 Release"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\z_average.c
# End Source File
# Begin Source File

SOURCE=.\z_connective.c
# End Source File
# Begin Source File

SOURCE=.\z_coordinates.c
# End Source File
# Begin Source File

SOURCE=.\z_datetime.c
# End Source File
# Begin Source File

SOURCE=.\z_dfreq.c
# End Source File
# Begin Source File

SOURCE=.\z_down.c
# End Source File
# Begin Source File

SOURCE=.\z_drip.c
# End Source File
# Begin Source File

SOURCE=.\z_index.c
# End Source File
# Begin Source File

SOURCE=.\z_limiter.c
# End Source File
# Begin Source File

SOURCE=.\z_makesymbol.c
# End Source File
# Begin Source File

SOURCE=.\z_matrix.c
# End Source File
# Begin Source File

SOURCE=.\z_msgfile.c
# End Source File
# Begin Source File

SOURCE=.\z_mtx.c
# End Source File
# Begin Source File

SOURCE=.\z_multiline.c
# End Source File
# Begin Source File

SOURCE=.\z_multiplex.c
# End Source File
# Begin Source File

SOURCE=.\z_noise.c
# End Source File
# Begin Source File

SOURCE=.\z_nop.c
# End Source File
# Begin Source File

SOURCE=.\z_pack.c
# End Source File
# Begin Source File

SOURCE=.\z_pdf.c
# End Source File
# Begin Source File

SOURCE=.\z_point.c
# End Source File
# Begin Source File

SOURCE=.\z_quantize.c
# End Source File
# Begin Source File

SOURCE=.\z_random.c
# End Source File
# Begin Source File

SOURCE=.\z_sfplay.c
# End Source File
# Begin Source File

SOURCE=.\z_sfrecord.c
# End Source File
# Begin Source File

SOURCE=.\z_sigaverage.c
# End Source File
# Begin Source File

SOURCE=.\z_sigbin.c
# End Source File
# Begin Source File

SOURCE=.\z_sigmatrix.c
# End Source File
# Begin Source File

SOURCE=.\z_sigpack.c
# End Source File
# Begin Source File

SOURCE=.\z_sigzero.c
# End Source File
# Begin Source File

SOURCE=.\z_sort.c
# End Source File
# Begin Source File

SOURCE=.\z_stat.c
# End Source File
# Begin Source File

SOURCE=.\z_strings.c
# End Source File
# Begin Source File

SOURCE=.\z_swap.c
# End Source File
# Begin Source File

SOURCE=.\z_tabread4.c
# End Source File
# Begin Source File

SOURCE=.\z_testfun.c
# End Source File
# Begin Source File

SOURCE=.\z_zdelay.c
# End Source File
# Begin Source File

SOURCE=.\zexy.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\m_pd.h
# End Source File
# Begin Source File

SOURCE=.\Zexy.h
# End Source File
# End Group
# End Target
# End Project
