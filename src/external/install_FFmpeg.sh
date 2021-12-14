#!/bin/bash -ex

TARGET=$1
REPO=$2
FFMPEG_REPO=FFmpeg

cd ..
if [ "${REPO}" != "oss" ] ; then
    if [ ! -d "${FFMPEG_REPO}" ] ; then
        mkdir ${FFMPEG_REPO} && cd ${FFMPEG_REPO}
        git init
        git remote add origin https://github.com/FFmpeg/FFmpeg.git
        git fetch --depth 1 origin 6b6b9e593dd4d3aaf75f48d40a13ef03bdef9fdb
        git checkout FETCH_HEAD
        cd -
    fi
    if [ ! -f "${FFMPEG_REPO}/libavcodec/distributed_encoder.c" ] ; then
        cd ${FFMPEG_REPO}
        patch -p1 < ../ffmpeg/patches/FFmpeg_OMAF.patch
        cd ..
    fi
fi

if [ "${TARGET}" == "server" ] ; then

    mkdir -p build/external/ffmpeg_server
    cd build/external/ffmpeg_server
    ../../../${FFMPEG_REPO}/configure --prefix=/usr --libdir=/usr/local/lib \
        --enable-static --enable-shared --enable-gpl --enable-nonfree \
        --disable-optimizations --disable-vaapi
    make -j $(nproc)
    sudo make install

elif [ "${TARGET}" == "client" ] ; then

    mkdir -p build/external/ffmpeg_client
    cd build/external/ffmpeg_client
    ../../../${FFMPEG_REPO}/configure --enable-shared
    make -j $(nproc)
    sudo make install

fi
