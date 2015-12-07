@echo off

if not exist build mkdir build
cd build

if not exist vs mkdir vs
if not exist clang mkdir clang
if not exist intel mkdir intel
if not exist gcc mkdir gcc



REM MSVC
echo.
echo Generating MSVC files
cd vs
cmake -G  "Visual Studio 14 2015 Win64" ../../
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
cmake -G  "Visual Studio 14 2015 Win64" -T "Intel C++ Compiler 16.0"  ../../
cd..


REM GCC
echo.
echo Generating GCC files
cd gcc
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ../../
cd..

REM leave build folder
cd ..