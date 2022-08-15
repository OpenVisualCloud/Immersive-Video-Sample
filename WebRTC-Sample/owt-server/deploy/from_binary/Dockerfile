################################
 
# Base image centos 7.6.1810
FROM centos:7.6.1810
LABEL Description="This is the complete image for WebRTC360 CentOS 7.6"
LABEL Vendor="Intel Corporation"
WORKDIR /home/WebRTC360
ARG WORKDIR=/home/WebRTC360

# Prerequisites
RUN rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-7 && \
    yum install -y ca-certificates wget xz-utils && \
    yum install -y epel-release boost-system boost-thread && \
    yum install -y log4cxx glib2 freetype-devel psmisc && \
    yum install -y rabbitmq-server mongodb-org docbook2X

ARG NODE_VER=v10.21.0
ARG NODE_REPO=https://nodejs.org/dist/${NODE_VER}/node-${NODE_VER}-linux-x64.tar.xz
RUN source ~/.bashrc && \
    wget ${NODE_REPO} && \
    tar xf node-${NODE_VER}-linux-x64.tar.xz && \
    cp node-*/* /usr/local -rf && rm -rf node-* && \
    rm -rf /var/cache/yum/*

RUN mkdir -pv ${WORKDIR}/dist && \
    mkdir -pv ${WORKDIR}/build/libdeps/build

COPY rest/ ${WORKDIR}/rest/
COPY dist/ ${WORKDIR}/dist/
COPY build/ ${WORKDIR}/build/
COPY deploy/test1_h265_3840x2048_30fps_30M_200frames.mp4 ${WORKDIR}
COPY scripts/mongodb-org-4.4.repo /etc/yum.repos.d/

RUN cd rest/scripts/ && \
    npm install

COPY deploy/systemctl.py /usr/bin/systemctl
RUN chmod a+x /usr/bin/systemctl

ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${WORKDIR}/build/libdeps/build/lib
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${WORKDIR}/build/libdeps/build/lib64:/usr/local/lib

