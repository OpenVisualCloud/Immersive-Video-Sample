#!/bin/bash -ex

ROOT=`pwd`
WEBRTC_LINUX_CLIENT_SDK=${ROOT}/webrtc_linux_client_sdk/release
PLAYER_BUILD=${ROOT}/player/

# configure
IMMERSIVE_ROOT=${ROOT}/../../src

if [ ! -f ${IMMERSIVE_ROOT}/player/tinyxml2.h ]; then
    cp -v ${IMMERSIVE_ROOT}/utils/tinyxml2.h ${IMMERSIVE_ROOT}/player/
fi

if [ ! -f ${IMMERSIVE_ROOT}/player/tinyxml2.cpp ]; then
    cp -v ${IMMERSIVE_ROOT}/utils/tinyxml2.cpp ${IMMERSIVE_ROOT}/player/
fi

mkdir -p ${PLAYER_BUILD}/Build
cd ${PLAYER_BUILD}/Build

cmake -DUSE_OMAF=OFF -DUSE_WEBRTC=ON -DWEBRTC_LINUX_SDK=${WEBRTC_LINUX_CLIENT_SDK} ${IMMERSIVE_ROOT}/player
make -j

rm -fv ${PLAYER_BUILD}/render
cp -v ${PLAYER_BUILD}/Build/render ${PLAYER_BUILD}/render

if [ ! -f ${PLAYER_BUILD}/config.xml ]; then
    cp -v ${IMMERSIVE_ROOT}/player/config.xml ${PLAYER_BUILD}/config.xml
    sed 's|<sourceType>.*</sourceType>|<sourceType>2</sourceType>|g' -i ${PLAYER_BUILD}/config.xml

    sed 's|<resolution>4k</resolution>|<resolution>4k</resolution>|g' -i ${PLAYER_BUILD}/config.xml
    sed 's|<server_url>.*</server_url>|<server_url>http://owt-server-ip:3001</server_url>|g' -i ${PLAYER_BUILD}/config.xml
fi

