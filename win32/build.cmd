@echo off
setlocal ENABLEDELAYEDEXPANSION

set script=%0
set arch=%1
set config=%2

if "%arch%" == "" ( 
  call :usage
  exit /b 1
  )

if "%config%" == "" (
  call :usage
  exit /b 1
)

where.exe msbuild.exe >NUL 2>NUL
if ERRORLEVEL 1 (
  echo Must run this in a VS dev cmd
  exit /b 2
)

msbuild %~dp0..\WapProjectTemplate1\WapProjectTemplate1.wapproj /p:Platform=%arch;Configuration=%config% /restore /p:RestorePackagesConfig=true
echo copy WapProjectTemplate1\bin\%arch%\%config%\resources.pri %~dp0%config%
exit /b 0

endlocal

:usage
echo   Usage:   %script% arch config