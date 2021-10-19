#!/bin/bash -e

SUDO=""
if [[ $EUID -ne 0 ]]; then
  SUDO="sudo -E"
fi

ROOT=`pwd`/webrtc_linux_client_sdk
BUILD=${ROOT}/Build
PREFIX=${ROOT}/release
DEPS=${BUILD}/deps
PATCHES=${ROOT}/../patches

install_dependencies() {
    ${SUDO} apt-get update
    ${SUDO} apt install -y git build-essential wget python cmake pkg-config libglib2.0-dev libgtk-3-dev libasound2-dev libpulse-dev

    # player
    ${SUDO} apt install -y yasm libgoogle-glog-dev libva-dev libglm-dev libglfw3-dev libgles2-mesa-dev libglu1-mesa-dev liblzma-dev

    # set git user
    if [ -z "`git config --get user.email`" ]; then
        git config --global user.email "you@example.com"
    fi

    if [ -z "`git config --get user.name`" ]; then
        git config --global user.name "Your Name"
    fi
}

install_openssl() {
    cd ${DEPS}
    local SSL_VERSION="1_1_1h"
    rm -rf openssl* OpenSSL*

    wget -c https://github.com/openssl/openssl/archive/OpenSSL_${SSL_VERSION}.tar.gz
    tar xf OpenSSL_${SSL_VERSION}.tar.gz
    cd openssl-OpenSSL_${SSL_VERSION}

    ./config shared -m64 --prefix=${PREFIX} --openssldir=${PREFIX}
    make -j
    make install
}

install_socket_io_client() {
    cd ${DEPS}

    rm -rf socket.io-client-cpp
    git clone -b 2.x --recurse-submodules https://github.com/socketio/socket.io-client-cpp.git

    cd socket.io-client-cpp
    mkdir -p build
    cd build

    cmake -DCMAKE_C_FLAGS="-fPIC" -DCMAKE_CXX_FLAGS="-fPIC" -DOPENSSL_ROOT_DIR:STRING=${PREFIX} -DCMAKE_INSTALL_INCLUDEDIR=${PREFIX}/include -DCMAKE_INSTALL_LIBDIR=${PREFIX}/lib ../
    make -j
    make install
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

    rm -fr src
    git clone https://github.com/open-webrtc-toolkit/owt-client-native.git src
    cd src
    git checkout 2a9d948b59502559843d63775a395affb10cb128
    sed -i 's/2fa91a1fc71b324ab46483777d7e6da90c57d3c6/28f5c7fd13db33267dcd7ad18851e9750c59d69a/g' DEPS

    gclient sync --no-history
    cd third_party/webrtc
    patch -p1 < ${PATCHES}/webrtc-Implement-FOV-RTCP-feedback.patch
    cd -

    patch -p1 < ${PATCHES}/sdk-Implement-FOV-RTCP-feedback.patch
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
  ./configure --prefix=${PREFIX} --enable-shared --disable-static --disable-vaapi
  make -j
  make install
}

install_safestringlib(){
  cd ${BUILD}

  rm -fr safestringlib
  git clone https://github.com/intel/safestringlib.git

  cd safestringlib
  git checkout 245c4b8cff1d2e7338b7f3a82828fc8e72b29549

  mkdir build
  cd build
  cmake ..
  make -j

  cp -v libsafestring_shared.so ${PREFIX}/lib/
  mkdir -p ${PREFIX}/include/safestringlib
  cp -rfv ../include/* ${PREFIX}/include/safestringlib/
}

install_360scvp(){
  cd ${BUILD}

  rm -rf 360scvp

  mkdir 360scvp
  cd 360scvp

  sed -i "s@INCLUDE_DIRECTORIES\(.*\)@INCLUDE_DIRECTORIES\1\nINCLUDE_DIRECTORIES(${PREFIX}/include)@" ${BUILD}/../../../../src/360SCVP/CMakeLists.txt
  sed -i "s@LINK_DIRECTORIES\(.*\)@LINK_DIRECTORIES\1\nLINK_DIRECTORIES(${PREFIX}/lib)@" ${BUILD}/../../../../src/360SCVP/CMakeLists.txt

  cmake -DCMAKE_INSTALL_PREFIX=${PREFIX} ${BUILD}/../../../../src/360SCVP/
  make -j
  make install
}

mkdir -p ${BUILD}
mkdir -p ${DEPS}

install_dependencies
install_openssl
install_socket_io_client
install_owt_client_native

# player
install_ffmpeg
install_safestringlib
install_360scvp

