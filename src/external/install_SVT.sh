#!/bin/sh -e

mkdir -p ../build/external
cd ../build/external

if [ ! -d "./SVT-HEVC" ] ; then
    git clone https://github.com/OpenVisualCloud/SVT-HEVC.git
fi

cd SVT-HEVC
git checkout ec0d95c7e0d5be20586e1b87150bdfb9ae97cf4d
patch -p1 < ../../../external/0001-strict-memory-for-svt.patch

cd Build/linux/
./build.sh
cd Release
sudo make install
