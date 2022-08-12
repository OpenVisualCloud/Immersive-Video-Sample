#!/bin/bash -ex
SCRIPT=`pwd`/$0
PATHNAME=`dirname $SCRIPT`
ROOT=$PATHNAME/..
BUILD_DIR=$ROOT/build

LIB_DIR=$BUILD_DIR/libdeps
PREFIX_DIR=$LIB_DIR/build/

OWT_ROOT=$ROOT/third_party/owt-server/

install_mongodb() {
    pushd ${ROOT}/scripts > /dev/null
    sudo cp -f mongodb-org-4.4.repo /etc/yum.repos.d/
    sudo yum install mongodb-org -y
    popd
}

install_svt_hevc(){
  pushd $ROOT/third_party >/dev/null

  if [ -d SVT-HEVC ]
  then
    rm -rf SVT-HEVC
  fi

  git clone https://github.com/OpenVisualCloud/SVT-HEVC.git
  pushd SVT-HEVC >/dev/null
  git checkout v1.5.0
  # PR 580 is under review
  git fetch origin pull/580/head && git checkout FETCH_HEAD

  mkdir build
  pushd build >/dev/null
  cmake -DCMAKE_INSTALL_PREFIX=${PREFIX_DIR} -DCMAKE_INSTALL_LIBDIR=lib ..
  make -j4 && make install
  popd >/dev/null

  # pseudo lib
  echo \
    'const char* stub() {return "this is a stub lib";}' \
    > pseudo-svtHevcEnc.cpp
  gcc pseudo-svtHevcEnc.cpp -fPIC -shared -o pseudo-svtHevcEnc.so

  popd >/dev/null
  popd >/dev/null
}

install_safestringlib(){
  pushd $ROOT/third_party >/dev/null

  if [ -d safestringlib ]
  then
    rm -fr safestringlib
  fi

  git clone https://github.com/intel/safestringlib.git

  cd safestringlib
  git checkout 245c4b8cff1d2e7338b7f3a82828fc8e72b29549

  mkdir build
  cd build
  cmake ..
  make -j$(nproc)

  cp -v libsafestring_shared.so ${PREFIX_DIR}/lib/
  mkdir -p ${PREFIX_DIR}/include/safestringlib
  cp -rfv ../include/* ${PREFIX_DIR}/include/safestringlib/
}

install_360scvp(){
  sudo yum install -y glog-devel gflags-devel
  pushd $ROOT/third_party >/dev/null

  local SCVP_VER="826c61a3cc2804774697a9d7278032c895a4f17d"
  local SCVP_REPO=https://github.com/OpenVisualCloud/Immersive-Video-Sample.git

  if [ -d Immersive-Video-Sample ]
  then
    rm -fr Immersive-Video-Sample
  fi

  git clone ${SCVP_REPO}
  pushd Immersive-Video-Sample/src/360SCVP
  git checkout ${SCVP_VER}
  rm build -rf
  mkdir build
  pushd  build

  sed -i "s@INCLUDE_DIRECTORIES\(.*\)@INCLUDE_DIRECTORIES\1\nINCLUDE_DIRECTORIES(${PREFIX_DIR}/include)@" ../CMakeLists.txt
  sed -i "s@LINK_DIRECTORIES\(.*\)@LINK_DIRECTORIES\1\nLINK_DIRECTORIES(${PREFIX_DIR}/lib)@" ../CMakeLists.txt

  cmake -DCMAKE_INSTALL_PREFIX=${PREFIX_DIR} -DCMAKE_INSTALL_LIBDIR=lib ../
  make -j$(nproc)
  make install

  popd
  popd
  popd
}

install_yaml-cpp() {
  pushd ${ROOT}/third_party > /dev/null
  [ -d yaml-cpp ] && rm -rf yaml-cpp
  git clone -b yaml-cpp-0.6.3 https://github.com/jbeder/yaml-cpp.git
  pushd yaml-cpp > /dev/null
  mkdir build
  pushd build > /dev/null
  cmake -DYAML_BUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${PREFIX_DIR} ../
  make -j$(nproc)
  make install
  popd
  popd
  popd
}

rebuild_libwebrtc(){
  pushd $OWT_ROOT/third_party/webrtc-m79/src

  patch -f -p1 < $ROOT/scripts/patches/0001-Impl-Rtcp-FOV-Feedback.patch
  cd ..
  source ../../scripts/installWebrtc.sh

  popd
}

if [ ! -d $ROOT/third_party ]
then
  mkdir $ROOT/third_party
fi

if [ ! -d $PREFIX_DIR ]
then
  mkdir -p $PREFIX_DIR
fi

install_mongodb
install_svt_hevc
install_safestringlib
install_360scvp
install_yaml-cpp
rebuild_libwebrtc

