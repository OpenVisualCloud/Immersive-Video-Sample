#!/bin/bash -e

DIR=$(dirname $(readlink -f "$0"))
yml="$DIR/docker-compose.yml"

sudo -E docker-compose -f "$yml" up ${1}
