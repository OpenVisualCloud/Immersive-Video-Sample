#!/bin/bash -ex

TARGET=$1
PREBUILD_FLAG=$2
EX_PATH=${PWD}
FFMPEG_PATH=${PWD}/../../..
GIT_SHORT_HEAD=`git rev-parse --short HEAD`

parameters_usage(){
    echo 'Usage: 1. <target>:           [ server, client, test ]'
    echo '       2. <prebuild_flag>:    [ y, n ]'
}

build_server(){
    if [ "${PREBUILD_FLAG}" == "y" ] ; then
        ./prebuild.sh server
    fi
    sudo cp ../ffmpeg/dependency/*.so /usr/local/lib/
    sudo cp ../ffmpeg/dependency/*.pc /usr/local/lib/pkgconfig/
    sudo cp ../ffmpeg/dependency/*.h /usr/local/include/
    sudo cp ../ffmpeg/dependency/WorkerServer /root

    mkdir -p ../build/server
    cd ../build/server
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
    cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=server ../..
    make -j $(nproc)
    sudo make install
}

build_client(){
    if [ "${PREBUILD_FLAG}" == "y" ] ; then
        ./prebuild.sh client
    fi

    mkdir -p ../build/client
    cd ../build/client
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
    cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=client ../../
    make -j $(nproc)
    sudo make install
    cp ../../player/config.xml ./player
}

build_ci(){
    source /opt/rh/devtoolset-7/enable
    PREBUILD_FLAG="n"

    # Build server
    ./build_Nokia_omaf.sh
    ./install_FFmpeg.sh server
    cd ${EX_PATH}

    build_server

    cd ${EX_PATH} && ./fpm.sh server ${GIT_SHORT_HEAD} && rm -rf ../FFmpeg

    # Build client
    ./install_FFmpeg.sh client
    cd ${EX_PATH}

    build_client 

    cd ${EX_PATH} && ./fpm.sh client ${GIT_SHORT_HEAD} && rm -rf ../FFmpeg
}

build_test(){
    echo "Compiling unit test ..."
    source /opt/rh/devtoolset-7/enable

    mkdir -p ../build/test && cd ../build/test
    cp -r ../../google_test/gtest/ .
    if [ ! -d "./googletest" ];then
        git clone https://github.com/google/googletest.git
        cd googletest && git checkout -b v1.8.x origin/v1.8.x
        cd googletest && mkdir build && cd build
        cmake -DBUILD_SHARED_LIBS=ON .. && make -j $(nproc)
        g++ -I../include/ -I.. -c ../src/gtest-all.cc -D_GLIBCXX_USE_CXX11_ABI=0
        g++ -I../include/ -I.. -c ../src/gtest_main.cc -D_GLIBCXX_USE_CXX11_ABI=0
        ar -rv libgtest.a gtest-all.o gtest_main.o
    fi

    cd ${EX_PATH}/..
    mkdir -p build/test/360SCVP
    mkdir -p build/test/distributed_encoder
    mkdir -p build/test/VROmafPacking
    mkdir -p build/test/OmafDashAccess

    # Compile 360SCVP test
    cd build/test/360SCVP && \
       g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
       ../../../360SCVP/test/testI360SCVP.cpp \
       -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -L/usr/local/lib testI360SCVP.o \
       ../googletest/googletest/build/libgtest.a -o \
       testI360SCVP -I/usr/local/include/ -l360SCVP \
       -lstdc++ -lpthread -lm -L/usr/local/lib

    # Compile OmafDashAccess test
    cd ../OmafDashAccess && \
       g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
         ../../../OmafDashAccess/test/testMediaSource.cpp \
         -I../../../utils -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
         ../../../OmafDashAccess/test/testMPDParser.cpp \
         -I../../../utils -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
         ../../../OmafDashAccess/test/testOmafReader.cpp \
         -I../../../utils -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
         ../../../OmafDashAccess/test/testOmafReaderManager.cpp \
         -I../../../utils -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -L/usr/local/lib testMediaSource.o \
         ../googletest/googletest/build/libgtest.a -o \
         testMediaSource -I/usr/local/include/ -lOmafDashAccess \
         -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
       g++ -L/usr/local/lib testMPDParser.o \
         ../googletest/googletest/build/libgtest.a -o \
         testMPDParser -I/usr/local/include/ -lOmafDashAccess \
         -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
       g++ -L/usr/local/lib testOmafReader.o \
         ../googletest/googletest/build/libgtest.a -o \
         testOmafReader -I/usr/local/include/ -lOmafDashAccess \
         -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
       g++ -L/usr/local/lib testOmafReaderManager.o \
         ../googletest/googletest/build/libgtest.a -o \
         testOmafReaderManager -I/usr/local/include/ -lOmafDashAccess\
         -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib

    # Compile VROmafPacking test
    cd ../VROmafPacking && \
       g++ -I../../../google_test -std=c++11 -g -c \
         ../../../VROmafPacking/test/testHevcNaluParser.cpp \
         -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -I../../../google_test -std=c++11 -g -c \
         ../../../VROmafPacking/test/testVideoStream.cpp \
         -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -I../../../google_test -std=c++11 -g -c \
         ../../../VROmafPacking/test/testExtractorTrack.cpp \
         -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -I../../../google_test -std=c++11 -g -c \
         ../../../VROmafPacking/test/testDefaultSegmentation.cpp \
         -D_GLIBCXX_USE_CXX11_ABI=0 && \
       g++ -L/usr/local/lib testHevcNaluParser.o \
         ../googletest/googletest/build/libgtest.a -o \
         testHevcNaluParser -I/usr/local/lib -lVROmafPacking \
         -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib && \
       g++ -L/usr/local/lib testVideoStream.o \
         ../googletest/googletest/build/libgtest.a -o \
         testVideoStream -I/usr/local/lib -lVROmafPacking \
         -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib && \
       g++ -L/usr/local/lib testExtractorTrack.o \
         ../googletest/googletest/build/libgtest.a -o \
         testExtractorTrack -I/usr/local/lib -lVROmafPacking \
         -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib && \
       g++ -L/usr/local/lib testDefaultSegmentation.o \
         ../googletest/googletest/build/libgtest.a -o \
         testDefaultSegmentation -I/usr/local/lib -lVROmafPacking \
         -l360SCVP -lstdc++ -lpthread -lm -L/usr/local/lib
}

if [ $# == 2 ] ; then

    if [ "${TARGET}" == "server" ] ; then
        if [ "${PREBUILD_FLAG}" != "y" ] && [ "${PREBUILD_FLAG}" != "n" ] ; then
            parameters_usage
            exit 1
        else
            build_server
        fi
    elif [ "${TARGET}" == "client" ] ; then
        if [ "${PREBUILD_FLAG}" != "y" ] && [ "${PREBUILD_FLAG}" != "n" ] ; then
            parameters_usage
            exit 1
        else
            build_client
        fi
    else
	parameters_usage
    fi

elif [ $# == 1 ] ; then

    if [ "${TARGET}" == "ci" ] ; then
        build_ci
    elif [ "${TARGET}" == "test" ] ; then
        build_test
    else
        parameters_usage
    fi

else
    parameters_usage
fi
