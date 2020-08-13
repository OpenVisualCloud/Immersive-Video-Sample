#!/bin/sh -e

mkdir -p ../build/external
cd ../build/external

if [ ! -d "./openHEVC" ] ; then
    git clone https://github.com/OpenHEVC/openHEVC.git
fi

cd openHEVC
git checkout ffmpeg_update
patch -p1 < ../../../external/ffmpeg_update_add_circle_list_for_to_free_frame.patch
./configure --libdir=/usr/lib64 --disable-sdl2
make -j `nproc`
sudo make install
