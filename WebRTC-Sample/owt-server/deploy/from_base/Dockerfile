################################
 
# Base image owt-base:5.0
FROM owt-base:5.0
LABEL Description="This is the complete image for WebRTC360 CentOS 7.6"
LABEL Vendor="Intel Corporation"
WORKDIR /home/WebRTC360
ARG WORKDIR=/home/WebRTC360

# Configure proxy if necessary
RUN if [ -n "${http_proxy}" ] ; then \
    echo "proxy=${http_proxy}" >> /etc/yum.conf && \
    echo "http_proxy=${http_proxy}" >> /etc/wgetrc && \
    echo "https_proxy=${https_proxy}" >> /etc/wgetrc && \
    git config --global http.proxy ${http_proxy} && \
    git config --global https.proxy ${https_proxy} ; \
    fi

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

