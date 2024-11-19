#!/usr/bin/sh
# xmake project -k compile_commands
if [ ! -d build ];then
    mkdir build
fi
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES ..
if [  $? -eq 0 ];then
   mv -f compile_commands.json ..
else
    echo "gen compile_commands.json error"
fi
