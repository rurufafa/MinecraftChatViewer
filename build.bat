@echo off
setlocal

rmdir /s /q build
rmdir /s /q deploy
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="D:/Qt/6.9.1/mingw_64/lib/cmake"
cmake --build build

mkdir deploy
copy build\MCV.exe deploy\
D:\Qt\6.9.1\mingw_64\bin\windeployqt.exe --dir deploy deploy\MCV.exe

copy D:\Qt\Tools\mingw1310_64\bin\libstdc++-6.dll deploy\
copy D:\Qt\Tools\mingw1310_64\bin\libgcc_s_seh-1.dll deploy\
copy D:\Qt\Tools\mingw1310_64\bin\libwinpthread-1.dll deploy\

pause
