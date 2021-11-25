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

nuget restore %~dp0
msbuild %~dp0..\WapProjTemplate1\WapProjTemplate1.wapproj /p:Platform=%arch% /p:Configuration=%config% /p:RestorePackagesConfig=true /restore  /bl
copy %~dp0..\WapProjTemplate1\bin\%arch%\%config%\resources.pri %~dp0%config%
exit /b 0

endlocal

:usage
echo   Usage:   %script% arch config