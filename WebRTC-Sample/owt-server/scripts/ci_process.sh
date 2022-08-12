#!/bin/bash -ex

ln -s /home/WebRTC360/third_party `pwd`/../
ln -s /home/WebRTC360/build `pwd`/../

source /opt/rh/devtoolset-7/enable
export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"

./build.js -t all
./pack.sh
