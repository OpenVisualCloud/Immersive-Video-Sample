#!/bin/bash -ex

SCRIPT=`pwd`/$0
PATH_NAME=`dirname ${SCRIPT}`
BUILD_DIR=${PATH_NAME}/../
BASE_NAME="owt-base:5.0"
IMAGE_CASE=$1
IMAGE_NAME=$2

usage() {
    echo -e "Usage:     $0 [OPTION] [NAME]\n"
    echo -e "OPTION:    Build from <scratch> / <binary> / <ovc>"
    echo -e "           e.g. scratch"
    echo -e "NAME:      Required image name"
    echo -e "           e.g. webrtc360:latest"
    exit 1
}

start_build_process() {
    cd ${BUILD_DIR}
    docker build -t $1 \
        --build-arg http_proxy=$http_proxy \
        --build-arg https_proxy=$https_proxy \
        -f ${PATH_NAME}/${IMAGE_CASE}/Dockerfile .
}

build_from_binary() {
    IMAGE_CASE="from_binary"
    if [ ! -d "${BUILD_DIR}/dist" ]; then
        echo "missing dir: WebRTC360/dist"
        exit 1
    fi
    
    if [ ! -d "${BUILD_DIR}/build" ]; then
        echo "missing dir: WebRTC360/build"
        exit 1
    fi

    start_build_process ${IMAGE_NAME}
}

build_from_scratch() {
    IMAGE_CASE="from_scratch"
    start_build_process ${IMAGE_NAME}
}

build_server_base() {
    IMAGE_CASE="owt_server_base"
    BASE_INFO="`docker images | grep owt-base || true`"
    if [ -z "${BASE_INFO}" ] ; then
        echo "Start building owt-server base image"
        start_build_process "${BASE_NAME}"
    else
        BASE_IMAGE_NAME=`echo "${BASE_INFO}" | awk -F ' ' '{print $1}'`
        BASE_IMAGE_TAG=`echo "${BASE_INFO}" | awk -F ' ' '{print $2}'`
        echo "owt-server exists: ${BASE_IMAGE_NAME}:${BASE_IMAGE_TAG}"
    fi
}

build_from_base() {
    IMAGE_CASE="from_base"
    start_build_process ${IMAGE_NAME}
}

if [ $# != 2 ]; then
    usage $*
else
    cp ${BUILD_DIR}/../../Sample-Videos/test1_h265_3840x2048_30fps_30M_200frames.mp4 .
    if [ ${IMAGE_CASE} == "binary" ] ; then
        build_from_binary
    elif [ ${IMAGE_CASE} == "scratch" ] ; then
        build_from_scratch
    elif [ ${IMAGE_CASE} == "base" ] ; then
        build_server_base
        build_from_base
    else
        usage $*
    fi
    rm -f ./test1_h265_3840x2048_30fps_30M_200frames.mp4
fi

