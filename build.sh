#!/usr/bin/sh
if [ ! -d build ];then
    mkdir build
fi

if [ ! -f build/Makefile ];then
    cmake  -Bbuild -H.
fi
cd build
make
cd ..
