#!/bin/bash

rm -fr boost
mkdir boost
cd boost

if [ -n "`which wget`" ]; then 
	wget http://downloads.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.zip
elif [ -n "`which curl`" ]; then 
	curl -LO http://downloads.sourceforge.net/project/boost/boost/1.57.0/boost_1_57_0.zip
fi
unzip boost_1_57_0.zip
mv boost_1_57_0/* .
rmdir boost_1_57_0
rm boost_1_57_0.zip

if [ `uname -s` == "Linux" ]; then

./bootstrap.sh
./b2 --adress-model=64 --toolset=gcc-4.9

elif [ `uname -s` == "Darwin" ]; then

cat >>tools/build/example/user-config.jam <<EOF
using clang : 11
    : "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
    : <cxxflags>"-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk -std=c++11 -stdlib=libc++" <linkflags>"-stdlib=libc++"
    ;
EOF

./bootstrap.sh --with-toolset=clang
./bjam toolset=clang-11

fi

