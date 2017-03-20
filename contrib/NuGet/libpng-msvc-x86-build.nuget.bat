REM @echo off

Echo LIB Windows Build NuGet

REM # XEON win32 Build Vars #
set _SCRIPT_DRIVE=%~d0
set _SCRIPT_FOLDER=%~dp0
set SRC=%_SCRIPT_FOLDER%\..\..\
set INITDIR=%_SCRIPT_FOLDER%
set BUILDTREE=%SRC%\build-win\
SET tbs_arch=x86
SET vcvar_arg=x86
SET cmake_platform="Visual Studio 15 2017"

REM # VC Vars #
SET VCVAR="%programfiles(x86)%\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat"
if exist %VCVAR% call %VCVAR% %vcvar_arg%
SET VCVAR="%programfiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
if exist %VCVAR% call %VCVAR% %vcvar_arg%

REM # Clean Build Tree #
rd /s /q %BUILDTREE%
mkdir %BUILDTREE%
cd %BUILDTREE%

:nuget_Dep
REM # packages from nuget #
mkdir %BUILDTREE%\deps
cd %BUILDTREE%\deps
SET VER=1.2.11.8899
set ZLIBDIR=%BUILDTREE%\deps\zlib-msvc-%tbs_arch%.%VER%\build\native
nuget install zlib-msvc-%tbs_arch% -Version %VER%

:copy_files
set BINDIR=%SRC%\build-nuget\
rd /s /q %BINDIR%
mkdir %BINDIR%

:static_LIB
REM # LIB STATIC #
ECHO %cmake_platform% STATIC

rd /s /q %BUILDTREE%\libpng
mkdir %BUILDTREE%\libpng
cd %BUILDTREE%\libpng
cmake -G %cmake_platform% ^
-DBUILD_SHARED_LIBS:BOOL=OFF ^
-DCMAKE_CXX_FLAGS_RELEASE="/MD" ^
-DCMAKE_CXX_FLAGS_DEBUG="/MDd" ^
-DCMAKE_C_FLAGS_RELEASE="/MD" ^
-DCMAKE_C_FLAGS_DEBUG="/MDd" ^
-DCMAKE_INSTALL_PREFIX=%BINDIR% ^
-DZLIB_LIBRARY=%ZLIBDIR%\lib_release\zlibstatic.lib ^
-DZLIB_INCLUDE_DIR=%ZLIBDIR%\include ^
-DCMAKE_BUILD_TYPE="Release" %SRC%
cmake --build . --config Release --target install

xcopy /Q /C /Y %ZLIBDIR%\bin_release\*.dll %BINDIR%bin

move %BINDIR%lib %BINDIR%lib_release
move %BINDIR%bin %BINDIR%bin_release

REM # Clean Build Tree #
rd /s /q %BUILDTREE%\libpng

REM # DEBUG #
rd /s /q %BUILDTREE%\libpng
mkdir %BUILDTREE%\libpng
cd %BUILDTREE%\libpng
cmake -G %cmake_platform% ^
-DBUILD_SHARED_LIBS:BOOL=OFF ^
-DCMAKE_CXX_FLAGS_RELEASE="/MD" ^
-DCMAKE_CXX_FLAGS_DEBUG="/MDd" ^
-DCMAKE_C_FLAGS_RELEASE="/MD" ^
-DCMAKE_C_FLAGS_DEBUG="/MDd" ^
-DCMAKE_INSTALL_PREFIX=%BINDIR% ^
-DZLIB_LIBRARY=%ZLIBDIR%\lib_debug\zlibstaticd.lib ^
-DZLIB_INCLUDE_DIR=%ZLIBDIR%\include ^
-DCMAKE_BUILD_TYPE="DEBUG" %SRC%
cmake --build . --config DEBUG --target install

xcopy /Q /C /Y %ZLIBDIR%\bin_debug\*.dll %BINDIR%bin

move %BINDIR%lib %BINDIR%lib_debug
move %BINDIR%bin %BINDIR%bin_debug

:nuget_req
REM # make nuget packages from binaries #
copy %INITDIR%\libpng-msvc-%tbs_arch%.targets %BINDIR%\libpng-msvc-%tbs_arch%.targets
cd %BUILDTREE%
nuget pack %INITDIR%\libpng-msvc-%tbs_arch%.nuspec
cd %INITDIR%
REM --- exit ----
GOTO:eof
