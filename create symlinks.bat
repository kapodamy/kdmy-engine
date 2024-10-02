echo off
cls

chcp 65001 >nul

NET SESSION >nul 2>&1
IF NOT %ERRORLEVEL% EQU 0 (
    echo ¡¡¡ This script requires administrator privileges !!!
    pause
    exit
)

echo WARNING
echo this script will delete the following folders or broken symlinks:
echo.
echo.
echo src-dreamcast\output\cd_root\assets
echo src-javascript\assets
echo src-javascript\expansions
echo src-desktop\kdy-e\bin\x64\Debug\assets
echo src-desktop\kdy-e\bin\x64\Debug\expansions
echo fontatlas-javascript\src
echo fontatlas-javascript\include
echo.
echo.

set /P stub=Press ENTER to confirm

echo.
echo.

setlocal enableextensions
pushd  "%~dp0"

rmdir /Q src-dreamcast\output\cd_root\assets
rmdir /Q src-javascript\assets
rmdir /Q src-javascript\expansions
rmdir /Q src-desktop\kdy-e\bin\x64\Debug\assets
rmdir /Q src-desktop\kdy-e\bin\x64\Debug\expansions
rmdir /Q fontatlas-javascript\src
rmdir /Q fontatlas-javascript\include

echo.
echo.

del /Q src-dreamcast\output\cd_root\assets
del /Q src-javascript\assets
del /Q src-javascript\expansions
del /Q src-desktop\kdy-e\bin\x64\Debug\assets
del /Q src-desktop\kdy-e\bin\x64\Debug\expansions
del /Q fontatlas-javascript\src
del /Q fontatlas-javascript\include

echo.
echo.

mklink /D src-dreamcast\output\cd_root\assets ..\..\..\assets_dc
mklink /D src-javascript\assets ..\assets
mklink /D src-javascript\expansions ..\expansions
mklink /D src-desktop\kdy-e\bin\x64\Debug\assets ..\..\..\..\..\assets
mklink /D src-desktop\kdy-e\bin\x64\Debug\expansions ..\..\..\..\..\expansions
mklink /D fontatlas-javascript\src ..\src-dreamcast\src
mklink /D fontatlas-javascript\include ..\src-dreamcast\include

popd

echo.
pause
