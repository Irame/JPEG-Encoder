@echo off

call ShiaLaBeouf.bat

REM search msbuild.exe
REM http://stackoverflow.com/questions/328017/path-to-msbuild
set bb.build.msbuild.exe=
for /D %%D in (%SYSTEMROOT%\Microsoft.NET\Framework\v4*) do set msbuild.exe=%%D\MSBuild.exe

if not defined msbuild.exe echo error: can't find MSBuild.exe & goto :eof
if not exist "%msbuild.exe%" echo error: %msbuild.exe%: not found & goto :eof

set projectdir=JPEG-Encoder-SIMD
set projectfile=%projectdir%\JPEG-Encoder-SIMD.vcxproj
set msbuildCMD=/p:Configuration=Release /t:rebuild

echo.
echo --=== Building with MSVC 14.0 ===--

%msbuild.exe% build\vs14\%projectfile% %msbuildCMD%
set msvc14=%errorlevel%

echo --=== End building with MSVC 14.0 ===--
echo.

echo.
echo --=== Building with MSVC 15.0 ===--

%msbuild.exe% build\vs15\%projectfile% %msbuildCMD%
set msvc15=%errorlevel%

echo --=== End building with MSVC 15.0 ===--
echo.

:clang
echo.
echo --=== Begin building with Clang ===--

%msbuild.exe% build\clang\%projectfile% %msbuildCMD%
set clang=%errorlevel%

echo --=== End building with Clang ===--
echo.

:intel
echo.
echo --=== Begin building with Intel Compiler ===--
%msbuild.exe% build\intel\%projectfile% %msbuildCMD%
set intel=%errorlevel%


echo --=== End building with Intel Compiler ===--
echo.

:gcc
echo.
echo --=== Begin building with GCC ===--

mingw32-make --directory=build\gcc\ --always-make 
set gcc=%errorlevel%

echo --=== End building with GCC ===--
echo.

if %msvc14% == 0 (
	echo Running MSVC 14.0
	build\vs14\%projectdir%\Release\JPEG-Encoder-SIMD.exe %*
) else (
	echo Skipping MSVC 14.0
)
if %msvc15% == 0 (
	echo Running MSVC 15.0
	build\vs15\%projectdir%\Release\JPEG-Encoder-SIMD.exe %*
) else (
	echo Skipping MSVC 15.0
)
if %clang% == 0 (
	echo Running Clang
	build\clang\%projectdir%\Release\JPEG-Encoder-SIMD.exe %*
) else (
	echo Skipping Clang
)
if %intel% == 0 (
	echo Running Intel
	build\intel\%projectdir%\Release\JPEG-Encoder-SIMD.exe %*
) else (
	echo Skipping Intel
)
if %gcc% == 0 (
	echo Running GCC
	build\gcc\%projectdir%\JPEG-Encoder-SIMD.exe %*
) else (
	echo Skipping GCC
)

:eof
