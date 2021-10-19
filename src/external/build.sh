#!/bin/bash -ex

TARGET=$1
PREBUILD_FLAG=$2
EX_PATH=${PWD}
SRC_PATH=${PWD}/..

parameters_usage(){
    echo 'Usage: 1. <target>:           [ server, client, test ]'
    echo '       2. <prebuild_flag>:    [ y, n ]'
}

build_server(){
    if [ "${PREBUILD_FLAG}" == "y" ] ; then
        ./prebuild.sh server
    fi
    mkdir -p ../build/server
    cd ../build/server
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
    if [ "$1" == "oss" ] ; then
        cd ${EX_PATH}/../distributed_encoder/util
        thrift -r --gen cpp shared.thrift
        patch gen-cpp/shared_types.h Implement_operator_RegionInformation.patch
        cd -
        cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=server -DDE_FLAG=true -DUSE_SAFE_MEM_LIB=OFF ../..
    else
        sudo cp ../../ffmpeg/dependency/*.so /usr/local/lib/
        sudo cp ../../ffmpeg/dependency/*.pc /usr/local/lib/pkgconfig/
        sudo cp ../../ffmpeg/dependency/*.h /usr/local/include/
        sudo cp ../../ffmpeg/dependency/WorkerServer /root
        cmake -DCMAKE_BUILD_TYPE=Release -DTARGET=server ../..
    fi
    make -j $(nproc)
    sudo make install
    cp ../../ffmpeg/dependency/*.xml ffmpeg
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
    cp ../../player/app/linux/config.xml ./player/app
}

build_ci(){
    source /opt/rh/devtoolset-7/enable
    source /opt/rh/rh-ruby23/enable
    PREBUILD_FLAG="n"
    GIT_SHORT_HEAD=`git rev-parse --short HEAD`

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

    BASIC_CONFIG="-I${SRC_PATH}/google_test -std=c++11 -I../util/ "`
                `"-I${SRC_PATH}/utils -D_GLIBCXX_USE_CXX11_ABI=0 -g -c"
    SHARED_CONFIG="-L/usr/local/lib -I/usr/local/include/ "`
                 `"../googletest/googletest/build/libgtest.a "`
                 `"-lstdc++ -lpthread -lglog -lm -l360SCVP -lsafestring_shared "

    # Compile 360SCVP test
    cd build/test/360SCVP && \
        g++ ${BASIC_CONFIG} ${EX_PATH}/../360SCVP/test/testI360SCVP_common.cpp && \
        g++ ${BASIC_CONFIG} ${EX_PATH}/../360SCVP/test/testI360SCVP_erp.cpp && \
        g++ ${BASIC_CONFIG} ${EX_PATH}/../360SCVP/test/testI360SCVP_cubemap.cpp && \
        g++ testI360SCVP_common.o ${SHARED_CONFIG} -o testI360SCVP_common && \
        g++ testI360SCVP_erp.o ${SHARED_CONFIG} -o testI360SCVP_erp && \
        g++ testI360SCVP_cubemap.o ${SHARED_CONFIG} -o testI360SCVP_cubemap

    # Compile OmafDashAccess test
    DA_TEST_PATH="${SRC_PATH}/OmafDashAccess/test"
    DA_SHARED_CONFIG="${SHARED_CONFIG} -lOmafDashAccess"
    cd ../OmafDashAccess && \
        g++ ${BASIC_CONFIG} ${DA_TEST_PATH}/testMediaSource.cpp && \
        g++ ${BASIC_CONFIG} ${DA_TEST_PATH}/testMPDParser.cpp && \
        g++ ${BASIC_CONFIG} ${DA_TEST_PATH}/testOmafReader.cpp && \
        g++ ${BASIC_CONFIG} ${DA_TEST_PATH}/testOmafReaderManager.cpp && \
        g++ testMediaSource.o ${DA_SHARED_CONFIG} -o testMediaSource && \
        g++ testMPDParser.o ${DA_SHARED_CONFIG} -o testMPDParser && \
        g++ testOmafReader.o ${DA_SHARED_CONFIG} -o testOmafReader && \
        g++ testOmafReaderManager.o ${DA_SHARED_CONFIG} -o testOmafReaderManager

    # Compile VROmafPacking test
    OP_TEST_PATH="${SRC_PATH}/VROmafPacking/test"
    OP_VS_CONFIG="-I${SRC_PATH}/plugins/StreamProcess_Plugin/VideoStream_Plugin/common/ "`
                `"-I${SRC_PATH}/plugins/StreamProcess_Plugin/VideoStream_Plugin/HevcVideoStream/"
    OP_SHARED_CONFIG="${SHARED_CONFIG} -lVROmafPacking -lHevcVideoStreamProcess -ldl"
    cd ../VROmafPacking && \
        g++ ${OP_VS_CONFIG} ${BASIC_CONFIG} ${OP_TEST_PATH}/testHevcNaluParser.cpp && \
        g++ ${OP_VS_CONFIG} ${BASIC_CONFIG} ${OP_TEST_PATH}/testVideoStream.cpp && \
        g++ ${OP_VS_CONFIG} ${BASIC_CONFIG} ${OP_TEST_PATH}/testExtractorTrack.cpp && \
        g++ ${OP_VS_CONFIG} ${BASIC_CONFIG} ${OP_TEST_PATH}/testDefaultSegmentation.cpp && \
        g++ testHevcNaluParser.o ${OP_SHARED_CONFIG} -o testHevcNaluParser && \
        g++ testVideoStream.o ${OP_SHARED_CONFIG} -o testVideoStream && \
        g++ testExtractorTrack.o ${OP_SHARED_CONFIG} -o testExtractorTrack && \
        g++ testDefaultSegmentation.o ${OP_SHARED_CONFIG} -o testDefaultSegmentation

    if [ "$1" == "oss" ] ; then
        # Compile distributed_encoder test
        DE_TEST_PATH="${SRC_PATH}/distributed_encoder/test"
        DE_BASIC_CONFIG="${BASIC_CONFIG} -I/usr/local/include/svt-hevc "`
                       `"-I../../../distributed_encoder/util/"
        DE_SHARED_CONFIG="${SHARED_CONFIG} -lDistributedEncoder -lEncoder -pthread "`
                        `"-I/usr/local/include/thrift -I/usr/local/include/svt-hevc "`
                        `"-lthrift -lthriftnb -lSvtHevcEnc -lopenhevc -levent -lz "`
                        `"-lavutil -lavdevice -lavfilter -lavformat -lavcodec "`
                        `"-lswscale -lswresample -lXv -lX11 -llzma "`
                        `"-lXext -lxcb -lxcb-shm -lxcb-shape -lxcb-xfixes "
        cd ../distributed_encoder && \
            g++ ${DE_BASIC_CONFIG} ${DE_TEST_PATH}/testMainEncoder.cpp && \
            g++ ${DE_BASIC_CONFIG} ${DE_TEST_PATH}/testWorkSession.cpp && \
            g++ ${DE_BASIC_CONFIG} ${DE_TEST_PATH}/testDecoder.cpp && \
            g++ ${DE_BASIC_CONFIG} ${DE_TEST_PATH}/testEncoder.cpp && \
            g++ ${DE_BASIC_CONFIG} ${DE_TEST_PATH}/testSubEncoder.cpp && \
            g++ ${DE_BASIC_CONFIG} ${DE_TEST_PATH}/testSubEncoderManager.cpp && \
            g++ testMainEncoder.o ${DE_SHARED_CONFIG} -o testMainEncoder && \
            g++ testWorkSession.o ${DE_SHARED_CONFIG} -o testWorkSession && \
            g++ testDecoder.o ${DE_SHARED_CONFIG} -o testDecoder && \
            g++ testEncoder.o ${DE_SHARED_CONFIG} -o testEncoder && \
            g++ testSubEncoder.o ${DE_SHARED_CONFIG} -o testSubEncoder && \
            g++ testSubEncoderManager.o ${DE_SHARED_CONFIG} -o testSubEncoderManager
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
