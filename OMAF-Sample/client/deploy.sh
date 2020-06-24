#!/bin/bash -ex
cd ../../src/external
./build.sh client y
./fpm.sh client 1.0.0
