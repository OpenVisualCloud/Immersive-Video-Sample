#!/bin/bash -x
NDK_r18b_PATH="../../external/android/android-ndk-r18b"
echo "Start build android ndk for client libraries ..."
mkdir -p ../build/android/360SCVP
mkdir -p ../build/android/isolib
mkdir -p ../build/android/OmafDashAccess
mkdir -p ../build/android/player/player_lib

# Install 360SCVP
cd ../build/android/360SCVP && \
cmake ../../../360SCVP -DUSE_ANDROID_NDK=ON -DDEBUG=NO -DCMAKE_TOOLCHAIN_FILE=${NDK_r18b_PATH}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_TOOLCHAIN=aarch64-linux-android-4.9 -DANDROID_PLATFORM=android-21 -DANDROID_STD=c++_shared && \
make -j && \
sudo make install

# Install isolib
cd ../isolib && \
cmake ../../../isolib -DUSE_ANDROID_NDK=ON -DDEBUG=NO -DCMAKE_TOOLCHAIN_FILE=${NDK_r18b_PATH}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_TOOLCHAIN=aarch64-linux-android-4.9 -DANDROID_PLATFORM=android-21 -DANDROID_STD=c++_static && \
make -j && \
sudo cp dash_parser/libdashparser.a /usr/local/lib/

# Install OmafDashAccess
cd ../OmafDashAccess && \
cmake ../../../OmafDashAccess -DUSE_ANDROID_NDK=ON -DDEBUG=NO -DCMAKE_TOOLCHAIN_FILE=${NDK_r18b_PATH}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_TOOLCHAIN=aarch64-linux-android-4.9 -DANDROID_PLATFORM=android-21 -DANDROID_STD=c++_shared && \
make -j && \
sudo make install

cd ../player/player_lib && \
cmake ../../../../player/player_lib -DUSE_OMAF=ON -DANDROID_OS=ON -DDEBUG=NO -DCMAKE_TOOLCHAIN_FILE=../${NDK_r18b_PATH}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_TOOLCHAIN=aarch64-linux-android-4.9 -DANDROID_PLATFORM=android-21 -DANDROID_STD=c++_shared && \
make -j && \
sudo make install

cd ../../../../

mkdir -p ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp /usr/local/lib/libcurl.so ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp /usr/local/lib/libsafestring_shared.so ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp ./build/external/android/openssl-output/lib/libssl.so ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp /usr/local/lib/libglog.so ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp ./build/external/android/openssl-output/lib/libcrypto.so ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp /usr/local/lib/lib360SCVP.so ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp /usr/local/lib/libOmafDashAccess.so ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp /usr/local/lib/libdashparser.a ./player/app/android/app/src/main/jniLibs/arm64-v8a/
sudo cp /usr/local/lib/libMediaPlayer.so ./player/app/android/app/src/main/jniLibs/arm64-v8a/