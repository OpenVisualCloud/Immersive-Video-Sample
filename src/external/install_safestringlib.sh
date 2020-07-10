#!/bin/bash -e

mkdir -p ../build/external
cd ../build/external
if [ ! -d "./safestringlib" ] ; then
    git clone https://github.com/intel/safestringlib.git
fi

cd safestringlib
cmake .
make -j $(nproc) -f Makefile
sudo cp libsafestring_shared.so /usr/local/lib/
sudo mkdir -p /usr/local/include/safestringlib/
sudo cp ./include/* /usr/local/include/safestringlib/
