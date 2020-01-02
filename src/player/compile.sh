#!/bin/bash -ex

cp ../utils/tinyxml2.h ./
cp ../utils/tinyxml2.cpp ./

cmake -DUSE_OMAF=ON -DUSE_WEBRTC=OFF  ./

make -j
