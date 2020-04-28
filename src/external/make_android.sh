#!/bin/bash -x
NDK_r18b_PATH="../../external/android/android-ndk-r18b"
echo "Start build android ndk for client libraries ..."
mkdir -p ../build/android/360SCVP
mkdir -p ../build/android/isolib
mkdir -p ../build/android/OmafDashAccess

# Install 360SCVP
cd ../build/android/360SCVP && \
cmake ../../../360SCVP -DUSE_ANDROID_NDK=ON -DDEBUG=NO -DCMAKE_TOOLCHAIN_FILE=${NDK_r18b_PATH}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_TOOLCHAIN=aarch64-linux-android-4.9 -DANDROID_PLATFORM=android-21 -DANDROID_STD=c++_shared && \
make -j && \
sudo make install

# Install isolib
cd ../isolib && \
cmake ../../../isolib -DDEBUG=NO -DCMAKE_TOOLCHAIN_FILE=${NDK_r18b_PATH}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_TOOLCHAIN=aarch64-linux-android-4.9 -DANDROID_PLATFORM=android-21 -DANDROID_STD=c++_static && \
make -j && \
sudo cp dash_parser/libdashparser.a /usr/local/lib/

# Install OmafDashAccess
cd ../OmafDashAccess && \
cmake ../../../OmafDashAccess -DUSE_ANDROID_NDK=ON -DDEBUG=NO -DCMAKE_TOOLCHAIN_FILE=${NDK_r18b_PATH}/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_TOOLCHAIN=aarch64-linux-android-4.9 -DANDROID_PLATFORM=android-21 -DANDROID_STD=c++_shared && \
make -j && \
sudo make install
