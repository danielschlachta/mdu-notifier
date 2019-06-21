@echo off

c:

cd c:\Qt\Qt5.5.1\5.5\mingw492_32\bin

call qtenv2.bat

d:
cd "\UserFiles\Daniel\Qt Projects\mdu-notifier\installer-xp"

rmdir /s /q build
mkdir build
cd build

qmake ..\..

mingw32-make -f Makefile.Release
cd ..

rmdir /s /q deploy
mkdir deploy

copy build\release\mdu-notifier.exe deploy

windeployqt.exe --compiler-runtime --no-translations --release deploy

cd ..\wixgen
c:\cygwin\bin\bash --login -c "cd '/cygdrive/d/UserFiles/Daniel/Qt Projects/mdu-notifier/wixgen'; ./wixgen.sh ../installer-xp deploy | tee ../installer-xp/mdu-notifier-xp.wxs"

cd ..\installer-xp

PATH=%PATH%;"C:\Program Files\WiX Toolset v3.11\bin"
candle mdu-notifier-xp.wxs
light -ext WixUIExtension -ext WixUtilExtension -cultures:en-us -out mdu-notifier-xp.msi mdu-notifier-xp.wixobj

pause
