:: Hopsan model validation script
:: This script calls hopsancli to validate models for all hvc files found.

@echo off
SETLOCAL EnableDelayedExpansion

set failed=0
set okPause=1
set hopsan_install_dir=%~dp0

if not "%~1"=="" (
  set "hopsan_install_dir=%~1"
)

if "%~2"=="nopause" (
  set okPause=0
)

for /F "delims==" %%x in ('dir /B /S *.hvc') do (
  pushd %hopsan_install_dir%\bin
  if not exist hopsancli_d.exe (
    if not exist hopsancli.exe (
      echo "hopsancli.exe not found in %hopsan_install_dir%\bin"
      set failed=1
    )
  )
  if exist hopsancli_d.exe (
    echo "Evaluating with hopsancli_d: %%x"
    hopsancli_d.exe -t "%%x"
    if ERRORLEVEL 1 set failed=1 
  )
  if exist hopsancli.exe (
    echo "Evaluating with hopsancli: %%x"
    hopsancli.exe -t "%%x"
    if ERRORLEVEL 1 set failed=1 
  )
  popd
)
if %failed% EQU 1 (
  echo ERROR: There was at least one failure!
  if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
    pause
  )
  exit /B 1
)
if %okPause% EQU 1 (
  if "%HOPSAN_BUILD_SCRIPT_NOPAUSE%" == "" (
    pause
  )
)
exit /B 0
