#!/bin/bash

ROOT=`pwd`
WEBRTC_LINUX_CLIENT_SDK=${ROOT}/webrtc_linux_client_sdk/release
PLAYER_BUILD=${ROOT}/player/

# configure
PLAYER_SRC=${ROOT}/../../src/player

mkdir -p ${PLAYER_BUILD}/Build
cd ${PLAYER_BUILD}/Build

cmake -DUSE_OMAF=OFF -DUSE_WEBRTC=ON -DWEBRTC_LINUX_SDK=${WEBRTC_LINUX_CLIENT_SDK} ${PLAYER_SRC}
make -j

rm -fv ${PLAYER_BUILD}/render
cp -v ${PLAYER_BUILD}/Build/render ${PLAYER_BUILD}/render

if [ ! -f ${PLAYER_BUILD}/config.xml ]; then
    cp -v ${PLAYER_SRC}/config.xml ${PLAYER_BUILD}/config.xml
    sed 's|<sourceType>.*</sourceType>|<sourceType>2</sourceType>|g' -i ${PLAYER_BUILD}/config.xml

    sed 's|<resolution>.*</resolution>|<resolution>4k</resolution>|g' -i ${PLAYER_BUILD}/config.xml
    sed 's|<server_url>.*</server_url>|<server_url>http://owt-server-ip:3001</server_url>|g' -i ${PLAYER_BUILD}/config.xml
fi

