#!/bin/bash -x

TARGET=$1
REPO=$2

cd ..
if [ "${REPO}" != "oss" ] ; then
    if [ ! -d "./FFmpeg" ] ; then
        if [ ! -f "./ffmpeg-4.3.1.tar.xz" ] ; then
            wget http://ffmpeg.org/releases/ffmpeg-4.3.1.tar.xz
        fi
        tar xf ffmpeg-4.3.1.tar.xz && mv ffmpeg-4.3.1 FFmpeg
    fi
    if [ ! -f "FFmpeg/libavcodec/distributed_encoder.c" ] ; then
        cd FFmpeg
        patch -p1 < ../ffmpeg/patches/FFmpeg_OMAF.patch
        cd ..
    fi
fi

if [ "${TARGET}" == "server" ] ; then

    mkdir -p build/external/ffmpeg_server
    cd build/external/ffmpeg_server
    ../../../FFmpeg/configure --prefix=/usr --libdir=/usr/local/lib \
        --enable-static --enable-shared --enable-gpl --enable-nonfree \
        --disable-optimizations --disable-vaapi --disable-filter=erp2cubmap_mdf
    make -j $(nproc)
    sudo make install

elif [ "${TARGET}" == "client" ] ; then

    mkdir -p build/external/ffmpeg_client
    cd build/external/ffmpeg_client
    ../../../FFmpeg/configure --enable-shared --disable-filter=erp2cubmap_mdf
    make -j $(nproc)
    sudo make install

fi
