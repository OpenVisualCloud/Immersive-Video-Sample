FROM centos:7.6.1810
LABEL Description="This is the complete image for WebRTC360 CentOS 7.6"
LABEL Vendor="Intel Corporation"
WORKDIR /home/WebRTC360
ARG WORKDIR=/home/WebRTC360
ARG DEPSDIR=/third_party/owt-server/build/libdeps/build/

# Install prerequisites
RUN rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7
RUN yum install -y ca-certificates wget git sudo make && rm -rf /var/cache/yum/*
RUN wget https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/e/epel-release-7-14.noarch.rpm && \
    rpm -Uvh epel-release*rpm && rm -rf epel-release-7-14.noarch.rpm
RUN yum install -y centos-release-scl-rh && rm -rf /var/cache/yum/*
RUN yum install -y epel-release boost-system boost-thread xz-utils && rm -rf /var/cache/yum/*
RUN yum install -y log4cxx glib2 freetype-devel psmisc devtoolset-7-gcc* && rm -rf /var/cache/yum/*
RUN yum install -y rabbitmq-server libcurl-devel mongodb-org && rm -rf /var/cache/yum/*

# Configure proxy if necessary
RUN if [ -n "${http_proxy}" ] ; then \
    echo "http_proxy=${http_proxy}" >> /etc/wgetrc && \
    echo "https_proxy=${https_proxy}" >> /etc/wgetrc && \
    git config --global http.proxy ${http_proxy} && \
    git config --global https.proxy ${https_proxy} ; \
    fi

# Clone owt-server repo and install
RUN mkdir -p ${WORKDIR}/third_party && \
    cd ${WORKDIR}/third_party && \
    git clone --branch 5.0.x https://github.com/open-webrtc-toolkit/owt-server.git && \
    cd ${WORKDIR}/third_party/owt-server && \
    source /opt/rh/devtoolset-7/enable && \
    yum install -y python3 && rm -rf /var/cache/yum/* && \
    export LD_LIBRARY_PATH=${WORKDIR}/${DEPSDIR}/lib/:$LD_LIBRARY_PATH && \
    export LD_LIBRARY_PATH=${WORKDIR}/${DEPSDIR}/lib64/:$LD_LIBRARY_PATH && \
    export PKG_CONFIG_PATH=${WORKDIR}/${DEPSDIR}/lib/pkgconfig/:$PKG_CONFIG_PATH && \
    export PKG_CONFIG_PATH=${WORKDIR}/${DEPSDIR}/lib64/pkgconfig/:$PKG_CONFIG_PATH && \
    ./scripts/installDepsUnattended.sh

RUN cd ${WORKDIR}/third_party/owt-server && \
    source /opt/rh/devtoolset-7/enable && \
    \. "$HOME/.nvm/nvm.sh" && \
    export LD_LIBRARY_PATH=${WORKDIR}/${DEPSDIR}/lib/:$LD_LIBRARY_PATH && \
    export LD_LIBRARY_PATH=${WORKDIR}/${DEPSDIR}/lib64/:$LD_LIBRARY_PATH && \
    export PKG_CONFIG_PATH=${WORKDIR}/${DEPSDIR}/lib/pkgconfig/:$PKG_CONFIG_PATH && \
    export PKG_CONFIG_PATH=${WORKDIR}/${DEPSDIR}/lib64/pkgconfig/:$PKG_CONFIG_PATH && \
    ./scripts/build.js -t mcu --check

# Clone owt-client-javascript repo and install
RUN mkdir -p ${WORKDIR}/third_party && \
    cd ${WORKDIR}/third_party && \
    git clone --branch 5.0.x https://github.com/open-webrtc-toolkit/owt-client-javascript.git && \
    cd ${WORKDIR}/third_party/owt-client-javascript/scripts && \
    . "$HOME/.nvm/nvm.sh" && \
    npm install -g grunt-cli && \
    npm install -g mongojs && \
    npm i npm@6.13.1 -g && \
    npm install && \
    grunt

# Pack owt
RUN cd ${WORKDIR}/third_party/owt-server && \
    . "$HOME/.nvm/nvm.sh" && \
    export LD_LIBRARY_PATH=${WORKDIR}/${DEPSDIR}/lib/:$LD_LIBRARY_PATH && \
    export LD_LIBRARY_PATH=${WORKDIR}/${DEPSDIR}/lib64/:$LD_LIBRARY_PATH && \
    export PKG_CONFIG_PATH=${WORKDIR}/${DEPSDIR}/lib/pkgconfig/:$PKG_CONFIG_PATH && \
    export PKG_CONFIG_PATH=${WORKDIR}/${DEPSDIR}/lib64/pkgconfig/:$PKG_CONFIG_PATH && \
    ./scripts/pack.js \
        -t all -i -with-ffmpeg \
        -p ../owt-client-javascript/dist/samples/conference

# Get necessary files ready
COPY rest/ ${WORKDIR}/rest/
COPY source/ ${WORKDIR}/source/
COPY scripts/ ${WORKDIR}/scripts/
COPY scripts/mongodb-org-4.4.repo /etc/yum.repos.d/

# Install cmake
ARG CMAKE_VER=3.12.4
ARG CMAKE_REPO=https://cmake.org/files
RUN wget -O - ${CMAKE_REPO}/v${CMAKE_VER%.*}/cmake-${CMAKE_VER}.tar.gz | tar xz && \
    cd cmake-${CMAKE_VER} && \
    source /opt/rh/devtoolset-7/enable && \
    ./bootstrap --prefix="/usr" --system-curl && \
    make -j$(nproc) && \
    make install && \
    cd ${WORKDIR} && rm -rf ./cmake*

# Install deps
RUN source /opt/rh/devtoolset-7/enable && \
    . "$HOME/.nvm/nvm.sh" && \
    ./scripts/install_deps.sh

# Build immersive video mixer node
RUN source /opt/rh/devtoolset-7/enable && \
    . "$HOME/.nvm/nvm.sh" && \
    ./scripts/build.js -t all

# Pack IM video DIST
RUN source /opt/rh/devtoolset-7/enable && \
    ./scripts/pack.sh

COPY deploy/test1_h265_3840x2048_30fps_30M_200frames.mp4 ${WORKDIR}
COPY deploy/systemctl.py /usr/bin/systemctl
RUN chmod a+x /usr/bin/systemctl
