#!/bin/bash -x
./prebuild.sh client
mkdir -p ../build/client
cd ../build/client
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=client ..
make -j `nproc`
make install

