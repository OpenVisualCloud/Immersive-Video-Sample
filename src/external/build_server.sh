#!/bin/bash
./prebuild.sh server
mkdir -p ../build/server
cd ../build/server
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=server ..
make -j `nproc`
make install

