@echo off
echo installing zexy on your system
mkdir ..\extra
copy zexy.dll ..\extra

echo copying help files
mkdir ..\extra\help-zexy
copy examples\* ..\extra\help-zexy

echo copying abstractions
copy abs\*.pd ..\extra

echo done
