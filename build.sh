#!/usr/bin/sh
if [ ! -d build ];then
    mkdir build
fi
mkdir build
cmake  -Bbuild -H.
