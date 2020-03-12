#!/bin/bash -x

TARGET=$1
cd ..
if [ ! -d "./FFmpeg" ];then 
    git clone https://github.com/FFmpeg/FFmpeg.git
fi
cd FFmpeg

if [ ${TARGET} == "server" ] ; then

    git checkout release/4.1
    git checkout c2ac3b8e6a040e33d53fa13548848c8ba981a8e4
    cd -
    patch -p1 < ffmpeg/patches/FFmpeg_OMAF.patch
    
    mkdir -p build/external/ffmpeg_server
    cd build/external/ffmpeg_server
    ../../../FFmpeg/configure --prefix=/usr --libdir=/usr/local/lib --enable-static --enable-shared --enable-gpl --enable-nonfree --disable-optimizations --disable-vaapi
    make -j $(nproc)
    sudo make install

elif [ ${TARGET} == "client" ] ; then

    patch -p1 < ../ffmpeg/patches/0001-Add-avcodec_receive_frame2-for-vaapi-hardware-decodi.patch
    cd -
    mkdir -p build/external/ffmpeg_client
    cd build/external/ffmpeg_client
    ../../../FFmpeg/configure --enable-shared
    make -j $(nproc)
    sudo make install
    
fi

