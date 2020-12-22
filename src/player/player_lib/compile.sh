#!/bin/bash -ex

cmake -DUSE_OMAF=ON -DUSE_WEBRTC=OFF -DLINUX_OS=ON ./

make -j
