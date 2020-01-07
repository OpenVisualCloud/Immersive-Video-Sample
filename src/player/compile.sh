#!/bin/bash -ex

cmake -DUSE_OMAF=ON -DUSE_WEBRTC=OFF  ./

make -j
