#!/bin/sh

cd ../external
./build_glog.sh
./build_Nokia_omaf.sh
cd ../VROmafPacking
cmake ./
make
