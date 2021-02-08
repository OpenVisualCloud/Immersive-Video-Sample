
FROM centos:7.6.1810 AS build
WORKDIR /home
SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# COMMON BUILD TOOLS
RUN yum install -y -q bzip2 make autoconf libtool git wget ca-certificates pkg-config gcc gcc-c++ bison flex patch epel-release yum-devel libcurl-devel zlib-devel;

# Install cmake
ARG CMAKE_VER=3.13.1
ARG CMAKE_REPO=https://cmake.org/files
RUN wget -O - ${CMAKE_REPO}/v${CMAKE_VER%.*}/cmake-${CMAKE_VER}.tar.gz | tar xz && \
    cd cmake-${CMAKE_VER} && \
    ./bootstrap --prefix="/usr/local" --system-curl && \
    make -j8 && \
    make install

# Install automake, use version 1.14 on CentOS
ARG AUTOMAKE_VER=1.14
ARG AUTOMAKE_REPO=https://ftp.gnu.org/pub/gnu/automake/automake-${AUTOMAKE_VER}.tar.xz
RUN wget -O - ${AUTOMAKE_REPO} | tar xJ && \
    cd automake-${AUTOMAKE_VER} && \
    ./configure --prefix=/usr --libdir=/usr/local/lib64 --disable-doc && \
    make -j8 && \
    make install

# Build NASM
ARG NASM_VER=2.13.03
ARG NASM_REPO=https://www.nasm.us/pub/nasm/releasebuilds/${NASM_VER}/nasm-${NASM_VER}.tar.bz2
RUN  wget ${NASM_REPO} && \
     tar -xaf nasm* && \
     cd nasm-${NASM_VER} && \
     ./autogen.sh && \
     ./configure --prefix="/usr/local" --libdir=/usr/local/lib64 && \
     make -j8 && \
     make install

# Build YASM
ARG YASM_VER=1.3.0
ARG YASM_REPO=https://www.tortall.net/projects/yasm/releases/yasm-${YASM_VER}.tar.gz
RUN  wget -O - ${YASM_REPO} | tar xz && \
     cd yasm-${YASM_VER} && \
     sed -i "s/) ytasm.*/)/" Makefile.in && \
     ./configure --prefix="/usr/local" --libdir=/usr/local/lib64 && \
     make -j8 && \
     make install

# Build libnice
ARG NICE_VER="0.1.4"
ARG NICE_REPO=http://nice.freedesktop.org/releases/libnice-${NICE_VER}.tar.gz
ARG LIBNICE_PATCH_VER="4.3.1"
ARG LIBNICE_PATCH_REPO=https://github.com/open-webrtc-toolkit/owt-server/archive/v${LIBNICE_PATCH_VER}.tar.gz

RUN yum install -y -q glib2-devel

RUN wget -O - ${NICE_REPO} | tar xz && \
    cd libnice-${NICE_VER} && \
    wget -O - ${LIBNICE_PATCH_REPO} | tar xz  && \
    patch -p1 < owt-server-${LIBNICE_PATCH_VER}/scripts/patches/libnice014-agentlock.patch && \
    patch -p1 < owt-server-${LIBNICE_PATCH_VER}/scripts/patches/libnice014-agentlock-plus.patch && \
    patch -p1 < owt-server-${LIBNICE_PATCH_VER}/scripts/patches/libnice014-removecandidate.patch && \
    patch -p1 < owt-server-${LIBNICE_PATCH_VER}/scripts/patches/libnice014-keepalive.patch && \
    patch -p1 < owt-server-${LIBNICE_PATCH_VER}/scripts/patches/libnice014-startcheck.patch && \
    patch -p1 < owt-server-${LIBNICE_PATCH_VER}/scripts/patches/libnice014-closelock.patch && \
    ./configure --prefix="/usr/local" --libdir=/usr/local/lib64 && \
    make -s V= && \
    make install


# Build open ssl
ARG OPENSSL_VER="1.1.1h"
ARG OPENSSL_REPO=http://www.openssl.org/source/openssl-${OPENSSL_VER}.tar.gz
ARG BUILD_PREFIX=/usr/local/ssl
ARG BUILD_DESTDIR=/home/build

RUN wget -O - ${OPENSSL_REPO} | tar xz && \
    cd openssl-${OPENSSL_VER} && \
    ./config no-ssl3 --prefix=${BUILD_PREFIX} --openssldir=${BUILD_PREFIX} -Wl,-rpath=${BUILD_PREFIX}/lib -fPIC && \
    make depend && \
    make -s V=0  && \
    make install

# Build libre
ARG LIBRE_VER="v0.5.0"
ARG LIBRE_REPO=https://github.com/creytiv/re.git

RUN git clone ${LIBRE_REPO} && \
    cd re && \
    git checkout ${LIBRE_VER} && \
    make SYSROOT_ALT="/usr" RELEASE=1 && \
    make install SYSROOT_ALT="/usr" RELEASE=1 PREFIX="/usr"

# Build usrsctp

ARG USRSCTP_VERSION="30d7f1bd0b58499e1e1f2415e84d76d951665dc8"
ARG USRSCTP_FILE="${USRSCTP_VERSION}.tar.gz"
ARG USRSCTP_EXTRACT="usrsctp-${USRSCTP_VERSION}"
ARG USRSCTP_URL="https://github.com/sctplab/usrsctp/archive/${USRSCTP_FILE}"

RUN yum install -y -q which

RUN wget -O - ${USRSCTP_URL} | tar xz && \
    mv ${USRSCTP_EXTRACT} usrsctp && \
    cd usrsctp && \
    ./bootstrap && \
    ./configure --prefix="/usr/local" --libdir=/usr/local/lib64 && \
    make && \
    make install

# Build libsrtp2
ARG SRTP2_VER="2.1.0"
ARG SRTP2_REPO=https://codeload.github.com/cisco/libsrtp/tar.gz/v${SRTP2_VER}

RUN yum install -y -q curl

RUN curl -o libsrtp-${SRTP2_VER}.tar.gz ${SRTP2_REPO} && \
    tar xzf libsrtp-${SRTP2_VER}.tar.gz && \
    cd libsrtp-${SRTP2_VER} && \
    export PKG_CONFIG_PATH="/usr/local/lib64/pkgconfig" && \
    export CFLAGS="-fPIC" && \
    ./configure --enable-openssl --prefix="/usr/local" --with-openssl-dir="/usr/local/ssl/" && \
    make -s V=0  && \
    make install

# Build fdk-aac
ARG FDK_AAC_VER=v0.1.6
ARG FDK_AAC_REPO=https://github.com/mstorsjo/fdk-aac/archive/${FDK_AAC_VER}.tar.gz

RUN wget -O - ${FDK_AAC_REPO} | tar xz && mv fdk-aac-${FDK_AAC_VER#v} fdk-aac && \
    cd fdk-aac && \
    autoreconf -fiv && \
    ./configure --prefix="/usr/local" --libdir=/usr/local/lib64 --enable-shared && \
    make -j8 && \
    make install DESTDIR=/home/build && \
    make install


# Fetch FFmpeg source
ARG FFMPEG_VER=n4.1
ARG FFMPEG_REPO=https://github.com/FFmpeg/FFmpeg/archive/${FFMPEG_VER}.tar.gz
ARG FFMPEG_1TN_PATCH_REPO=https://patchwork.ffmpeg.org/patch/11625/raw
ARG FFMPEG_THREAD_PATCH_REPO=https://patchwork.ffmpeg.org/patch/11035/raw

ARG FFMPEG_PATCHES_RELEASE_VER=0.1
ARG FFMPEG_PATCHES_RELEASE_URL=https://github.com/VCDP/CDN/archive/v${FFMPEG_PATCHES_RELEASE_VER}.tar.gz
ARG FFMPEG_PATCHES_PATH=/home/CDN-${FFMPEG_PATCHES_RELEASE_VER}
RUN wget -O - ${FFMPEG_PATCHES_RELEASE_URL} | tar xz


RUN yum install -y -q libass-devel freetype-devel zlib-devel openssl-devel

RUN wget -O - ${FFMPEG_REPO} | tar xz && mv FFmpeg-${FFMPEG_VER} FFmpeg && \
    cd FFmpeg ;

# Compile FFmpeg
RUN cd /home/FFmpeg && \
    export PKG_CONFIG_PATH="/usr/local/lib64/pkgconfig" && \
    ./configure --prefix="/usr/local" --libdir=/usr/local/lib64 --enable-shared --disable-static --disable-libvpx --disable-vaapi --enable-libfreetype --enable-libfdk-aac  --enable-nonfree && \

    make -j8 && \
    make install && make install DESTDIR="/home/build"


# Install node
ARG NODE_VER=v10.21.0
ARG NODE_REPO=https://nodejs.org/dist/${NODE_VER}/node-${NODE_VER}-linux-x64.tar.xz

RUN yum install -y -q ca-certificates wget xz-utils

RUN wget ${NODE_REPO} && \
    tar xf node-${NODE_VER}-linux-x64.tar.xz && \
    cp node-*/* /usr/local -rf && \
    rm -rf node-*

# Fetch SVT-HEVC
ARG SVT_HEVC_VER=v1.4.3
ARG SVT_HEVC_REPO=https://github.com/intel/SVT-HEVC

RUN yum install -y -q patch centos-release-scl && \
    yum install -y -q devtoolset-7

# hadolint ignore=SC1091
RUN git clone ${SVT_HEVC_REPO} && \
    cd SVT-HEVC/Build/linux && \
    export PKG_CONFIG_PATH="/usr/local/lib64/pkgconfig" && \
    git checkout ${SVT_HEVC_VER} && \
    mkdir -p ../../Bin/Release && \
    ( source /opt/rh/devtoolset-7/enable && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib64 -DCMAKE_ASM_NASM_COMPILER=yasm ../.. && \
    make -j8 && \
    make install DESTDIR=/home/build && \
    make install )


# Build OWT specific modules

ARG OWTSERVER_COMMIT=fd71357d6fdbd57d3c4be2028976bc2b34fff781
ARG OWTSERVER_REPO=https://github.com/open-webrtc-toolkit/owt-server.git
ARG OPENH264_MAJOR=1
ARG OPENH264_MINOR=7
ARG OPENH264_SOVER=4
ARG OPENH264_SOURCENAME=v${OPENH264_MAJOR}.${OPENH264_MINOR}.0.tar.gz
ARG OPENH264_SOURCE=https://github.com/cisco/openh264/archive/v${OPENH264_MAJOR}.${OPENH264_MINOR}.0.tar.gz
ARG OPENH264_BINARYNAME=libopenh264-${OPENH264_MAJOR}.${OPENH264_MINOR}.0-linux64.${OPENH264_SOVER}.so
ARG OPENH264_BINARY=https://github.com/cisco/openh264/releases/download/v${OPENH264_MAJOR}.${OPENH264_MINOR}.0/${OPENH264_BINARYNAME}.bz2
ARG LICODE_COMMIT="8b4692c88f1fc24dedad66b4f40b1f3d804b50ca"
ARG LICODE_REPO=https://github.com/lynckia/licode.git
ARG LICODE_PATCH_REPO=https://github.com/open-webrtc-toolkit/owt-server/tree/master/scripts/patches/licode/
ARG SAFESTRINGLIB_COMMIT="245c4b8cff1d2e7338b7f3a82828fc8e72b29549"
ARG SAFESTRINGLIB_REPO=https://github.com/intel/safestringlib.git
ARG SCVP_VER="9ce286edf4d5976802bf488b4dd90a16ecc28c36"
ARG SCVP_REPO=https://github.com/OpenVisualCloud/Immersive-Video-Sample
ARG WEBRTC_REPO=https://github.com/open-webrtc-toolkit/owt-deps-webrtc.git
ARG SERVER_PATH=/home/owt-server
ARG OWT_SDK_REPO=https://github.com/open-webrtc-toolkit/owt-client-javascript.git
ARG OWT_BRANCH=360-video
ARG OWT_BRANCH_JS=master
ARG OWT_BRANCH_JS_COMMIT="d727af2927731ff16214d73f57964a992258636d"
ARG WEBRTC_COMMIT="c2aa290cfe4f63d5bfbb6540122a5e6bf2783187"

ARG FDKAAC_LIB=/home/build/usr/local/lib64
RUN yum install -y -q python-devel glib2-devel boost-devel log4cxx-devel glog-devel gflags-devel
RUN yum install -y -q patch centos-release-scl devtoolset-7
ENV PYTHONIOENCODING=UTF-8
# Install 360scvp
# hadolint ignore=SC1091
RUN cd /home && \
    source /opt/rh/devtoolset-7/enable && \
    git clone ${SAFESTRINGLIB_REPO} && \
    cd safestringlib && git reset --hard ${SAFESTRINGLIB_COMMIT} && \
    mkdir build && cd build && cmake .. && \
    make -j && \
    mkdir -p /usr/local/lib && \
    cp libsafestring_shared.so /usr/local/lib && \
    mkdir -p /usr/local/lib64 && \
    cp libsafestring_shared.so /usr/local/lib64 && \
    mkdir -p /home/build/usr/local/lib64 && \
    cp libsafestring_shared.so /home/build/usr/local/lib64 && \
    mkdir -p /usr/local/include/safestringlib && \
    cp -rf ../include/* /usr/local/include/safestringlib/
RUN cd /home && \
    git clone ${SCVP_REPO} && \
    cd Immersive-Video-Sample/src/360SCVP && \
    git reset --hard ${SCVP_VER} && \
    mkdir build && \
    cd build && \
    source /opt/rh/devtoolset-7/enable && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_INSTALL_LIBDIR=lib64 ../ && \
    make -j && \
    make install DESTDIR=/home/build && \
    make install

# 1. Clone OWT server source code
# 2. Clone licode source code and patch
# 3. Clone webrtc source code and patch

# hadolint ignore=SC1091
RUN git clone -b ${OWT_BRANCH} ${OWTSERVER_REPO} && \
    source /opt/rh/devtoolset-7/enable && \

    cd ${SERVER_PATH} && git reset --hard ${OWTSERVER_COMMIT} && \
    curl https://patch-diff.githubusercontent.com/raw/open-webrtc-toolkit/owt-server/pull/708.patch | git apply && \

    # Install node modules for owt
    npm config set proxy=${http_proxy} && \
    npm config set https-proxy=${http_proxy} && \
    npm install -g --loglevel error node-gyp@v6.1.0 grunt-cli underscore jsdoc && \
    cd ${SERVER_PATH} && npm install nan && \

    # Get openh264 for owt
    cd ${SERVER_PATH}/third_party && \
    mkdir openh264 && cd openh264 && \
    wget ${OPENH264_SOURCE} && \
    wget ${OPENH264_BINARY} && \
    tar xzf ${OPENH264_SOURCENAME} openh264-${OPENH264_MAJOR}.${OPENH264_MINOR}.0/codec/api && \
    ln -s -v openh264-${OPENH264_MAJOR}.${OPENH264_MINOR}.0/codec codec && \
    bzip2 -d ${OPENH264_BINARYNAME}.bz2 && \
    ln -s -v ${OPENH264_BINARYNAME} libopenh264.so.${OPENH264_SOVER} && \
    ln -s -v libopenh264.so.${OPENH264_SOVER} libopenh264.so && \
    echo 'const char* stub() {return "this is a stub lib";}' > pseudo-openh264.cpp && \
    gcc pseudo-openh264.cpp -fPIC -shared -o pseudo-openh264.so && \

    # Get licode for owt
    cd ${SERVER_PATH}/third_party && git clone ${LICODE_REPO} && \
    cd licode && \
    git reset --hard ${LICODE_COMMIT} && \
    wget -r -nH --cut-dirs=5 --no-parent ${LICODE_PATCH_REPO} && \
    git apply ${SERVER_PATH}/scripts/patches/licode/*.patch && \
    mkdir -p ${SERVER_PATH}/build/libdeps/build/include && \
    cp erizoAPI/lib/json.hpp ${SERVER_PATH}/build/libdeps/build/include && \

    # Install webrtc for owt
    cd ${SERVER_PATH}/third_party && mkdir webrtc  && cd webrtc &&\
    export GIT_SSL_NO_VERIFY=1 && \
    git clone -b 59-server ${WEBRTC_REPO} src && cd src && \
    git reset --hard ${WEBRTC_COMMIT} && \
    ./tools-woogeen/install.sh && \
    patch -p1 < ${SERVER_PATH}/scripts/patches/0001-Implement-RtcpFOVObserver.patch && \
    ./tools-woogeen/build.sh && \

    # Get js client sdk for owt
    cd /home && git clone -b ${OWT_BRANCH_JS} ${OWT_SDK_REPO} && cd owt-client-javascript/scripts && git reset --hard ${OWT_BRANCH_JS_COMMIT}  && npm install && grunt  && \
    export LD_LIBRARY_PATH=/usr/local/lib64 && \
    #Build and pack owt
    cd ${SERVER_PATH} && export CPLUS_INCLUDE_PATH=/usr/local/include/svt-hevc && export PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig && ./scripts/build.js -t mcu -r -c && \
    ./scripts/pack.js -t all --install-module --no-pseudo --sample-path /home/owt-client-javascript/dist/samples/conference

FROM centos:7.6.1810
LABEL Description="This is the image for owt development on CentOS 7.6"
LABEL Vendor="Intel Corporation"
WORKDIR /home

# Prerequisites
# Install node
ARG NODE_VER=v10.21.0
ARG NODE_REPO=https://nodejs.org/dist/${NODE_VER}/node-${NODE_VER}-linux-x64.tar.xz

RUN yum install -y -q ca-certificates wget xz-utils

RUN wget ${NODE_REPO} && \
    tar xf node-${NODE_VER}-linux-x64.tar.xz && \
    cp node-*/* /usr/local -rf && \
    rm -rf node-*

COPY --from=build /home/owt-server/dist /home/owt
COPY --from=build /home/build /
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib64:
RUN echo "[mongodb-org-3.6]" >> /etc/yum.repos.d/mongodb-org-3.6.repo && \
    echo "name=MongoDB Repository" >> /etc/yum.repos.d/mongodb-org-3.6.repo && \
    echo "baseurl=https://repo.mongodb.org/yum/redhat/7/mongodb-org/3.6/x86_64/" >> /etc/yum.repos.d/mongodb-org-3.6.repo && \
    echo "gpgcheck=1" >> /etc/yum.repos.d/mongodb-org-3.6.repo && \
    echo "enabled=1" >> /etc/yum.repos.d/mongodb-org-3.6.repo && \
    echo "gpgkey=https://www.mongodb.org/static/pgp/server-3.6.asc" >> /etc/yum.repos.d/mongodb-org-3.6.repo && \
    yum install epel-release boost-system boost-thread log4cxx glib2 freetype-devel -y && \
    yum install rabbitmq-server mongodb-org glog-devel gflags-devel -y && \
    yum remove -y -q epel-release && \
    rm -rf /var/cache/yum/*;

COPY scripts/init.sh scripts/restApi.sh scripts/restart.sh scripts/sleep.sh scripts/start.sh /home/
COPY scripts/scripts/ /home/scripts/
RUN cd /home/scripts &&\
    npm config set proxy=${http_proxy} && \
    npm config set https-proxy=${http_proxy} && \
    npm install

