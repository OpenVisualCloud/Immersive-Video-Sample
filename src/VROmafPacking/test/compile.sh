#!/bin/bash -e

cp ../../google_test/libgtest.a .

mkdir vs_plugin
cd vs_plugin
cp ../../../plugins/StreamProcess_Plugin/VideoStream_Plugin/common/NaluParser.h ./
cp ../../../plugins/StreamProcess_Plugin/VideoStream_Plugin/HevcVideoStream/HevcNaluParser.h ./
cp ../../../utils/VROmafPacking_def.h ./
cp ../../../utils/OmafPackingLog.h ./
cd ../

g++ -I../ -I./vs_plugin -I../../isolib -I../../google_test/ -std=c++11 -g -c testHevcNaluParser.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../ -I./vs_plugin -I../../isolib -I../../google_test/ -std=c++11 -g -c testVideoStream.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../ -I./vs_plugin -I../../isolib -I../../google_test/ -std=c++11 -g -c testExtractorTrack.cpp -D_GLIBCXX_USE_CXX11_ABI=0
g++ -I../ -I./vs_plugin -I../../isolib -I../../google_test/ -std=c++11 -g -c testDefaultSegmentation.cpp -D_GLIBCXX_USE_CXX11_ABI=0

LD_FLAGS="-L/usr/local/lib -lVROmafPacking -l360SCVP -lHevcVideoStreamProcess -ldl -lstdc++ -lpthread -lm -L/usr/local/lib"

g++ -L/usr/local/lib testHevcNaluParser.o libgtest.a -o testHevcNaluParser ${LD_FLAGS}
g++ -L/usr/local/lib testVideoStream.o libgtest.a -o testVideoStream ${LD_FLAGS}
g++ -L/usr/local/lib testExtractorTrack.o libgtest.a -o testExtractorTrack ${LD_FLAGS}
g++ -L/usr/local/lib testDefaultSegmentation.o libgtest.a -o testDefaultSegmentation ${LD_FLAGS}

./testHevcNaluParser
./testVideoStream
./testExtractorTrack
./testDefaultSegmentation

rm -rf vs_plugin
