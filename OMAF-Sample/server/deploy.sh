#!/bin/bash -ex

PROXY=$1

parameters_usage(){
    echo 'Usage: 1. <proxy>:  proxy setting. [optional]'
}

mkdir -p src
cd ../..
cp -r src/360SCVP OMAF-Sample/server/src
cp -r src/external OMAF-Sample/server/src
cp -r src/ffmpeg OMAF-Sample/server/src
cp -r src/player OMAF-Sample/server/src
cp -r src/utils OMAF-Sample/server/src
cp -r src/VROmafPacking OMAF-Sample/server/src
cp -r src/CMakeLists.txt OMAF-Sample/server/src
cp -r Sample-Videos OMAF-Sample/server/src
mkdir -p OMAF-Sample/server/src/OmafDashAccess
ls src/OmafDashAccess/ | grep -v mp4lib | xargs -i cp -r src/OmafDashAccess/{} OMAF-Sample/server/src/OmafDashAccess
cd OMAF-Sample/server

if [ $# = 0 ] ; then
    docker build -t immersive_server:v0.1 .
elif [ $# = 1 ] ; then
    if [ "$1" = "-h" ] ; then
        parameters_usage
    else
        PROXY=$1
        docker build -t immersive_server:v0.1 --build-arg HTTP_PROXY=${PROXY} --build-arg HTTPS_PROXY=${PROXY} .
        echo 'case 1'
        echo "PROXY:${PROXY}"
    fi
else
    parameters_usage
    exit 0
fi

rm -rf src
