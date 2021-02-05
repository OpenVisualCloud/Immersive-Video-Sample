#!/bin/bash -ex

parameters_usage(){
    echo 'Usage: 1. <path>:   original file path'
    echo '       2. <proxy>:  proxy setting. [optional]'
}

REPOPATH=`echo $1 | awk -F "OMAF-Sample" '{print $1}'`
SRCPATH="${REPOPATH}src/"
DSTPATH="${REPOPATH}OMAF-Sample/server/src/"

mkdir -p ${DSTPATH}
cd ${DSTPATH}..

cp -r ${SRCPATH}360SCVP ${DSTPATH}
cp -r ${SRCPATH}external ${DSTPATH}
cp -r ${SRCPATH}ffmpeg ${DSTPATH}
cp -r ${SRCPATH}player ${DSTPATH}
cp -r ${SRCPATH}utils ${DSTPATH}
cp -r ${SRCPATH}isolib ${DSTPATH}
cp -r ${SRCPATH}trace ${DSTPATH}
cp -r ${SRCPATH}plugins ${DSTPATH}
cp -r ${SRCPATH}VROmafPacking ${DSTPATH}
cp -r ${SRCPATH}OmafDashAccess ${DSTPATH}
cp -r ${SRCPATH}CMakeLists.txt ${DSTPATH}
cp -r ${REPOPATH}Sample-Videos ${DSTPATH}

if [ $# = 1 ] ; then
    docker build -t immersive_server:v1.4 .
elif [ $# = 2 ] ; then
    if [ "$1" = "-h" ] ; then
        parameters_usage
    else
        PROXY=$2
        docker build -t immersive_server:v1.4 \
            --build-arg http_proxy=${PROXY} \
            --build-arg https_proxy=${PROXY} .
        echo "PROXY:${PROXY}"
    fi
else
    parameters_usage
    exit 0
fi

rm -rf ${DSTPATH}
