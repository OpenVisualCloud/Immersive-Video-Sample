#!/bin/bash -ex

export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64:$LD_LIBRARY_PATH
cd ../build/test

# 360SCVP test
################################
cd 360SCVP
cp ../../../360SCVP/test/*265 .

./testI360SCVP

cd -

# # OmafDashAccess test
# ################################
# cd OmafDashAccess
# 
# ./testMediaSource --gtest_filter=*_static
# ./testMediaSource --gtest_filter=*_live
# ./testMediaSource --gtest_filter=*_static_withPredictor
# ./testMediaSource --gtest_filter=*_live_withPredictor
# ./testMediaSource --gtest_filter=*_static_changeViewport
# ./testMediaSource --gtest_filter=*_live_changeViewport
# ./testMPDParser
# ./testOmafReader
# ./testOmafReaderManager
# 
# rm -rf ./segs_for_readertest*
# 
# cd -

# VROmafPacking test
################################
cd VROmafPacking
cp ../../../VROmafPacking/test/*265 .
cp ../../../VROmafPacking/test/*bin .

./testHevcNaluParser
./testVideoStream
./testExtractorTrack
./testDefaultSegmentation
