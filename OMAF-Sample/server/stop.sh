#!/bin/bash -e

DIR=$(dirname $(readlink -f "$0"))

for YAML in $(find "${DIR}" -maxdepth 1 -name "*.yaml" -print); do
    kubectl delete --wait=false -f "${YAML}" --ingore-not-found=true 2>/dev/null
done
