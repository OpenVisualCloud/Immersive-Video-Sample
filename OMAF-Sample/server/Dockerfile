FROM centos:7.6.1810

RUN if [ -n "${http_proxy}" ] ; then \
    echo "proxy=${http_proxy}" >> /etc/yum.conf && \
    echo "http_proxy=${http_proxy}" >> /etc/wgetrc && \
    echo "https_proxy=${https_proxy}" >> /etc/wgetrc ; \
    fi

WORKDIR /home/immersive
ARG WORKDIR=/home/immersive

# Install denpendency
RUN yum install -y centos-release-scl-rh && \
    yum install -y wget git bzip2 xz sudo devtoolset-7-gcc* && \
    wget https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/e/epel-release-7-14.noarch.rpm && \
    rpm -Uvh epel-release*rpm && \
    yum install -y openssl centos-release-scl scl-utils gmp gmp-devel && \
    yum install -y mpfr mpfr-devel libmpc libmpc-devel patch autoconf && \
    yum install -y libtool automake libcurl-devel libxml2-devel && \
    yum install -y libevent-devel.x86_64 openssl-devel bc redhat-lsb && \
    yum install -y libXrandr libXrandr-devel libXinerama libXinerama-devel && \
    yum install -y libXcursor libXcursor-devel libXi libXi-devel glm-devel && \
    yum install -y mesa-libGL mesa-libGL-devel mesa-libGLU && \
    yum install -y mesa-libGLU-devel mesa-libGLES-devel mesa-libEGL-devel && \
    yum install -y SDL2 SDL2-devel libcurl4-openssl-dev libglfw3 && \
    yum install -y libXv-devel glfw glfw-devel xz-devel lzma -y && \
    yum install -y uuid.x86_64 uuid-devel.x86_64 popt-devel.x86_64 -y && \
    yum install -y numactl.x86_64 numactl-devel.x86_64 numactl-libs.x86_64 -y && \
    rm -rf /var/cache/yum/* && rm -rf * && \
    if [ -n "${http_proxy}" ]; then \
    git config --global http.proxy ${http_proxy} && \
    git config --global https.proxy ${https_proxy} ; \
    fi

RUN yum -y install rh-ruby23 rh-ruby23-ruby-devel rh-ruby23-rubygem* && \
    source /opt/rh/rh-ruby23/enable && \
    yum -y install rpm-build && \
    if [ -n "${http_proxy}" ]; then \
    gem install -p ${http_proxy} fpm ; \
    else \
    gem install fpm ; \
    fi

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
    sed -i '23s/OFF/ON/' CMakeLists.txt && \
    cmake -H. -Bbuild -G "Unix Makefiles" && \
    cmake --build build && \
    cmake --build build --target install && \
    cd ${WORKDIR} && rm -rf ./glog

# Install lttng
RUN source /opt/rh/devtoolset-7/enable && \
    wget -c https://lttng.org/files/urcu/userspace-rcu-latest-0.11.tar.bz2 && \
    tar -xjf userspace-rcu-latest-0.11.tar.bz2 && \
    cd userspace-rcu-0.11.* && \
    ./configure && \
    make -j $(nproc) && \
    make install && ldconfig && \
    cd ../ && rm -rf userspace-rcu-0.11.* && \
    wget -c http://lttng.org/files/lttng-ust/lttng-ust-latest-2.11.tar.bz2 && \
    tar -xjf lttng-ust-latest-2.11.tar.bz2 && \
    cd lttng-ust-2.11.* && \
    ./configure --disable-man-pages && \
    make -j $(nproc) && \
    make install && ldconfig && \
    cd ../ && rm -rf lttng-ust-2.11.*

# Copy source
COPY ./src ${WORKDIR}
COPY src/ffmpeg/dependency/*.so /usr/local/lib/
COPY src/ffmpeg/dependency/*.pc /usr/local/lib/pkgconfig/
COPY src/ffmpeg/dependency/*.h /usr/local/include/
COPY src/ffmpeg/dependency/WorkerServer /root

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

# Install openHEVC
RUN git clone https://github.com/OpenHEVC/openHEVC.git && \
    cd openHEVC && \
    source /opt/rh/devtoolset-7/enable && \
    git config --global user.email "you@example.com" && \
    git config --global user.name "Your Name" && \
    git checkout ffmpeg_update && \
    git am --whitespace=fix ../external/Update-buffer-operation-and-fix-stream-loop-coredump.patch && \
    ./configure --libdir=/usr/lib64 --disable-sdl2 && \
    make -j `nproc` && \
    make install && \
    cd ${WORKDIR} && rm -rf ./openHEVC

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

# Build safe string lib
RUN git clone https://github.com/intel/safestringlib.git && \
    cd safestringlib && \
    source /opt/rh/devtoolset-7/enable && \
    cmake . && \
    make -j `nproc` -f Makefile && \
    cp libsafestring_shared.so /usr/local/lib/ && \
    mkdir -p /usr/local/include/safestringlib/ && \
    cp ./include/* /usr/local/include/safestringlib/ && \
    cd ${WORKDIR} && rm -rf ./safestringlib

# Configure And Run Nginx
COPY nginx_conf /usr/local/nginx/conf

# Build Server
RUN cd external && \
    source /opt/rh/devtoolset-7/enable && \
    ./install_FFmpeg.sh server && \
    ./build.sh server n && \
    mv ../build/server/ffmpeg/ffmpeg ../Sample-Videos && \
    cd .. && rm -rf `ls | grep -v Sample-Videos`

COPY run.sh ${WORKDIR}/Sample-Videos
COPY src/ffmpeg/dependency/*.xml ${WORKDIR}/Sample-Videos/

# Unset proxy if necessary
RUN if [ -n "${http_proxy}" ]; then \
    sed -i '$d' /etc/yum.conf && \
    sed -i '1,$d' /etc/wgetrc && \
    git config --global --unset http.proxy && \
    git config --global --unset https.proxy ; \
    fi
