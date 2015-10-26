@echo off
echo "Deleting old boost directory"
rmdir /S /Q boost
mkdir boost
cd boost
..\bin\win\wget.exe http://downloads.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.7z
..\bin\win\7za.exe x boost_1_57_0.7z
FOR /D %%p IN ("boost_1_57_0\*") DO MOVE %%p .
move boost_1_57_0\* .
rmdir boost_1_57_0
del boost_1_57_0.7z

call .\bootstrap.bat
rem todo: --stagedir=lib\x64
rem .\b2 address-model=64 toolset=msvc-11.0 
rem .\b2 address-model=32 toolset=msvc-11.0 

.\b2 address-model=64 toolset=msvc-12.0 
rem .\b2 address-model=32 toolset=msvc-12.0 

cd ..
