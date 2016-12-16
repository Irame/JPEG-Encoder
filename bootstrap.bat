@echo off

if not exist build mkdir build
cd build

if not exist vs14 mkdir vs14
if not exist vs15 mkdir vs15
if not exist clang mkdir clang
if not exist intel mkdir intel
if not exist gcc mkdir gcc



REM MSVC 14.0
echo.
echo Generating MSVC 14.0 files
cd vs14
cmake -G  "Visual Studio 14 2015 Win64" ../../
cd ..


REM MSVC 15.0
echo.
echo Generating MSVC 15.0 files
cd vs15
cmake -G  "Visual Studio 15 2017 Win64" ../../
cd ..

REM CLANG
echo.
echo Generating Clang files
cd clang
cmake -G  "Visual Studio 14 2015 Win64" -T "LLVM-vs2014"  ../../
cd..

REM INTEL
echo.
echo Generating Intel files
cd intel
cmake -G  "Visual Studio 14 2015 Win64" -T "Intel C++ Compiler 17.0"  ../../
cd..


REM GCC
echo.
echo Generating GCC files
cd gcc
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ../../
cd..

REM leave build folder
cd ..