#!/bin/sh

cd ../external
./build_glog.sh
./build_Nokia_omaf.sh
cd ../VROmafPacking
cp ../utils/tinyxml2.h ./
cp ../utils/tinyxml2.cpp ./
cmake ./
make
