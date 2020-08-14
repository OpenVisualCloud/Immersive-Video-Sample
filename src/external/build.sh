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
    ./install_safestringlib.sh
    mkdir -p ../build/server
    cd ../build/server
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
    if [ "$1" == "oss" ] ; then
        cd ${EX_PATH}/../distributed_encoder/util
        thrift -r --gen cpp shared.thrift
        patch gen-cpp/shared_types.h Implement_operator_RegionInformation.patch
        cd -
        cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=server -DDE_FLAG=true ../..
    else
        sudo cp ../../ffmpeg/dependency/*.so /usr/local/lib/
        sudo cp ../../ffmpeg/dependency/*.pc /usr/local/lib/pkgconfig/
        sudo cp ../../ffmpeg/dependency/*.h /usr/local/include/
        sudo cp ../../ffmpeg/dependency/WorkerServer /root
        cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=server ../..
    fi
    make -j $(nproc)
    sudo make install
}

build_client(){
    if [ "${PREBUILD_FLAG}" == "y" ] ; then
        ./prebuild.sh client
    fi
    ./install_safestringlib.sh
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
    if [ "$1" == "oss" ] ; then
        ./install_FFmpeg.sh server "oss"
        cd ${EX_PATH} && build_server "oss"
        mkdir -p ${EX_PATH}/../ffmpeg/dependency/
        cp /usr/local/lib/libDistributedEncoder.so ${EX_PATH}/../ffmpeg/dependency/
        cp /usr/local/lib/libEncoder.so ${EX_PATH}/../ffmpeg/dependency/
        cp /root/WorkerServer ${EX_PATH}/../ffmpeg/dependency/
    else
        ./install_FFmpeg.sh server
        cd ${EX_PATH} && build_server
    fi

    if [ "$1" == "oss" ] ; then
        cd ${EX_PATH}/../build/external && mkdir -p ffmpeg_server_so
        sudo cp /usr/local/lib/libavcodec.so ffmpeg_server_so/libavcodec.so.58
        sudo cp /usr/local/lib/libavutil.so ffmpeg_server_so/libavutil.so.56
        sudo cp /usr/local/lib/libavformat.so ffmpeg_server_so/libavformat.so.58
        sudo cp /usr/local/lib/libavfilter.so ffmpeg_server_so/libavfilter.so.7
        sudo cp /usr/local/lib/libswresample.so ffmpeg_server_so/libswresample.so.3
        sudo cp /usr/local/lib/libpostproc.so ffmpeg_server_so/libpostproc.so.55
        sudo cp /usr/local/lib/libswscale.so ffmpeg_server_so/libswscale.so.5
        cd ${EX_PATH} && ./fpm.sh server ${GIT_SHORT_HEAD}
    fi

    # Build client
    cd ${EX_PATH} && ./install_FFmpeg.sh client
    cd ${EX_PATH} && build_client

    if [ "$1" == "oss" ] ; then
        cd ${EX_PATH}/../build/external && mkdir -p ffmpeg_client_so
        sudo cp ffmpeg_client/libavcodec/libavcodec.so.58 ffmpeg_client_so/libavcodec.so.58
        sudo cp ffmpeg_client/libavutil/libavutil.so.56 ffmpeg_client_so/libavutil.so.56
        sudo cp ffmpeg_client/libavformat/libavformat.so.58 ffmpeg_client_so/libavformat.so.58
        sudo cp ffmpeg_client/libavfilter/libavfilter.so.7 ffmpeg_client_so/libavfilter.so.7
        sudo cp ffmpeg_client/libavdevice/libavdevice.so.58 ffmpeg_client_so/libavdevice.so.58
        sudo cp ffmpeg_client/libswscale/libswscale.so.5 ffmpeg_client_so/libswscale.so.5
        sudo cp ffmpeg_client/libswresample/libswresample.so.3 ffmpeg_client_so/libswresample.so.3
        sudo cp /usr/local/lib/libpostproc.so.55 ffmpeg_client_so/libpostproc.so.55
        cd ${EX_PATH} && ./fpm.sh client ${GIT_SHORT_HEAD}
    fi
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
    mkdir -p build/test/OmafDashAccess
    mkdir -p build/test/VROmafPacking
    mkdir -p build/test/distributed_encoder

    # Compile 360SCVP test
    cd build/test/360SCVP && \
        g++ -I../../../google_test -std=c++11 -I../util/ -g -c \
          ../../../360SCVP/test/testI360SCVP.cpp \
          -D_GLIBCXX_USE_CXX11_ABI=0 && \
        g++ -L/usr/local/lib testI360SCVP.o \
          ../googletest/googletest/build/libgtest.a -o \
          testI360SCVP -I/usr/local/include/ -l360SCVP -lglog \
          -lstdc++ -lpthread -lsafestring_shared -lm -L/usr/local/lib

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
          testMediaSource -I/usr/local/include/ -lOmafDashAccess -lsafestring_shared \
          -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
        g++ -L/usr/local/lib testMPDParser.o \
          ../googletest/googletest/build/libgtest.a -o \
          testMPDParser -I/usr/local/include/ -lOmafDashAccess -lsafestring_shared \
          -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
        g++ -L/usr/local/lib testOmafReader.o \
          ../googletest/googletest/build/libgtest.a -o \
          testOmafReader -I/usr/local/include/ -lOmafDashAccess -lsafestring_shared \
          -lstdc++ -lpthread -lglog -l360SCVP -lm -L/usr/local/lib && \
        g++ -L/usr/local/lib testOmafReaderManager.o \
          ../googletest/googletest/build/libgtest.a -o \
          testOmafReaderManager -I/usr/local/include/ -lOmafDashAccess -lsafestring_shared\
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
          testHevcNaluParser -I/usr/local/include -lVROmafPacking \
          -l360SCVP -lsafestring_shared -lstdc++ -lpthread -lm \
          -L/usr/local/lib && \
        g++ -L/usr/local/lib testVideoStream.o \
          ../googletest/googletest/build/libgtest.a -o \
          testVideoStream -I/usr/local/include -lVROmafPacking \
          -l360SCVP -lsafestring_shared -lstdc++ -lpthread -lm \
          -L/usr/local/lib && \
        g++ -L/usr/local/lib testExtractorTrack.o \
          ../googletest/googletest/build/libgtest.a -o \
          testExtractorTrack -I/usr/local/include -lVROmafPacking \
          -l360SCVP -lsafestring_shared -lstdc++ -lpthread -lm \
          -L/usr/local/lib && \
        g++ -L/usr/local/lib testDefaultSegmentation.o \
          ../googletest/googletest/build/libgtest.a -o \
          testDefaultSegmentation -I/usr/local/include -lVROmafPacking \
          -l360SCVP -lsafestring_shared -lstdc++ -lpthread -lm \
          -L/usr/local/lib

    if [ "$1" == "oss" ] ; then
        # Compile distributed_encoder test
        cd ../distributed_encoder && \
            g++ -I../../../google_test -std=c++11 \
              -I/usr/local/include/svt-hevc \
              -I../../../distributed_encoder/util/ -g -c \
              ../../../distributed_encoder/test/testMainEncoder.cpp \
              -D_GLIBCXX_USE_CXX11_ABI=0 && \
            g++ -I../../../google_test -std=c++11 \
              -I../../../distributed_encoder/util/ -g -c \
              ../../../distributed_encoder/test/testWorkSession.cpp \
              -D_GLIBCXX_USE_CXX11_ABI=0 && \
            g++ -I../../../google_test -std=c++11 \
              -I../../../distributed_encoder/util/ -g -c \
              ../../../distributed_encoder/test/testDecoder.cpp \
              -D_GLIBCXX_USE_CXX11_ABI=0 && \
            g++ -I../../../google_test -std=c++11 \
              -I../../../distributed_encoder/util/ -g -c \
              ../../../distributed_encoder/test/testSubEncoder.cpp \
              -D_GLIBCXX_USE_CXX11_ABI=0 -I/usr/local/include/svt-hevc && \
            g++ -I../../../google_test -std=c++11 \
              -I../../../distributed_encoder/util/ -g -c \
              ../../../distributed_encoder/test/testSubEncoderManager.cpp \
              -D_GLIBCXX_USE_CXX11_ABI=0 && \
            g++ -I../../../google_test -std=c++11 \
              -I../../../distributed_encoder/util/ -g -c \
              ../../../distributed_encoder/test/testEncoder.cpp \
              -D_GLIBCXX_USE_CXX11_ABI=0 && \
            g++ -L/usr/local/lib testMainEncoder.o \
              ../googletest/googletest/build/libgtest.a -o testMainEncoder \
              -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
              -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
              -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
              -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
              -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
              -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
              -L/usr/local/lib && \
            g++ -L/usr/local/lib testWorkSession.o \
              ../googletest/googletest/build/libgtest.a -o testWorkSession \
              -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
              -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
              -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
              -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
              -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
              -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
              -L/usr/local/lib && \
            g++ -L/usr/local/lib testDecoder.o \
              ../googletest/googletest/build/libgtest.a -o testDecoder \
              -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
              -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
              -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
              -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
              -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
              -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
              -L/usr/local/lib && \
            g++ -L/usr/local/lib testSubEncoder.o \
              ../googletest/googletest/build/libgtest.a -o testSubEncoder \
              -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
              -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
              -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
              -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
              -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
              -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
              -L/usr/local/lib && \
            g++ -L/usr/local/lib testEncoder.o \
              ../googletest/googletest/build/libgtest.a -o testEncoder \
              -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
              -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
              -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
              -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
              -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
              -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
              -L/usr/local/lib && \
            g++ -L/usr/local/lib testSubEncoderManager.o \
              ../googletest/googletest/build/libgtest.a -o testSubEncoderManager \
              -I/usr/local/include/thrift -I/usr/local/include/svt-hevc \
              -lDistributedEncoder -lEncoder -lstdc++ -lpthread -lthrift \
              -lSvtHevcEnc -lopenhevc -lthriftnb -levent -lglog -pthread \
              -lavdevice -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes -lavfilter \
              -lswscale -lavformat -lavcodec -llzma -lz -lswresample -lavutil \
              -lva-drm -lva-x11 -lm -lva -lXv -lX11 -lXext -l360SCVP \
              -L/usr/local/lib
    fi
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
    elif [ "${TARGET}" == "ci_oss" ] ; then
        build_ci "oss"
    elif [ "${TARGET}" == "test" ] ; then
        build_test
    elif [ "${TARGET}" == "test_oss" ] ; then
        build_test "oss"
    else
        parameters_usage
    fi

else
    parameters_usage
fi
