#!/bin/bash -ex

ORIPATH=$(pwd)
ITEM=$1
VERSION=$2
PACKAGE=package
LIBDIR=${ORIPATH}/../build/${PACKAGE}/${ITEM}/usr/lib64/webrtc-${ITEM}/
BINDIR=${ORIPATH}/../build/${PACKAGE}/${ITEM}/usr/bin/webrtc-${ITEM}/
NAME=$(echo "webrtc-${item}")

parameters_usage(){
    echo 'Usage: 1. <item>:           [ server, client ]'
    echo '       2. <version>:        Version of current package.'
}

package(){
    if [ ${ITEM} = "server" ] ; then
        # echo 'sudo cp /usr/lib64/immersive-server/libSingleVideoPacking.so /usr/local/lib' >> post
        echo 'sudo ldconfig' > post
    elif [ ${ITEM} = "client" ] ; then
        echo 'sudo ldconfig' > post
    fi
    fpm \
        -f \
        -s dir \
        -t $1 \
        -n webrtc-$2$3 \
        -v 1${VERSION} \
        --iteration 1.el7 \
        -C ${PACKAGE}/$2 \
        -p ${PACKAGE} \
        --after-install post
    rm -rf ./post
}

if [ "${ITEM}" = "-h" ] || [ $# != 2 ] ; then
    parameters_usage
    exit 0
fi
if [ "${ITEM}" != "server" ] && [ "${ITEM}" != "client" ] ; then
    parameters_usage
    exit 0
fi

mkdir -p ${LIBDIR}
mkdir -p ${BINDIR}

if [ ${ITEM} = "client" ] ; then
    git log | head -n 3 > git_info
    SDK_PATH="webrtc_linux_client_sdk/release"
    SDK_LIB_PATH="webrtc_linux_client_sdk/release/lib"
    cp ${SDK_LIB_PATH}/libMediaPlayer.so                              ${LIBDIR}
    cp ${SDK_LIB_PATH}/libssl.so.1.1                                  ${LIBDIR}
    cp ${SDK_LIB_PATH}/libcrypto.so.1.1                               ${LIBDIR}
    cp ${SDK_LIB_PATH}/libavcodec.so.58                               ${LIBDIR}
    cp ${SDK_LIB_PATH}/libavutil.so.56                                ${LIBDIR}
    cp ${SDK_LIB_PATH}/libswresample.so.3                             ${LIBDIR}
    cp ${SDK_LIB_PATH}/lib360SCVP.so                                  ${LIBDIR}
    cp /usr/local/lib64/libglog.so.0                                  ${LIBDIR}
    cp /usr/lib64/libSDL2-2.0.so.0                                    ${LIBDIR}
    cp player/player_app_build/render                                 ${BINDIR}
    cp player/config.xml                                              ${BINDIR}
    strip ${LIBDIR}/*
    strip ${BINDIR}/render
    package rpm ${ITEM}
    package deb ${ITEM}
fi
