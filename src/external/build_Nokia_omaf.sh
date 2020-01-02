#!/bin/sh -e
cd ../build/external

if [ ! -d "./omaf" ] ; then
   git clone https://github.com/nokiatech/omaf.git
fi

if [ -L ${PWD}/../../OmafDashAccess/mp4lib ] ; then
    rm -rf ${PWD}/../../OmafDashAccess/mp4lib
fi
ln -s ${PWD}/omaf/Mp4/srcs ${PWD}/../../OmafDashAccess/mp4lib
cd omaf
patch -p1 < ../../../external/nokia_omaf_patch_for_extrator_reader.diff
cd Mp4/srcs

if [ ! -d "./build" ] ; then
    mkdir build
fi
cd build

cmake ..
make -j`nproc`

cp -r ../api/streamsegmenter ../../../../../../VROmafPacking/
sudo cp lib/libstreamsegmenter_static_fpic.a /usr/local/lib/
sudo cp lib/libstreamsegmenter_static.a /usr/local/lib/
sudo cp lib/libmp4vr_static_fpic.a /usr/local/lib/
sudo cp lib/libmp4vr_static.a /usr/local/lib/
