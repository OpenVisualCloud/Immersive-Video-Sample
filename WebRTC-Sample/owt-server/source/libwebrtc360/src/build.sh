#!/bin/zsh -ex

source /opt/rh/devtoolset-7/enable

FILE_PATH="${PWD}/"
HIGH_INPUT_4K="4k/stitcher-hi-res-0x7f799802df20.hevc"
LOW_INPUT_4K="4k/stitcher-low-res-0x7f799802df20.hevc"
HIGH_INPUT_8K="8k/stitcher-hi-res-0x7f7eec001050.hevc"
LOW_INPUT_8K="8k/stitcher-low-res-0x7f799802df20.hevc"

mkdir -p build && cd build && cmake .. && make -j8

cd ../sample

mkdir -p build && cd build && cmake .. && make -j8

./sample --help

# ./sample \
#     -f 30 \
#     -r 4K \
#     -h "${FILE_PATH}${HIGH_INPUT_4K}" \
#     -l "${FILE_PATH}${LOW_INPUT_4K}"

