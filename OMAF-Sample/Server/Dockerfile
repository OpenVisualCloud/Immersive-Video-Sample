FROM centos:7.6.1810
# FROM centos:7.4.1708

ENV http_proxy http://child-prc.intel.com:913
ENV https_proxy http://child-prc.intel.com:913
ENV no_proxy 127.0.0.1,*.intel.com

RUN yum install -y -q openssl centos-release-scl scl-utils gmp gmp-devel mpfr mpfr-devel libmpc libmpc-devel patch autoconf libtool automake libcurl-devel wget git -y && \
    rm -rf /var/cache/yum/*

RUN yum install -y yum install devtoolset-7-gcc* libevent-devel.x86_64 openssl-devel bc -y && \
    rm -rf /var/cache/yum/*

WORKDIR /home/immersive
ARG WORKDIR=/home/immersive

# Build zlib
ARG ZLIB_VER=1.2.11
ARG ZILB_REPO=http://zlib.net/zlib-${ZLIB_VER}.tar.gz
RUN wget -O - ${ZILB_REPO} | tar xz && \
    cd zlib-${ZLIB_VER} && \
    source /opt/rh/devtoolset-7/enable && \
    ./configure && \
    make -j$(nproc) && \
    make install && \
    cd ${WORKDIR} && rm -rf ./*

# Install cmake
ARG CMAKE_VER=3.12.4
ARG CMAKE_REPO=https://cmake.org/files
RUN wget -O - ${CMAKE_REPO}/v${CMAKE_VER%.*}/cmake-${CMAKE_VER}.tar.gz | tar xz && \
    cd cmake-${CMAKE_VER} && \
    source /opt/rh/devtoolset-7/enable && \
    ./bootstrap --prefix="/usr" --system-curl && \
    make -j$(nproc) && \
    make install && \
    cd ${WORKDIR} && rm -rf ./*

# # Install automake, use version 1.14 on CentOS
# ARG AUTOMAKE_VER=1.14
# ARG AUTOMAKE_REPO=https://ftp.gnu.org/pub/gnu/automake/automake-${AUTOMAKE_VER}.tar.xz
#     RUN yum install -y -q automake

# Build YASM
ARG YASM_VER=1.3.0
ARG YASM_REPO=https://www.tortall.net/projects/yasm/releases/yasm-${YASM_VER}.tar.gz
RUN  wget -O - ${YASM_REPO} | tar xz && \
     cd yasm-${YASM_VER} && \
     sed -i "s/) ytasm.*/)/" Makefile.in && \
     source /opt/rh/devtoolset-7/enable && \
     ./configure --prefix="/usr" --libdir=/usr/lib/x86_64-linux-gnu && \
     make -j$(nproc) && \
     make install && \
     cd ${WORKDIR} && rm -rf ./*

# Build libxml2
ARG LIBXML2_VER=2.9.6
ARG LIBXML2_REPO=http://www.xmlsoft.org/sources/libxml2-${LIBXML2_VER}.tar.gz
RUN wget -O - ${LIBXML2_REPO} | tar xz && \
    cd libxml2-${LIBXML2_VER} && \
    source /opt/rh/devtoolset-7/enable && \
    ././configure --with-python=no && \
    make -j$(nproc) && \
    make install && \
    cd ${WORKDIR} && rm -rf ./*

RUN yum install -y xz && \
    rm -rf /var/cache/yum/*

# Build CURL
ARG CURL_VER=7.66.0
ARG CURL_REPO=https://curl.haxx.se/download/curl-${CURL_VER}.tar.xz
RUN wget ${CURL_REPO} && \
    xz -d curl-${CURL_VER}.tar.xz && \
    tar -xvf curl-${CURL_VER}.tar && \
    cd curl-${CURL_VER} && \
    source /opt/rh/devtoolset-7/enable && \
    ./configure --with-darwinssl && \
    make -j$(nproc) && \
    make install && \
    cd /usr/local/include/libxml2 && \
    mv libxml ../ && \
    cd ${WORKDIR} && rm -rf ./*

# Install BOOST
ARG BOOST_REPO=https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.tar.gz
RUN wget -O - ${BOOST_REPO} | tar xz && \
    cd boost_1_63_0 && \
    source /opt/rh/devtoolset-7/enable && \
    ./bootstrap.sh --without-libraries=python && \
    ./b2 -a cxxflags="-D_GLIBCXX_USE_CXX11_ABI=0" -j`nproc` && \
    ./b2 cxxflags="-D_GLIBCXX_USE_CXX11_ABI=0" install && \
    cd ${WORKDIR} && rm -rf ./boost_1_63_0*

# Install SVT
RUN git clone https://github.com/OpenVisualCloud/SVT-HEVC.git && \
    cd SVT-HEVC && \
    source /opt/rh/devtoolset-7/enable && \
    git checkout ec0d95c7e0d5be20586e1b87150bdfb9ae97cf4d && \
    cd Build/linux/ && \
    ./build.sh && \
    cd Release && \
    make install && \
    cd ${WORKDIR} && rm -rf ./SVT-HEVC

# Install glog
RUN git clone https://github.com/google/glog.git && \
    cd glog && \
    source /opt/rh/devtoolset-7/enable && \
    ./autogen.sh && \
    ./configure && \
    make -j8 && \
    make install && \
    cd ${WORKDIR} && rm -rf ./glog

# # Clone git repo
# RUN echo "StrictHostKeyChecking no" >> /etc/ssh/ssh_config
# RUN git clone ssh://git@gitlab.devtools.intel.com:29418/vcdp/tids.git && \
# 	cd tids && \
#     git checkout OMAF_Compliance

# Copy source
COPY ./src ${WORKDIR}

# Install openHEVC
RUN git clone https://github.com/OpenHEVC/openHEVC.git && \
    cd openHEVC && \
    source /opt/rh/devtoolset-7/enable && \
    git checkout ffmpeg_update && \
    patch -p1 < ../external/ffmpeg_update_add_circle_list_for_to_free_frame.patch && \
    ./configure --libdir=/usr/lib64 --disable-sdl2 && \
    make -j `nproc` && \
    make install && \
    cd ${WORKDIR} && rm -rf ./openHEVC

# Install Thrift
ARG THRIFT_VER=0.12.0
ARG THRIFT_REPO=http://apache.osuosl.org/thrift/${THRIFT_VER}/thrift-${THRIFT_VER}.tar.gz
RUN wget -O - ${THRIFT_REPO} | tar xz && \
    cd thrift-${THRIFT_VER} && \
    source /opt/rh/devtoolset-7/enable && \
    patch configure ../external/Disable_cxx11_abi_for_thrift.patch && \
    sed -i '21 a #  include <unistd.h>' ./lib/cpp/src/thrift/transport/PlatformSocket.h && \
    ./configure --with-boost=/usr/local --with-boost-libdir=/usr/local/lib --with-libevent=/usr --with-java=0 && \
    make -j`nproc` && \
    make install && \
    cd ${WORKDIR} && rm -rf ./thrift-${THRIFT_VER}*

# Install OMAF
RUN git clone https://github.com/nokiatech/omaf.git && \
    ln -s ${PWD}/omaf/Mp4/srcs ${PWD}/OmafDashAccess/mp4lib && \
    cd omaf && \
    source /opt/rh/devtoolset-7/enable && \
    patch -p1 < ../external/nokia_omaf_patch_for_extrator_reader.diff && \
    cd Mp4/srcs && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j`nproc` && \
    cp -r ../api/streamsegmenter ../../../../VROmafPacking/ && \
    cp lib/libstreamsegmenter_static_fpic.a /usr/local/lib/ && \
    cp lib/libstreamsegmenter_static.a /usr/local/lib/ && \
    cp lib/libmp4vr_static_fpic.a /usr/local/lib/ && \
    cp lib/libmp4vr_static.a /usr/local/lib/

# Install Server FFmpeg Dependency
# COPY FFmpeg_OMAF.patch ${WORKDIR}/tids/external
RUN git clone https://github.com/FFmpeg/FFmpeg.git && \
    cd FFmpeg && \
    source /opt/rh/devtoolset-7/enable && \
    git checkout release/4.1 && \
    git checkout c2ac3b8e6a040e33d53fa13548848c8ba981a8e4 && \
    cd - && patch -p1 < ffmpeg/patches/FFmpeg_OMAF.patch && \
    mkdir -p external/FFmpeg && cd external/FFmpeg && \
    ../../FFmpeg/configure --prefix=/usr --libdir=/usr/local/lib --enable-static --enable-shared --enable-gpl --enable-nonfree --disable-optimizations --disable-vaapi && \
    make -j `nproc` && \
    make install && \
    cd ${WORKDIR} && rm -rf ./external/FFmpeg

# Build Nginx
ARG NGINX_VER=1.13.1
ARG NGINX_REPO=http://nginx.org/download/nginx-${NGINX_VER}.tar.gz
RUN wget -O - ${NGINX_REPO} | tar xz && \
    cd nginx-${NGINX_VER} && \
    source /opt/rh/devtoolset-7/enable && \
    ./configure --with-http_ssl_module && \
    make -j `nproc` && \
    make install && \
    cd ${WORKDIR} && rm -rf ./nginx*
EXPOSE 443
EXPOSE 8080
EXPOSE 9089
EXPOSE 9090

# Configure And Run Nginx
COPY nginx_conf /usr/local/nginx/conf
COPY src/ffmpeg/dependency/*.so /usr/local/lib/
COPY src/ffmpeg/dependency/*.pc /usr/local/lib/pkgconfig/
COPY src/ffmpeg/dependency/*.h /usr/local/include/
COPY src/ffmpeg/dependency/WorkerServer /root
COPY src/utils/tinyxml2.cpp ${WORKDIR}/VROmafPacking
COPY src/utils/tinyxml2.h ${WORKDIR}/VROmafPacking
# RUN /usr/local/nginx/sbin/nginx && \
#     systemctl stop firewalld.service

# Build Server
RUN sed -i "s/\-Werror//g" `grep "Werror" -rl ./` && \
    mkdir build && cd build && \
    source /opt/rh/devtoolset-7/enable && \
    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig:$PKG_CONFIG_PATH && \
    cmake -DCMAKE_BUILD_TYPE=Debug -DTARGET=server .. && \
    make -j `nproc` && \
    make install && \
    cd .. && ls
