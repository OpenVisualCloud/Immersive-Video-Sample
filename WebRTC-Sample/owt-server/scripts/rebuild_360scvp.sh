#!/bin/bash -ex

#This script is for debug purpose.
#Make sure you've already build the 360SCVP before.

SCRIPT=`pwd`/$0
PATHNAME=`dirname $SCRIPT`
ROOT=$PATHNAME/..
BUILD_DIR=$ROOT/build

LIB_DIR=$BUILD_DIR/libdeps
PREFIX_DIR=$LIB_DIR/build/

OWT_ROOT=$ROOT/third_party/owt-server/

main() {
    pushd ${ROOT}/third_party/Immersive-Video-Sample/src/360SCVP
    [[ -d build ]] && rm build -rf
    mkdir build
    pushd build
    cmake -DCMAKE_INSTALL_PREFIX=${PREFIX_DIR} -DCMAKE_INSTALL_LIBDIR=lib ../
    make -j$(nproc)
    make install
    popd
    popd
}

main $@



