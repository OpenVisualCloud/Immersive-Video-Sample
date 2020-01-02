#!/bin/sh -e

cd ../build/external/ffmpeg
../../../ffmpeg/configure --prefix=/usr --libdir=/usr/local/lib --enable-static --disable-shared --enable-gpl --enable-nonfree --disable-optimizations --disable-vaapi
make -j `nproc`
sudo make install
cd -
