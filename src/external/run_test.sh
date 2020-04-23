#!/bin/bash -ex

REPO=$1
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64:$LD_LIBRARY_PATH
cd ../build/test

# 360SCVP test
################################
cd 360SCVP
cp ../../../360SCVP/test/*265 .

./testI360SCVP

cd -

# VROmafPacking test
################################
cd VROmafPacking
cp ../../../VROmafPacking/test/*265 .
cp ../../../VROmafPacking/test/*bin .

./testHevcNaluParser
./testVideoStream
./testExtractorTrack
./testDefaultSegmentation

cd -

destroy_worker()
{
    STATUS=$(lsof -i:9090 | tail -n 1 | awk {'print $NF'})

    while [ "${STATUS}" != "EMPTY" ]
        do
            if [ "${STATUS}" == "(LISTEN)" ] ; then
                echo "LISTEN"
                PID=$(lsof -i:9090 | tail -n 1 | awk {'print $2'})
                echo ${PID}
                if [ ! -z "${PID}" ] ; then
                    kill -9 ${PID}
                    STATUS="EMPTY"
                fi
                sleep 2s
            else
                echo "EMPTY"
                STATUS="EMPTY"
                fi
        done
}

if [ "${REPO}" = "oss" ] ; then

    # OmafDashAccess test
    ################################
    cd OmafDashAccess
    curl -H 'X-JFrog-Art-Api: AKCp5dL3Kxmp2PhDfYhT2oFk4SDxJji5H8S38oAqmMSkiD46Ho8uCA282aJJhM9ZqCKLb64bw' -O "https://ubit-artifactory-sh.intel.com/artifactory/immersive_media-sh-local/testfile/segs_for_readertest_0909.tar.gz" && tar zxf segs_for_readertest_0909.tar.gz

    ./testMediaSource --gtest_filter=*_static
    ./testMediaSource --gtest_filter=*_live
    ./testMediaSource --gtest_filter=*_static_withPredictor
    ./testMediaSource --gtest_filter=*_live_withPredictor
    ./testMediaSource --gtest_filter=*_static_changeViewport
    ./testMediaSource --gtest_filter=*_live_changeViewport
    ./testMPDParser
    ./testOmafReader
    ./testOmafReaderManager

    rm -rf ./segs_for_readertest*

    cd -

    # distributed_encoder test
    ################################
    sudo yum install lsof -y

    cd distributed_encoder
    cp ../../../distributed_encoder/test/*265 .
    cp ../../../distributed_encoder/test/*264 .
    cp ../../../distributed_encoder/test/*yuv .
    cp ../../../distributed_encoder/test/*bin .
    cp ../../../distributed_encoder/test/*txt .

    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH
    export LD_LIBRARY_PATH=/usr/local/lib64:$LD_LIBRARY_PATH

    destroy_worker
    ./testMainEncoder
    destroy_worker
    ./testWorkSession
    destroy_worker
    ./testDecoder
    destroy_worker
    ./testSubEncoder
    destroy_worker
    ./testEncoder
    # ./testSubEncoderManager
fi

cd -
