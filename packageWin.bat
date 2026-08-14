@echo off
if exist Sandbox rd /S /Q Sandbox
if exist Sandbox_Win64.zip del /S /Q Sandbox_Win64.zip
if exist build rd /S /Q build

mkdir Sandbox
mkdir Sandbox\assets
mkdir Sandbox\data
mkdir build
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build

copy "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" Sandbox\
copy "C:\msys64\ucrt64\bin\libwinpthread-1.dll" Sandbox\
copy "C:\msys64\ucrt64\bin\libstdc++-6.dll" Sandbox\

xcopy /s /e /i /y /q assets Sandbox\assets\
copy build\sandbox.exe Sandbox\

powershell Compress-Archive -Path Sandbox -DestinationPath Sandbox_Win64.zip -Force
rd /S /Q Sandbox