#!/bin/bash -e

mkdir -p ../build/external
cd ../build/external
if [ ! -d "./glog" ] ; then
    git clone https://github.com/google/glog.git
fi

cd glog
git checkout v0.5.0
sed -i '23s/OFF/ON/' CMakeLists.txt
cmake -H. -Bbuild -G "Unix Makefiles"
cmake --build build
# cmake --build build --target test
sudo cmake --build build --target install
