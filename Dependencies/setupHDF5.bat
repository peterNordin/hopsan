@ECHO OFF
REM $Id$

REM Bat script building libHDF5 dependency automatically
REM Author: Peter Nordin peter.nordin@liu.se
REM Date:   2015-07-06

set filename=hdf5-1.8.15-patch1.zip
set dirname=hdf5-1.8.15-patch1

REM Automatic code starts here

REM Unpack
echo.
echo Clearing old directory (if it exists)
if exist %dirname% rd /s/q %dirname%
echo Unpacking %filename%
..\ThirdParty\7z\7z.exe x %filename% -y > nul

REM Build
echo.
echo ======================
echo Building 64-bit version of FMILibrary
echo ======================
set OLDPATH=%PATH%
call setHopsanBuildPaths.bat
REM We don want msys in the path so we have to build it manually
set PATH=%mingw_path%;%cmake_path%;%OLDPATH%
cd %dirname%
mkdir build
cd build
cmake -Wno-dev -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=ON -DHDF5_BUILD_FORTRAN=OFF -DCMAKE_INSTALL_PREFIX="../install" ../
mingw32-make.exe -j4
mingw32-make.exe install
echo.
echo Done
pause