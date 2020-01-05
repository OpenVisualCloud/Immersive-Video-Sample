#!/bin/bash -ex
cd ../..
cp -r src OMAF-Sample/Server
cd OMAF-Sample/Server
docker build -t test:v0.1 .
