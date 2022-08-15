#!/bin/bash -ex

SCRIPT=`pwd`/$0
PATHNAME=`dirname ${SCRIPT}`
ROOT=${PATHNAME}/..
OWT_ROOT=${ROOT}/third_party/owt-server/

export_node() {
    # node
    export NVM_DIR="$HOME/.nvm"
    [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm
    [ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"  # This loads nvm bash_completion
    nvm use v10
}

install_devtoolset() {
    sudo yum install -y docbook2X
    sudo yum install -y centos-release-scl
    sudo yum install -y devtoolset-7
    source /opt/rh/devtoolset-7/enable
}

install_owt_server() {
    [ ! -d ${ROOT}/third_party ] && mkdir -p ${ROOT}/third_party
    pushd ${ROOT}/third_party
    git clone --branch 5.0.x https://github.com/open-webrtc-toolkit/owt-server.git
    pushd owt-server
    ./scripts/installDepsUnattended.sh

    export_node

    ./scripts/build.js -t mcu --check
    popd
    popd
}

install_owt_js_client() {
    pushd ${ROOT}/third_party
    git clone --branch 5.0.x https://github.com/open-webrtc-toolkit/owt-client-javascript.git
    pushd owt-client-javascript
    pushd scripts
    npm install -g grunt-cli
    npm install
    grunt
    popd
    popd
    popd
}

pack_owt() {
    pushd ${OWT_ROOT}
    ./scripts/pack.js --full --install-module --no-pseudo --with-ffmpeg --app-path ../owt-client-javascript/dist/samples/conference
    popd
}

main() {
    install_devtoolset
    install_owt_server
    install_owt_js_client
    pack_owt
}

main $@
