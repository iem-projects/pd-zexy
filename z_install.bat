@echo off
echo installing zexy on your system
mkdir ..\externs
copy zexy.dll ..\externs
echo copying help files
mkdir ..\doc\5.reference\zexy
copy examples\* ..\doc\5.reference\zexy
echo done
echo on
