@echo off
setlocal

echo ==============================================
echo        GenesisEngine Premake Runner
echo ==============================================

REM Find premake5.exe next to this batch file
set SCRIPT_DIR=%~dp0
set PREMAKE="%SCRIPT_DIR%premake5.exe"

if not exist %PREMAKE% (
    echo ERROR: premake5.exe not found in:
    echo   %SCRIPT_DIR%
    echo.
    pause
    exit /B 1
)

echo Found Premake at:
echo   %PREMAKE%
echo.

echo Generating Visual Studio 2022 solution...
%PREMAKE% vs2022

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Premake failed.
    pause
    exit /B %ERRORLEVEL%
)

echo.
echo Premake completed successfully!
pause