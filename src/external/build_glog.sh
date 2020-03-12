#!/bin/bash -e

mkdir -p ../build/external
cd ../build/external
if [ ! -d "./glog" ] ; then
    git clone https://github.com/google/glog.git
fi

cd glog
./autogen.sh
./configure
make -j $(nproc)
sudo make install
