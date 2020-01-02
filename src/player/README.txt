
Build player on linux platform:

First install, please refer to ./install_script/setup_and_compile.sh

Build dependency libraries when codes update.

1. build 360SCVP library
$cd ../360SCVP
$mkdir build
$cd build
$cmake ..
$make -j
$sudo make install

2. build OmafDashAccess library
$cd ../OmafDashAccess
$mkdir build
$cd build
$cmake ..
$make -j
$sudo make install

3. build player
run compile.sh