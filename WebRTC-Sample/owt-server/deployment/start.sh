#!/bin/bash -e

SUDO=""
if [[ $EUID -ne 0 ]]; then
  SUDO="sudo -E"
fi

DIR=$(dirname $(readlink -f "$0"))
yml="$DIR/docker-compose.yml"

${SUDO} docker-compose -f "$yml" up ${1}
