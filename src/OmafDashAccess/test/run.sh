#!/bin/bash

# Download files
################################
curl -ukang1:AP2hWmPtk4RkLXUn2SMWfocyUq2 -O "https://ubit-artifactory-sh.intel.com/artifactory/npg_validation-sh-local/Immersive_Media_test/segs_for_readertest_0909.tar.gz" && tar zxvf segs_for_readertest_0909.tar.gz
#tar zxvf segs_for_readertest_0909.tar.gz
# Run test cases
################################

./testOmafReaderManager
if [ $? -ne 0 ]; then exit 1; fi

./testDownloaderPerf
if [ $? -ne 0 ]; then exit 1; fi

./testDownloader
if [ $? -ne 0 ]; then exit 1; fi

./testMediaSource --gtest_filter=*_static
if [ $? -ne 0 ]; then exit 1; fi
./testMediaSource --gtest_filter=*_live
if [ $? -ne 0 ]; then exit 1; fi
./testMediaSource --gtest_filter=*_static_withPredictor
if [ $? -ne 0 ]; then exit 1; fi
./testMediaSource --gtest_filter=*_live_withPredictor
if [ $? -ne 0 ]; then exit 1; fi
./testMediaSource --gtest_filter=*_static_changeViewport
if [ $? -ne 0 ]; then exit 1; fi
./testMediaSource --gtest_filter=*_live_changeViewport
if [ $? -ne 0 ]; then exit 1; fi
./testMPDParser
if [ $? -ne 0 ]; then exit 1; fi

curl -ukang1:AP2hWmPtk4RkLXUn2SMWfocyUq2 -O "https://ubit-artifactory-sh.intel.com/artifactory/npg_validation-sh-local/Immersive_Media_test/segs_for_readertest_0909.tar.gz" && tar zxvf segs_for_readertest_0909.tar.gz
#tar zxvf segs_for_readertest_0909.tar.gz

./testOmafReader
if [ $? -ne 0 ]; then exit 1; fi

# All caes passed
################################
rm -rf ./segs_for_readertest*
echo "All passed!"
