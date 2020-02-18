#!/bin/bash
./prebuild.sh server
sudo cp ../ffmpeg/dependency/*.so /usr/local/lib/
sudo cp ../ffmpeg/dependency/*.pc /usr/local/lib/pkgconfig/
sudo cp ../ffmpeg/dependency/*.h /usr/local/include/
sudo cp ../ffmpeg/dependency/WorkerServer /root

mkdir -p ../build/server
cd ../build/server
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=server ../..
make -j `nproc`
sudo make install

