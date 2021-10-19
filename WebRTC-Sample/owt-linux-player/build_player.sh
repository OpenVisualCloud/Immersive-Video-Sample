#!/bin/bash -e

ROOT=`pwd`
WEBRTC_LINUX_CLIENT_SDK=${ROOT}/webrtc_linux_client_sdk/release
PLAYER_BUILD=${ROOT}/player/


build_player_lib() {
    local src=${ROOT}/../../src/player/player_lib
    local build=${ROOT}/player/player_lib_build
    [[ ! -d ${build} ]] && mkdir -p ${build}
    pushd ${build}
    cmake -DLINUX_OS=ON -DUSE_OMAF=OFF -DUSE_WEBRTC=ON -DWEBRTC_LINUX_SDK=${WEBRTC_LINUX_CLIENT_SDK} ${src}
    make -j$(nproc)

    if [ -f ${WEBRTC_LINUX_CLIENT_SDK}/lib/libMediaPlayer.so ]; then
        rm -v ${WEBRTC_LINUX_CLIENT_SDK}/lib/libMediaPlayer.so
    fi
    cp -v ${build}/libMediaPlayer.so ${WEBRTC_LINUX_CLIENT_SDK}/lib/
    popd
}

build_player_app() {
    local src=${ROOT}/../../src/player/app
    local build=${ROOT}/player/player_app_build
    [[ ! -d ${build} ]] && mkdir -p ${build}
    pushd ${build}
    cmake -DUSE_OMAF=OFF -DUSE_WEBRTC=ON -DWEBRTC_LINUX_SDK=${WEBRTC_LINUX_CLIENT_SDK} ${src}
    make -j${nproc}

    [[ ! -f ${PLAYER_BUILD}/render ]] && rm -fv ${PLAYER_BUILD}/render
    cp -v ${build}/render ${PLAYER_BUILD}/render

    if [ ! -f ${PLAYER_BUILD}/config.xml ]; then
        cp -v ${src}/linux/config.xml ${PLAYER_BUILD}/config.xml
        sed 's|<sourceType>.*</sourceType>|<sourceType>2</sourceType>|g' -i ${PLAYER_BUILD}/config.xml
        sed 's|<resolution>.*</resolution>|<resolution>4k</resolution>|g' -i ${PLAYER_BUILD}/config.xml
        sed 's|<server_url>.*</server_url>|<server_url>http://owt-server-ip:3001</server_url>|g' -i ${PLAYER_BUILD}/config.xml
    fi

    if [ ! -f ${PLAYER_BUILD}/setupvars.sh ]; then
        echo "export LD_LIBRARY_PATH=${WEBRTC_LINUX_CLIENT_SDK}/lib:\$LD_LIBRARY_PATH" > ${PLAYER_BUILD}/setupvars.sh
    fi
}


main() {
    build_player_lib
    build_player_app
}

main $@
