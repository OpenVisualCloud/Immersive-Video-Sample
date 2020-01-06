#!/bin/bash -e

cd /home/owt

./bin/stop-all.sh
rm ./logs/*
./bin/start-all.sh
