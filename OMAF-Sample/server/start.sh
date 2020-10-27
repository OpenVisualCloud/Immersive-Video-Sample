#!/bin/bash -ex

DIR=$(dirname $(readlink -f "$0"))
NODEIP=$(echo $1 | awk -v RS='([0-9]+.){3}[0-9]+' 'RT{print RT}')

for YAML in $(find "${DIR}" -maxdepth 1 -name "*.yaml" -print); do
    sed -i "s/NODEIP/${NODEIP}/g" ${YAML}
    kubectl apply -f "${YAML}"
done
