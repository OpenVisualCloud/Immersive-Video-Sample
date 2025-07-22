#!/bin/bash -ex

cd ../build/test

# 360SCVP test
################################
cd 360SCVP
cp ../../../360SCVP/test/*265 .

./testI360SCVP

cd -

# OmafDashAccess test
################################
cd OmafDashAccess
# please prepare dash segments for test

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

# distributed_encoder test
################################

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

cd -
