#!/usr/bin/sh
if [ -d build ];then

else
    mkdir build
    fi
mkdir build
cmake  -Bbuild -H.
