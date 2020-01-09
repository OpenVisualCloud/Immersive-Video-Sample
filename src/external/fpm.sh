#!/bin/bash -x

ORIPATH=$(pwd)
VERSION=$1
PACKAGE=package
LIBDIR=${ORIPATH}/../../OMAF-Sample/client/${PACKAGE}/files/usr/lib64/immersive-client/
BINDIR=${ORIPATH}/../../OMAF-Sample/client/${PACKAGE}/files/usr/bin/immersive-client/

parameters_usage(){
    echo 'Usage: 1. <version>:        Version of current package.'
}

program_exists() {
    local RET='0'
    command -v $1  >/dev/null 2>&1 || { local RET='1'; }
    # fail on non-zero return value
    if [ ${RET} -ne 0 ]; then
        return 1
    fi

    return 0
}

package(){
    echo 'sudo ldconfig' > post
    fpm \
        -f \
        -s dir \
        -t $1 \
        -n immersive-client \
        -v ${VERSION} \
        --iteration 1.el7 \
        -C ${PACKAGE}/files \
        -p ${PACKAGE} \
        --after-install post
    rm -rf ./post
}

if [ "${VERSION}" = "-h" ] || [ $# != 1 ] ; then
    parameters_usage
    exit 0
fi

program_exists fpm
if [ $? != 0 ];then
    sudo apt-get -y install ruby rubygems ruby-dev
    sudo gem install fpm
fi

mkdir -p ${LIBDIR}
mkdir -p ${BINDIR}

git log | head -n 3 > git_info
cd ../build
cp external/MediaServerStudioEssentialsKBL2019R1HF1_10010/intel-linux-media-kbl-10010/opt/intel/mediasdk/lib64/libva-drm.so.2 ${LIBDIR}
cp external/MediaServerStudioEssentialsKBL2019R1HF1_10010/intel-linux-media-kbl-10010/opt/intel/mediasdk/lib64/libva-x11.so.2 ${LIBDIR}
cp external/MediaServerStudioEssentialsKBL2019R1HF1_10010/intel-linux-media-kbl-10010/opt/intel/mediasdk/lib64/libva.so.2     ${LIBDIR}
cp external/glog/.libs/libglog.so.0                                 ${LIBDIR}
cp client/360SCVP/lib360SCVP.so                                     ${LIBDIR}
cp client/OmafDashAccess/libOmafDashAccess.so                       ${LIBDIR}
cp client/player/render                                             ${BINDIR}
cp ../player/config.xml                                             ${BINDIR}
mv ../external/git_info                                             ${BINDIR}
cd ../../OMAF-Sample/client
strip ${LIBDIR}/*
strip ${BINDIR}/render
package rpm
package deb
