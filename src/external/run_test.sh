#!/bin/bash -ex

REPO=$1
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64:$LD_LIBRARY_PATH
cd ../build/test

# 360SCVP test
################################
cd 360SCVP
cp ../../../360SCVP/test/*265 .

./testI360SCVP_common
./testI360SCVP_erp
./testI360SCVP_cubemap

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
    PID=$(pidof WorkerServer_9090) || true
    echo ${PID}
    if [ ! -z "${PID}" ] ; then
        kill -9 ${PID}
    fi
    sleep 2s
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
    # local dash file will be deleted after release.
    curl -H 'X-JFrog-Art-Api: AKCp5dL3Kxmp2PhDfYhT2oFk4SDxJji5H8S38oAqmMSkiD46Ho8uCA282aJJhM9ZqCKLb64bw' -O "https://ubit-artifactory-sh.intel.com/artifactory/immersive_media-sh-local/testfile/segs_for_readertest_0909.tar.gz" && tar zxf segs_for_readertest_0909.tar.gz
    ./testMPDParser
    curl -H 'X-JFrog-Art-Api: AKCp5dL3Kxmp2PhDfYhT2oFk4SDxJji5H8S38oAqmMSkiD46Ho8uCA282aJJhM9ZqCKLb64bw' -O "https://ubit-artifactory-sh.intel.com/artifactory/immersive_media-sh-local/testfile/segs_for_readertest_0909.tar.gz" && tar zxf segs_for_readertest_0909.tar.gz
    ./testOmafReader
    curl -H 'X-JFrog-Art-Api: AKCp5dL3Kxmp2PhDfYhT2oFk4SDxJji5H8S38oAqmMSkiD46Ho8uCA282aJJhM9ZqCKLb64bw' -O "https://ubit-artifactory-sh.intel.com/artifactory/immersive_media-sh-local/testfile/segs_for_readertest_0909.tar.gz" && tar zxf segs_for_readertest_0909.tar.gz
    ./testOmafReaderManager

    rm -rf ./segs_for_readertest*

    cd -

    # distributed_encoder test
    ################################

    cd distributed_encoder
    cp ../../../distributed_encoder/test/*265 .
    cp ../../../distributed_encoder/test/*264 .
    cp ../../../distributed_encoder/test/*yuv .
    cp ../../../distributed_encoder/test/*bin .

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
