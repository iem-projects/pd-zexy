@echo off
REM ==============================================
REM adapt the following to your needs
REM ==============================================

REM where does PD reside ??
REM if you want spaces in the path, please use quotes ("")
set PDPATH=C:\programme\pd

REM which pd-version do we have ?
set PDVERSION=0.37

REM ==============================================
REM do not edit below this line !!!
REM ==============================================


echo installing zexy on your system
IF NOT EXIST %PDPATH%\bin\pd.exe goto location_error

set BINPATH=extra
set REFPATH=extra\help-zexy

if %PDVERSION% LSS 0.37 set REFPATH=doc\5.reference\zexy

echo Copying binary...
copy zexy.dll %PDPATH%\%BINPATH% > tempInstall.trash

echo copying help files
mkdir %PDPATH%\%REFPATH%
copy examples\* %PDPATH%\%REFPATH%

echo copying abstractions
copy abs\*.pd %PDPATH\%BINPATH%

echo done
goto end

:location_error
echo :
echo : i believe i am in the wrong directory
echo : i thought that the pd-executable would be %PDPATH%\bin\pd.exe
echo : obviously it is not !!!
echo : please edit this file and set the PDPATH-variable apropriatly
echo :
echo : stopping installation
echo :

:end
pause
