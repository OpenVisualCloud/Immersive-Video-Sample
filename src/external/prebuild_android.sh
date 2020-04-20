#!/bin/bash
mkdir -p ../build/external/android && cd ../build/external/android
#NDK download
if [ ! -d "android-ndk-r18b" ];then
    wget https://dl.google.com/android/repository/android-ndk-r18b-linux-x86_64.zip
    unzip android-ndk-r18b-linux-x86_64.zip
fi
cd android-ndk-r18b
#NDK path
NDK_r18b_PATH=${PWD}
cd ../
#glog
if [ ! -d "./glog" ];then
    git clone https://github.com/google/glog.git
fi
cd glog
patch -p1 < ../../../../external/0001-Update-glob-header-cpp-file-for-android-ndk-build.patch
if [ ! -d "./build" ];then
    mkdir build
fi
cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DDEBUG=NO -DCMAKE_TOOLCHAIN_FILE=$NDK_r18b_PATH/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_TOOLCHAIN=aarch64-linux-android-4.9 -DANDROID_PLATFORM=android-21 -DANDROID_STD=c++_shared
make -j
sudo make install
cd ../..
#curl
#NDK r14b download
if [ ! -d "android-ndk-r14b" ];then
    wget https://dl.google.com/android/repository/android-ndk-r14b-linux-x86_64.zip
    unzip android-ndk-r14b-linux-x86_64.zip
fi

if [ ! -d "./curl-7.66.0" ];then
    wget https://curl.haxx.se/download/curl-7.66.0.tar.xz
    tar xf curl-7.66.0.tar.xz
fi
if [ ! -d "./openssl-1.1.0f" ];then
    wget https://www.openssl.org/source/old/1.1.0/openssl-1.1.0f.tar.gz
    tar -xvf openssl-1.1.0f.tar.gz
fi
cd ../../../external
./openssl_arm64.sh
./curl_arm64.sh
cd ../build/external/android && sudo cp ./curl-output/arm64-v8a/lib/libcurl.so /usr/local/lib/
