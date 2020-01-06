#!/bin/bash -ex
mkdir -p src
cd ../..
cp -r src/360SCVP OMAF-Sample/server/src
cp -r src/external OMAF-Sample/server/src
cp -r src/ffmpeg OMAF-Sample/server/src
cp -r src/OmafDashAccess OMAF-Sample/server/src
cp -r src/player OMAF-Sample/server/src
cp -r src/utils OMAF-Sample/server/src
cp -r src/VROmafPacking OMAF-Sample/server/src
cp -r src/CMakeLists.txt OMAF-Sample/server/src
cp -r Sample-Videos OMAF-Sample/server/src
cd OMAF-Sample/server
docker build -t immersive_server:v0.1 .
rm -rf src
