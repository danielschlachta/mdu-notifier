@echo off

d:

cd \Qt\5.13.0\mingw73_64\bin

call qtenv2.bat
Z:
cd "\Qt Projects\mdu-notifier\installer"

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
c:\cygwin64\bin\bash --login -c "cd '/cygdrive/z/Qt Projects/mdu-notifier/wixgen'; ./wixgen.sh ../installer deploy | tee ../installer/mdu-notifier.wxs"

cd ..\installer

PATH=%PATH%;"C:\Program Files (x86)\WiX Toolset v3.11\bin"
candle mdu-notifier.wxs
light -ext WixUIExtension -ext WixUtilExtension -cultures:en-us -out mdu-notifier.msi mdu-notifier.wixobj

pause
