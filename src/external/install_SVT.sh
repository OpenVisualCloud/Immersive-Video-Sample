#!/bin/sh -e

AVX512FLAG=$1

mkdir -p ../build/external
cd ../build/external

if [ ! -d "./SVT-HEVC" ] ; then
    git clone https://github.com/OpenVisualCloud/SVT-HEVC.git
fi

cd SVT-HEVC
git checkout ec0d95c7e0d5be20586e1b87150bdfb9ae97cf4d
patch -p1 < ../../../external/0001-strict-memory-for-svt.patch
if [ "$AVX512FLAG" == "avx512" ] ; then
    sed -i '16d' Source/Lib/Codec/EbDefinitions.h
fi

cd Build/linux/
./build.sh
cd Release
sudo make install
