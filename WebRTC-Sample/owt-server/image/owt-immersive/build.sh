#!/bin/bash -e

IMAGE="xeon-centos76-service-owt-immersive"
DIR=$(dirname $(readlink -f "$0"))

. "$DIR/../../script/build.sh"
