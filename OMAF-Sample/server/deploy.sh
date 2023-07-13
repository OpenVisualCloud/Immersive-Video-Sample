#!/bin/bash -ex

parameters_usage() {
	echo 'Usage: 1. <path>:   original file path'
	echo '       2. <proxy>:  proxy setting. [optional]'
}

REPOPATH=$(echo $1 | awk -F "OMAF-Sample" '{print $1}')
SRCPATH="${REPOPATH}src/"
DSTPATH="${REPOPATH}OMAF-Sample/server/src/"
VERSION="v1.12"
IMAGEPREFIX="immersive-server"
BASETAG="${IMAGEPREFIX}-base:${VERSION}"
RUNTIMETAG="${IMAGEPREFIX}:${VERSION}"

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

if [ $# = 2 ]; then
	if [ "$1" = "-h" ]; then
		parameters_usage
	else
		PROXY=$2
		PROXYARGS="--build-arg http_proxy=${PROXY} "$(
		)"--build-arg https_proxy=${PROXY}"
		echo "PROXY:${PROXY}"
	fi
elif [ $# = 1 ]; then
	PROXYARGS=""
else
	parameters_usage
	exit 0
fi

DOCKER_BUILDKIT=1 docker build ${PROXYARGS} \
	--tag ${BASETAG} \
	--file Dockerfile.base .

DOCKER_BUILDKIT=1 docker build ${PROXYARGS} \
	--tag ${RUNTIMETAG} \
	--build-arg "base_image=${BASETAG}" \
	--file Dockerfile.runtime .

rm -rf ${DSTPATH}
