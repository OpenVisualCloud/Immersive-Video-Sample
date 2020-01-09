#!/bin/bash

ROOT=`pwd`/webrtc_linux_client_sdk
BUILD=${ROOT}/Build
PREFIX=${ROOT}/release
DEPS=${BUILD}/deps

install_dependencies() {
    if [ -z "`git config --get user.email`" ]; then
        git config --global user.email "you@example.com"
    fi

    if [ -z "`git config --get user.name`" ]; then
        git config --global user.name "Your Name"
    fi

    sudo -E apt-get update
    sudo -E apt install -y git build-essential wget python cmake pkg-config libglib2.0-dev libgtk-3-dev libasound2-dev libpulse-dev

    # player
    sudo -E apt install -y yasm libgoogle-glog-dev libva-dev libglm-dev libglfw3-dev libgles2-mesa-dev libglu1-mesa-dev liblzma-dev
}

install_openssl() {
    cd ${DEPS}

    rm -rf openssl-1.1.0l.tar.gz openssl-1.1.0l

    wget https://www.openssl.org/source/openssl-1.1.0l.tar.gz
    tar -zxvf openssl-1.1.0l.tar.gz
    cd openssl-1.1.0l

    ./config no-shared -m64 --prefix=${PREFIX} --openssldir=${PREFIX}
    make -j
    make install
}

install_boost() {
    cd ${DEPS}

    rm -rf boost_1_67_0.tar.gz boost_1_67_0

    wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz
    tar -zxvf boost_1_67_0.tar.gz
    cd boost_1_67_0

    ./bootstrap.sh
    ./b2 -j`nproc` variant=release link=static runtime-link=shared --with-system --with-random --with-date_time --with-regex --with-thread --with-filesystem --with-chrono --with-atomic
}

install_socket_io_client() {
    cd ${DEPS}

    rm -rf socket.io-client-cpp

    git clone --recurse-submodules https://github.com/socketio/socket.io-client-cpp.git
    cd socket.io-client-cpp/lib/websocketpp
    git pull origin master
    cd ../..

    mkdir -p build
    cd build
    cmake -DBOOST_ROOT:STRING=${DEPS}/boost_1_67_0 -DOPENSSL_ROOT_DIR:STRING=${PREFIX} ../
    make -j
    make install

    cp -v lib/Release/* ${PREFIX}/lib
    cp -v include/* ${PREFIX}/include
}

install_depot_tools() {
    cd ${DEPS}

    rm -rf depot_tools

    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
}

gen_gclient () {
    echo 'solutions = [
    {
        "managed": False,
        "name": "src",
        "url": "https://github.com/open-webrtc-toolkit/owt-client-native",
        "custom_deps": {},
        "deps_file": "DEPS",
        "safesync_url": "",
    },
]
target_os = []' > .gclient
}

install_owt_client_native () {
    install_depot_tools
    export PATH=${DEPS}/depot_tools:$PATH

    cd ${BUILD}

    mkdir -p owt-client-native
    cd owt-client-native
    gen_gclient

    git clone https://github.com/open-webrtc-toolkit/owt-client-native.git src
    cd src
    git checkout 2a9d948b59502559843d63775a395affb10cb128

    gclient sync --no-history

    sed -i 's/rtc_use_h264=true/rtc_use_h264=false/g' scripts/build_linux.py

    python scripts/build_linux.py --gn_gen --sdk --arch x64 --ssl_root ${PREFIX} --scheme release --output_path "out"

    cp -rfv out/include/* ${PREFIX}/include/
    cp -v out/libowt-release.a ${PREFIX}/lib/libowt.a
}

install_ffmpeg(){
  local VERSION="4.1.3"
  local DIR="ffmpeg-${VERSION}"
  local SRC="${DIR}.tar.bz2"
  local SRC_URL="http://ffmpeg.org/releases/${SRC}"

  cd ${BUILD}

  wget ${SRC_URL}
  rm -fr ${DIR}
  tar xf ${SRC}
  cd ${DIR}
  ./configure --prefix=${PREFIX} --disable-shared --enable-static --disable-vaapi
  make -j
  make install
}

install_360scvp(){
  cd ${BUILD}

  rm -rf 360scvp

  mkdir 360scvp
  cd 360scvp
  cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} ${BUILD}/../../../../src/360SCVP/
  make -j
  make install
}

mkdir -p ${BUILD}
mkdir -p ${DEPS}

install_dependencies
install_openssl
install_boost
install_socket_io_client
install_owt_client_native

# player
install_ffmpeg
install_360scvp
