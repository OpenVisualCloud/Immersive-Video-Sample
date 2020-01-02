#!/bin/bash -e

cd ../build/external
if [ ! -d "./glog" ] ; then
    git clone https://github.com/google/glog.git
fi

cd glog
./autogen.sh
./configure
make -j8
sudo make install
