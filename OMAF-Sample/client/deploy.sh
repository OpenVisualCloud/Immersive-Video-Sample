#!/bin/bash -ex
cd ../../src/external
./build_client.sh
./fpm.sh 1.0.0
